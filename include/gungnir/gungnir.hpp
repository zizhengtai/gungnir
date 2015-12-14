#ifndef GUNGNIR_HPP
#define GUNGNIR_HPP

#include <atomic>
#include <functional>
#include <stdexcept>
#include <thread>
#include <vector>

#include "external/blockingconcurrentqueue.h"

namespace gungnir {

using Task = std::function<void(void)>;

class TaskPool {
public:
    explicit TaskPool(std::size_t numThreads)
        : _numThreads{numThreads}
    {
        for (std::size_t i = 0; i < _numThreads; ++i) {
            _threads.emplace_back(std::thread([this] {
                Task t;

                _tasks.wait_dequeue(t);
                while (t) {
                    t();
                    _tasks.wait_dequeue(t);
                }
            }));
        }
    }

    ~TaskPool()
    {
        _destroyed = true; // prevent any future task dispatches

        for (std::size_t i = 0; i < _numThreads; ++i) {
            _tasks.enqueue(Task{});
        }
        for (auto &t: _threads) {
            t.join();
        }
    }

    void dispatch(Task&& task)
    {
        if (_destroyed) {
            throw std::runtime_error("task pool already destroyed");
        }
        if (!task) {
            throw std::invalid_argument("task has no target callable object");
        }

        _tasks.enqueue(std::move(task));
    }

private:
    std::atomic_bool _destroyed{false};
    const std::size_t _numThreads;
    std::vector<std::thread> _threads;
    moodycamel::BlockingConcurrentQueue<Task> _tasks;
};

}

#endif // GUNGNIR_HPP
