# Gungnir

[![Build Status](https://travis-ci.org/ZizhengTai/gungnir.svg?branch=master)](https://travis-ci.org/ZizhengTai/gungnir)
[![License](https://img.shields.io/badge/license-Apache_2.0-blue.svg)](LICENSE)
[![Join the chat at https://gitter.im/ZizhengTai/gungnir](https://badges.gitter.im/ZizhengTai/gungnir.svg)](https://gitter.im/ZizhengTai/gungnir?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Gungnir is a modern and easy-to-use C++ library for concurrent task scheduling, inspired by the [Grand Central Dispatch (GCD)](https://developer.apple.com/library/mac/documentation/Performance/Reference/GCD_libdispatch_Ref/index.html) framework for Objective-C/Swift. It is made with speed, ease of use, and full compatibility with the C++11 standard in mind.

Features:
* Designed to be used with C++11's [callable objects](http://en.cppreference.com/w/cpp/concept/Callable) and [lambda expressions](http://en.cppreference.com/w/cpp/language/lambda).
* Strives for speed and efficiency with thread pools.
* Reduces boilerplate code needed to manage threads, task queues, etc.
* Light-weight header-only library.

(Gungnir is Odin's powerful magical spear that always hits its mark. This library is named after the spear, in the hope of having the same power and reliability.)

## Installation

Gungnir is a header-only library, so no linking is required. Just drop it in your project.

## Usage

The task pool functionality is implemented by the `gungnir::TaskPool` class. You can submit tasks to be executed to a task pool with one of the `dispatch` member functions of the `gungnir::TaskPool` class:

```cpp
// using std::future;
// using std::once_flag;
// using std::vector;
// template <typename R> using Task = std::function<R()>;

void              dispatch(const Task<void> &task);
future<R>         dispatch(const Task<R> &task);
void              dispatch(Iter first, Iter last);
vector<future<R>> dispatch(Iter first, Iter last);

void              dispatchOnce(once_flag &flag, const Task<void> &task);

void              dispatchSerial(Iter first, Iter last);
vector<future<R>> dispatchSerial(Iter first, Iter last);

void              dispatchSync(Iter first, Iter last);
vector<R>         dispatchSync(Iter first, Iter last);
```

Some utility functions in the `gungnir` namespace make `std::future` and `std::shared_future` easier to work with:

```cpp
// using std::shared_future;

void onSuccess(const shared_future<R> &future, const S &callback);
void onFailure(const shared_future<R> &future, const F &callback);
void onComplete(const shared_future<R> &future, const S &success, const F &failure);
```

## Credits

Thanks to [Cameron](http://moodycamel.com/) for the blazing fast [moodycamel::ConcurrentQueue](https://github.com/cameron314/concurrentqueue).

## License

This project is licensed under the Apache License, Version 2.0. See the [LICENSE](./LICENSE) file for details.
