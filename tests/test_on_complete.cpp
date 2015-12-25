#include <condition_variable>
#include <exception>
#include <future>
#include <mutex>
#include <string>

#include "gungnir/gungnir.hpp"

#include "catch.hpp"

SCENARIO("onComplete invokes the first callback if a value is"
        " successfully extracted from the future, or the second"
        " if an exception is thrown when fulfulling the underlying"
        " promise of the future",
        "[on_complete]") {

    gungnir::TaskPool tp;

    GIVEN("a shared future that succeeds") {

        std::shared_future<std::string> f{
            tp.dispatch<std::string>([] { return "hello"; })};

        WHEN("passed to onComplete") {

            std::mutex m;
            std::condition_variable cv;
            std::string s;

            gungnir::onComplete(f, [&m, &cv, &s](const std::string &result) {
                std::unique_lock<std::mutex> lk{m};
                s = result;
                cv.notify_all();
            }, [](std::exception_ptr) {
            });

            THEN("the first callback is invoked") {

                std::unique_lock<std::mutex> lk{m};
                cv.wait(lk, [&s] { return !s.empty(); });

                REQUIRE(s == "hello");
            }
        }
    }

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

            gungnir::onComplete(f, [](const std::string &) {
            }, [&m, &cv, &s](std::exception_ptr eptr) {
                try {
                    std::rethrow_exception(eptr);
                } catch (const std::exception &e) {
                    std::unique_lock<std::mutex> lk{m};
                    s = e.what();
                    cv.notify_all();
                }
            });

            THEN("the second callback is invoked") {

                std::unique_lock<std::mutex> lk{m};
                cv.wait(lk, [&s] { return !s.empty(); });

                REQUIRE(s == "world");
            }
        }
    }
}

