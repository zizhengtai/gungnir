#include <mutex>
#include <vector>

#include "gungnir/gungnir.hpp"

#include "catch.hpp"

SCENARIO("serial dispatch maintains order of tasks", "[serial]") {

    GIVEN("an ordered sequence of tasks") {

        std::vector<int> v;
        std::mutex m;

        std::vector<gungnir::Task> tasks;
        for (int i = 0; i < 1000; ++i) {
            tasks.emplace_back([i, &v, &m] {
                std::unique_lock<std::mutex> lk(m);
                v.emplace_back(i);
            });
        }

        WHEN("serial-dispatched") {

            gungnir::TaskPool{8}.dispatchSerial(tasks.cbegin(), tasks.cend());

            THEN("they are executed in the identical order") {

                REQUIRE(v.size() == 1000);

                bool matched = true;
                for (int i = 0; i < 1000; ++i) {
                    if (v[i] != i) {
                        matched = false;
                        break;
                    }
                }
                REQUIRE(matched);
            }
        }
    }
}
