#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "gungnir/gungnir.hpp"

#include "catch.hpp"

SCENARIO("dispatchOnce executes task exactly once, "
        "even if called from different threads", "[once]") {

    GIVEN("a task") {

        std::atomic<int> x{0};
        auto task = [&x] { x += 42; };

        WHEN("passed to dispatchOnce from multiple threads") {

            {
                gungnir::TaskPool tp{8};
                std::once_flag flag;
                std::vector<std::thread> threads;
                for (int i = 0; i < 16; ++i) {
                    threads.emplace_back([&] { tp.dispatchOnce(flag, task); });
                }
                for (auto &t: threads) {
                    t.join();
                }
            }

            THEN("the task gets executed exactly once") {

                REQUIRE(x == 42);
            }
        }
    }
}

