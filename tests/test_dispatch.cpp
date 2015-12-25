#include <atomic>
#include <functional>
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
        for (int i = 3000; i < 4000; ++i) {
            tasks2.emplace_back([i, &x] { x += i; return i; });
        }

        WHEN("passed to dispatch") {

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
                    tp.dispatch<int>(std::bind(task2, i));
                }
                tp.dispatch(tasks1.cbegin(), tasks1.cend());
                tp.dispatch<int>(tasks2.cbegin(), tasks2.cend());
            }
            int result = x;

            THEN("task finishes before task pool is destroyed") {

                REQUIRE(result == (0 + 3999) * 4000 / 2);
            }
        }
    }
}

