#include <atomic>
#include <future>
#include <iterator>
#include <numeric>
#include <vector>

#include "gungnir/gungnir.hpp"

#include "catch.hpp"

SCENARIO("synchronous dispatch waits for all tasks to finish", "[sync]") {

    gungnir::TaskPool tp;

    GIVEN("a collection of tasks") {

        std::atomic_int count{0};

        std::vector<gungnir::Task> tasks;
        for (int i = 0; i < 1000; ++i) {
            tasks.emplace_back([i, &count] { count += i; });
        }

        WHEN("sync-dispatched") {

            tp.dispatchSync(std::cbegin(tasks), std::cend(tasks));
            int finalCount = count;

            THEN("all tasks finish before dispatch call returns") {

                REQUIRE(finalCount == (0 + 999) * 1000 / 2);
            }
        }
    }

    GIVEN("a collection of tasks that return values") {

        std::atomic_int count{0};

        std::vector<std::function<int()>> tasks;
        for (int i = 0; i < 1000; ++i) {
            tasks.emplace_back([i, &count] { count += i; return i; });
        }

        WHEN("sync-dispatched") {

            auto v = tp.dispatchSync<int>(std::cbegin(tasks), std::cend(tasks));
            int finalCount = count;
            
            THEN("all tasks finish before dispatch call returns") {

                REQUIRE(finalCount == (0 + 999) * 1000 / 2);
                REQUIRE(std::accumulate(std::cbegin(v), std::cend(v), 0)
                        == (0 + 999) * 1000 / 2);
            }
        }
    }
}
