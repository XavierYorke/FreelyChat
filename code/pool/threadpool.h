/*
 * @Author  :   XavierYorke 
 * @Contact :   mzlxavier1230@gmail.com
 * @Time    :   2023-07-08
 */

#pragma once
#include <assert.h>

#include <condition_variable>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

using namespace std;

class ThreadPool {
public:
    explicit ThreadPool(size_t thread_num = 8) : _pool(make_shared<Pool>()) {
        assert(thread_num > 0);
        for (size_t i = 0; i < thread_num; ++i) {
            thread([pool = _pool] {
                unique_lock<mutex> locker(pool->mtx);
                while (true) {
                    if (!pool->tasks.empty()) {
                        auto task = pool->tasks.front();
                        pool->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    }
                    else if (pool->isClosed) break;
                    else pool->cond.wait(locker);
                }
            }).detach();
        }
    }

    ThreadPool(ThreadPool&&) = default;

    ~ThreadPool() {
        if (static_cast<bool>(_pool)) {
            lock_guard<mutex> locker(_pool->mtx);
            _pool->isClosed = true;
        }
        _pool->cond.notify_all();
    }

    template <typename T>
    void addTask(T&& task) {
        lock_guard<mutex> locker(_pool->mtx);
        _pool->tasks.emplace(forward<T>(task));
        _pool->cond.notify_one();
    }

private:
    struct Pool {
        bool isClosed;
        mutex mtx;
        condition_variable cond;
        queue<function<void()>> tasks;
    };
    shared_ptr<Pool> _pool;
};