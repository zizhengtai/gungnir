#include <condition_variable>
#include <exception>
#include <future>
#include <mutex>
#include <string>

#include "gungnir/gungnir.hpp"

#include "catch.hpp"

SCENARIO("onSuccess invokes the callback if a value is"
        " successfully extracted from the future",
        "[on_success]") {

    gungnir::TaskPool tp;

    GIVEN("a shared future that succeeds") {

        std::shared_future<std::string> f{
            tp.dispatch<std::string>([] { return "hello"; })};

        WHEN("passed to onSuccess") {

            std::mutex m;
            std::condition_variable cv;
            std::string s;

            gungnir::onSuccess(f, [&m, &cv, &s](const std::string &result) {
                std::unique_lock<std::mutex> lk{m};
                s = result;
                cv.notify_all();
            });

            THEN("the callback is invoked") {

                std::unique_lock<std::mutex> lk{m};
                cv.wait(lk, [&s] { return !s.empty(); });

                REQUIRE(s == "hello");
            }
        }
    }
}

