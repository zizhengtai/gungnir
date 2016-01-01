#include <mutex>
#include <tuple>
#include <utility>
#include <vector>

#include "gungnir/gungnir.hpp"

#include "catch.hpp"

SCENARIO("dispatchSerial maintains order of tasks", "[serial]") {

    GIVEN("an ordered sequence of tasks") {

        std::vector<int> v1, v2, v3, v4;
        std::mutex m1, m2, m3, m4;

        std::vector<gungnir::Task<void>> tasks1, tasks2, tasks3, tasks4;
        for (int i = 0; i < 1000; ++i) {
            tasks1.emplace_back([i, &v1, &m1] {
                std::unique_lock<std::mutex> lk{m1};
                v1.emplace_back(i);
            });
        }
        for (int i = 0; i < 1000; ++i) {
            tasks2.emplace_back([i, &v2, &m2] {
                std::unique_lock<std::mutex> lk{m2};
                v2.emplace_back(i);
            });
        }
        for (int i = 0; i < 1000; ++i) {
            tasks3.emplace_back([i, &v3, &m3] {
                std::unique_lock<std::mutex> lk{m3};
                v3.emplace_back(i);
            });
        }
        for (int i = 0; i < 1000; ++i) {
            tasks4.emplace_back([i, &v4, &m4] {
                std::unique_lock<std::mutex> lk{m4};
                v4.emplace_back(i);
            });
        }

        WHEN("passed to dispatchSerial") {

            {
                gungnir::TaskPool tp{8};

                std::thread t1{[&tp, &tasks1] {
                    tp.dispatchSerial(tasks1.cbegin(), tasks1.cend());
                }};
                tp.dispatchSerial(tasks2.cbegin(), tasks2.cend());
                std::thread t2{[&tp, &tasks3] {
                    tp.dispatchSerial(tasks3.cbegin(), tasks3.cend());
                }};
                tp.dispatchSerial(tasks4.cbegin(), tasks4.cend());

                t1.join();
                t2.join();
            }

            THEN("they are executed in the identical order") {

                for (const auto &v: {v1, v2, v3, v4}) {
                    REQUIRE(v.size() == 1000);

                    bool matched = true;
                    for (int i = 0; matched && i < 1000; ++i) {
                        if (v[i] != i) {
                            matched = false;
                        }
                    }
                    REQUIRE(matched);
                }
            }
        }
    }

    GIVEN("an ordered sequence of tasks that return values") {

        std::vector<int> v1, v2, v3, v4;
        std::mutex m1, m2, m3, m4;

        std::vector<gungnir::Task<int>> tasks1, tasks2, tasks3, tasks4;
        for (int i = 0; i < 1000; ++i) {
            tasks1.emplace_back([i, &v1, &m1] {
                std::unique_lock<std::mutex> lk{m1};
                v1.emplace_back(i);
                return i;
            });
        }
        for (int i = 0; i < 1000; ++i) {
            tasks2.emplace_back([i, &v2, &m2] {
                std::unique_lock<std::mutex> lk{m2};
                v2.emplace_back(i);
                return i;
            });
        }
        for (int i = 0; i < 1000; ++i) {
            tasks3.emplace_back([i, &v3, &m3] {
                std::unique_lock<std::mutex> lk{m3};
                v3.emplace_back(i);
                return i;
            });
        }
        for (int i = 0; i < 1000; ++i) {
            tasks4.emplace_back([i, &v4, &m4] {
                std::unique_lock<std::mutex> lk{m4};
                v4.emplace_back(i);
                return i;
            });
        }

        WHEN("passed to dispatchSerial") {

            std::vector<std::future<int>> f1, f2, f3, f4;

            {
                gungnir::TaskPool tp{8};

                std::thread t1{[&tp, &tasks1, &f1] {
                    f1 = tp.dispatchSerial<int>(
                            tasks1.cbegin(), tasks1.cend());
                }};
                f2 = tp.dispatchSerial<int>(tasks2.cbegin(), tasks2.cend());
                std::thread t2{[&tp, &tasks3, &f3] {
                    f3 = tp.dispatchSerial<int>(
                            tasks3.cbegin(), tasks3.cend());
                }};
                f4 = tp.dispatchSerial<int>(tasks4.cbegin(), tasks4.cend());

                t1.join();
                t2.join();
            }

            THEN("they are executed in the identical order") {

                auto fvs = {
                    std::make_pair(std::ref(f1), std::ref(v1)),
                    std::make_pair(std::ref(f2), std::ref(v2)),
                    std::make_pair(std::ref(f3), std::ref(v3)),
                    std::make_pair(std::ref(f4), std::ref(v4)),
                };
                for (auto &fv: fvs) {
                    auto &f = fv.first;
                    auto &v = fv.second;

                    REQUIRE(f.size() == 1000);
                    REQUIRE(v.size() == 1000);

                    bool matched = true;
                    for (int i = 0; matched && i < 1000; ++i) {
                        if (v[i] != i || f[i].get() != i) {
                            matched = false;
                        }
                    }
                    REQUIRE(matched);
                }
            }
        }
    }
}

