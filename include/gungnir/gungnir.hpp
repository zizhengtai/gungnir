#ifndef GUNGNIR_HPP
#define GUNGNIR_HPP

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

#include <iostream>

#include "gungnir/external/blockingconcurrentqueue.h"

namespace gungnir {

using Task = std::function<void()>;

class TaskPool final {
public:
    explicit TaskPool(
            std::size_t numThreads = std::thread::hardware_concurrency())
        : numThreads_{numThreads}
    {
        threads_.reserve(numThreads_);

        for (std::size_t i = 0; i < numThreads_; ++i) {
            threads_.emplace_back([this] {
                Task t;

                tasks_.wait_dequeue(t);
                while (t) {
                    t();
                    tasks_.wait_dequeue(t);
                }
            });
        }
    }

    ~TaskPool()
    {
        destroyed_ = true; // prevent any future task dispatches

        for (std::size_t i = 0; i < numThreads_; ++i) {
            tasks_.enqueue(Task{});
        }
        for (auto &t: threads_) {
            t.join();
        }

        // pump until empty
        std::atomic_size_t numDoneThreads{0};
        for (auto &t: threads_) {
            t = std::thread([this, &numDoneThreads] {
                Task t;

                do {
                    while (tasks_.try_dequeue(t)) {
                        t();
                    }
                } while (numDoneThreads.fetch_add(
                            1, std::memory_order_acq_rel) + 1 ==
                        numThreads_);
            });
        }
        for (auto &t: threads_) {
            t.join();
        }
    }

    TaskPool(const TaskPool &other) = delete;
    TaskPool(TaskPool &&other) = delete;
    Task & operator=(const TaskPool &other) = delete;
    Task & operator=(TaskPool &&other) = delete;

    void dispatch(const Task &task)
    {
        checkArgs(task);

        tasks_.enqueue(task);
    }

    template <typename R>
    std::future<R> dispatch(const std::function<R()> &task)
    {
        checkArgs(task);

        auto p = std::make_shared<std::promise<R>>();
        tasks_.enqueue([p, task]() mutable {
            try {
                p->set_value(task());
            } catch (...) {
                p->set_exception(std::current_exception());
            }
        });
        return p->get_future();
    }

    template <typename Iter>
    void dispatch(Iter first, Iter last)
    {
        if (first >= last) {
            return;
        }
        checkArgs(first, last);

        tasks_.enqueue_bulk(first, last - first);
    }

    template <typename R, typename Iter>
    std::vector<std::future<R>> dispatch(Iter first, Iter last)
    {
        if (first >= last) {
            return {};
        }
        checkArgs(first, last);

        std::vector<std::future<R>> futures;
        futures.reserve(last - first);
        for (auto it = first; it != last; ++it) {
            futures.emplace_back(dispatch<R>(*it));
        }
        return futures;
    }

    template <typename Iter>
    void dispatchSerial(Iter first, Iter last)
    {
        if (first >= last) {
            return;
        }
        checkArgs(first, last);

        auto tasks = std::make_shared<std::vector<Task>>(first, last);
        dispatch([tasks] {
            for (const auto &t: *tasks) {
                t();
            }
        });
    }

    template <typename Iter>
    void dispatchSync(Iter first, Iter last)
    {
        if (first >= last) {
            return;
        }
        checkArgs(first, last);

        std::atomic_size_t count{static_cast<std::size_t>(last - first)};
        std::mutex m;
        std::condition_variable cv;

        for (auto it = first; it != last; ++it) {
            dispatch(std::bind([&](Task t) {
                t();

                std::unique_lock<std::mutex> lk(m);
                if (--count == 0) {
                    cv.notify_all();
                }
            }, *it));
        }

        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&count] { return count == 0; });
    }

    template <typename R, typename Iter>
    std::vector<R> dispatchSync(Iter first, Iter last)
    {
        if (first >= last) {
            return {};
        }
        checkArgs(first, last);

        std::vector<std::future<R>> futures;
        futures.reserve(last - first);

        for (auto it = first; it != last; ++it) {
            futures.emplace_back(dispatch<R>(std::bind([&](decltype(*it) t) {
                return t();
            }, *it)));
        }

        std::vector<R> results;
        results.reserve(last - first);

        for (auto &f: futures) {
            results.emplace_back(f.get());
        }
        return results;
    }

    void dispatchOnce(std::once_flag &flag, const Task &task)
    {
        std::call_once(flag, task);
    }

private:
    template <typename T>
    void checkArgs(const T &task) const
    {
        if (destroyed_) {
            throw std::runtime_error("task pool already destroyed");
        }
        if (!task) {
            throw std::invalid_argument("task has no target callable object");
        }
    }

    template <typename Iter>
    void checkArgs(Iter first, Iter last) const
    {
        using T = typename std::iterator_traits<Iter>::value_type;

        if (destroyed_) {
            throw std::runtime_error("task pool already destroyed");
        }
        if (!std::all_of(first, last, [](const T &t) { return t; })) {
            throw std::invalid_argument("task has no target callable object");
        }
    }

private:
    std::atomic_bool destroyed_{false};
    const std::size_t numThreads_;
    std::vector<std::thread> threads_;
    moodycamel::BlockingConcurrentQueue<Task> tasks_;
};

}

#endif // GUNGNIR_HPP

