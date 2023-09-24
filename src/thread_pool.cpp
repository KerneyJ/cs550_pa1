#include <bits/types/sig_atomic_t.h>
#include <condition_variable>
#include <functional>
#include <unistd.h>
#include <vector>
#include <thread>
#include <cstdio>
#include "thread_pool.hpp"

ThreadPool::ThreadPool() {
    int i, num_threads;

    interrupt = false;
    num_threads = std::thread::hardware_concurrency(); 

    printf("Creating %d threads\n", num_threads);

    for(i = 0; i < num_threads; i++) {
        threads.emplace_back(std::thread(&ThreadPool::thread_loop, this));
    }
}

void ThreadPool::queue_job(std::function<void()> job) {
    printf("Adding job to threadpool queue.\n");

    queue_lock.lock();
    work_queue.push(job);
    queue_lock.unlock();
    notify_work.notify_one();
}

void ThreadPool::teardown() {
    printf("Tearing down threadpool, joining all threads.\n");

    interrupt = true;
    notify_work.notify_all();
    for(std::thread &thread : threads) {
        thread.join();
    }
    threads.clear();
    while(!work_queue.empty())
        work_queue.pop();
}

void ThreadPool::thread_loop() {
    std::function<void()> job;

    while(true) {
        std::unique_lock work_lock(queue_lock);
        notify_work.wait(work_lock, [this]{ 
            return !work_queue.empty() || interrupt;
        });

        if(interrupt) {
            work_lock.unlock();
            return;
        }

        job = work_queue.front();
        work_queue.pop();
        work_lock.unlock();

        job();
    }
}