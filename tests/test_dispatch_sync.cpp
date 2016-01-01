#include <atomic>
#include <future>
#include <numeric>
#include <thread>
#include <vector>

#include "gungnir/gungnir.hpp"

#include "catch.hpp"

SCENARIO("dispatchSync waits for all tasks to finish", "[sync]") {

    gungnir::TaskPool tp{8};

    GIVEN("a collection of tasks") {

        std::atomic<int> count{0};

        std::vector<gungnir::Task<void>> tasks;
        for (int i = 0; i < 1000; ++i) {
            tasks.emplace_back([i, &count] { count += i; });
        }

        WHEN("passed to dispatchSync") {

            std::thread t1{[tasks, &tp, &count] {
                tp.dispatchSync(tasks.cbegin(), tasks.cend());
            }};
            std::thread t2{[tasks, &tp, &count] {
                tp.dispatchSync(tasks.cbegin(), tasks.cend());
            }};

            tp.dispatchSync(tasks.cbegin(), tasks.cend());
            t1.join();
            t2.join();
            int finalCount = count;

            THEN("all tasks finish before dispatchSync returns") {

                REQUIRE(finalCount == 3 * (0 + 999) * 1000 / 2);
            }
        }
    }

    GIVEN("a collection of tasks that return values") {

        std::atomic<int> count{0};

        std::vector<gungnir::Task<int>> tasks;
        for (int i = 0; i < 1000; ++i) {
            tasks.emplace_back([i, &count] { count += i; return i; });
        }

        WHEN("passed to dispatchSync") {

            std::vector<int> v1, v2;

            std::thread t1{[tasks, &tp, &count, &v1] {
                v1 = tp.dispatchSync<int>(tasks.cbegin(), tasks.cend());
            }};
            std::thread t2{[tasks, &tp, &count, &v2] {
                v2 = tp.dispatchSync<int>(tasks.cbegin(), tasks.cend());
            }};

            t1.join();
            t2.join();
            auto v3 = tp.dispatchSync<int>(tasks.cbegin(), tasks.cend());
            int finalCount = count;
            
            THEN("all tasks finish before dispatchSync call returns") {

                REQUIRE(finalCount == 3 * (0 + 999) * 1000 / 2);
                for (const auto &v: {v1, v2, v3}) {
                    REQUIRE(std::accumulate(v.cbegin(), v.cend(), 0)
                            == (0 + 999) * 1000 / 2);
                }
            }
        }
    }
}

