#include <atomic>
#include <future>
#include <numeric>
#include <vector>

#include "gungnir/gungnir.hpp"

#include "catch.hpp"

SCENARIO("dispatchSync waits for all tasks to finish", "[sync]") {

    gungnir::TaskPool tp{8};

    GIVEN("a collection of tasks") {

        std::atomic<int> count{0};

        std::vector<gungnir::Task> tasks;
        for (int i = 0; i < 1000; ++i) {
            tasks.emplace_back([i, &count] { count += i; });
        }

        WHEN("passed to dispatchSync") {

            tp.dispatchSync(tasks.cbegin(), tasks.cend());
            int finalCount = count;

            THEN("all tasks finish before dispatchSync returns") {

                REQUIRE(finalCount == (0 + 999) * 1000 / 2);
            }
        }
    }

    GIVEN("a collection of tasks that return values") {

        std::atomic<int> count{0};

        std::vector<std::function<int()>> tasks;
        for (int i = 0; i < 1000; ++i) {
            tasks.emplace_back([i, &count] { count += i; return i; });
        }

        WHEN("passed to dispatchSync") {

            auto v = tp.dispatchSync<int>(tasks.cbegin(), tasks.cend());
            int finalCount = count;
            
            THEN("all tasks finish before dispatchSync call returns") {

                REQUIRE(finalCount == (0 + 999) * 1000 / 2);
                REQUIRE(std::accumulate(v.cbegin(), v.cend(), 0)
                        == (0 + 999) * 1000 / 2);
            }
        }
    }
}

