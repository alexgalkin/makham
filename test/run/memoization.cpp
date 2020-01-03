/**
    Copyright 2020 Red Anchor Trading Co. Ltd.

    Distributed under the Boost Software License, Version 1.0.
    See <http://www.boost.org/LICENSE_1_0.txt>
 */


#include <iostream>

#include <fost/test>
#include <f5/makham/async.hpp>
#include <f5/makham/future.hpp>
#include <f5/makham/multi.hpp>

#include <chrono>


using namespace std::chrono_literals;


namespace {
    f5::makham::async<int> answer() {
        std::cout << "Answering 42" << std::endl;
        co_return 42;
    }
}


FSL_TEST_SUITE(memoization);


FSL_TEST_FUNCTION(awaiting_apart) {
    std::cout << "\n\nStarting awaiting_apart" << std::endl;

    auto as = answer();
    std::cout << "Got async.... Going to wrap in multi" << std::endl;
    std::this_thread::sleep_for(100ms);

    f5::makham::multi<f5::makham::async<int>> a = std::move(as);
    std::cout << "Got multi... Going to wrap in future and wait" << std::endl;
    std::this_thread::sleep_for(100ms);

    FSL_CHECK_EQ(f5::makham::future<int>::wrap(a).get(), 42);
    FSL_CHECK_EQ(f5::makham::future<int>::wrap(a).get(), 42);
}


FSL_TEST_FUNCTION(awaiting_together) {
    std::cout << "\n\nStarting awaiting_together" << std::endl;

    f5::makham::multi<f5::makham::async<int>> a{answer()};
    std::cout << "Got async and multi... Going to wrap in future and wait"
              << std::endl;
    std::this_thread::sleep_for(100ms);

    FSL_CHECK_EQ(f5::makham::future<int>::wrap(a).get(), 42);
    FSL_CHECK_EQ(f5::makham::future<int>::wrap(a).get(), 42);
}


FSL_TEST_FUNCTION(awaiting_without_pause) {
    std::cout << "\n\nStarting awaiting_without_pause" << std::endl;

    f5::makham::multi<f5::makham::async<int>> a{answer()};
    std::cout << "Got async and multi... Going to wrap in future and wait"
              << std::endl;

    FSL_CHECK_EQ(f5::makham::future<int>::wrap(a).get(), 42);
    FSL_CHECK_EQ(f5::makham::future<int>::wrap(a).get(), 42);
}
