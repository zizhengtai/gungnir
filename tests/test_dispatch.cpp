#include <atomic>
#include <cstring>
#include <exception>
#include <functional>
#include <future>
#include <vector>

#include "gungnir/gungnir.hpp"

#include "catch.hpp"

SCENARIO("dispatch finishes before task pool is destroyed",
        "[async]") {

    GIVEN("some tasks") {

        std::atomic<int> x{0};

        auto task1 = [&x](int i) { x += i; };
        auto task2 = [&x](int i) { x += i; return i; };
        std::vector<gungnir::Task<void>> tasks1;
        for (int i = 2000; i < 3000; ++i) {
            tasks1.emplace_back([i, &x] { x += i; });
        }
        std::vector<gungnir::Task<int>> tasks2;
        for (int i = 0; i < 10; ++i) {
            tasks2.emplace_back([i, &x] {
                throw std::runtime_error{{static_cast<char>(i + '0'), '\0'}};
                x += i;
                return i;
            });
        }
        for (int i = 3000; i < 4000; ++i) {
            tasks2.emplace_back([i, &x] { x += i; return i; });
        }

        WHEN("passed to dispatch") {

            std::vector<std::future<int>> f1, f2;

            {
                gungnir::TaskPool tp{8};

                // dispatched from multiple threads
                std::vector<std::thread> threads;
                for (int i = 0; i < 100; ++i) {
                    threads.emplace_back([i, &tp, &task1] {
                        for (int j = i * 10; j < (i + 1) * 10; ++j) {
                            tp.dispatch(std::bind(task1, j));
                        }
                    });
                }
                for (auto &t: threads) {
                    t.join();
                }
                
                for (int i = 1000; i < 2000; ++i) {
                    f1.emplace_back(tp.dispatch<int>(std::bind(task2, i)));
                }
                tp.dispatch(tasks1.cbegin(), tasks1.cend());
                f2 = tp.dispatch<int>(tasks2.cbegin(), tasks2.cend());
            }
            int result = x;

            THEN("task finishes before task pool is destroyed") {

                REQUIRE(result == (0 + 3999) * 4000 / 2);

                bool matched = true;
                for (int i = 0; matched && i < 1000; ++i) {
                    if (f1[i].get() != i + 1000) {
                        matched = false;
                    }
                }
                REQUIRE(matched);

                matched = true;
                for (int i = 0; matched && i < 10; ++i) {
                    try {
                        f2[i].get();
                        matched = false;
                    } catch (const std::runtime_error &e) {
                        const char s[] = {static_cast<char>(i + '0'), '\0'};
                        matched = !std::strcmp(e.what(), s);
                    } catch (...) {
                        matched = false;
                    }
                }
                for (int i = 0; matched && i < 1000; ++i) {
                    if (f2[i + 10].get() != i + 3000) {
                        matched = false;
                    }
                }
                REQUIRE(matched);
            }
        }
    }
}
