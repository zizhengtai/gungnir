#include <condition_variable>
#include <exception>
#include <future>
#include <mutex>
#include <string>

#include "gungnir/gungnir.hpp"

#include "catch.hpp"

SCENARIO("onFailure invokes the callback if an exception is"
        " thrown when fulfilling the underlying promise of the future",
        "[on_failure]") {

    gungnir::TaskPool tp;

    GIVEN("a shared future that fails") {

        std::shared_future<std::string> f{
            tp.dispatch<std::string>([] {
                throw std::runtime_error("world");
                return "hello";
            })};

        WHEN("passed to onFailure") {

            std::mutex m;
            std::condition_variable cv;
            std::string s;

            gungnir::onFailure(f, [&m, &cv, &s](std::exception_ptr eptr) {
                try {
                    std::rethrow_exception(eptr);
                } catch (const std::exception &e) {
                    std::unique_lock<std::mutex> lk{m};
                    s = e.what();
                    cv.notify_all();
                }
            });

            THEN("the callback is invoked") {

                std::unique_lock<std::mutex> lk{m};
                cv.wait(lk, [&s] { return !s.empty(); });

                REQUIRE(s == "world");
            }
        }
    }
}
