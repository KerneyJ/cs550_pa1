#include <bits/types/sig_atomic_t.h>
#include <functional>
#include <unistd.h>
#include <vector>
#include <thread>
#include <cstdio>
#include "thread_pool.hpp"

ThreadPool::ThreadPool() {
    int i, num_threads;

    interrupt = false;
    num_threads = 3;//std::thread::hardware_concurrency(); 

    printf("Creating %d threads\n", num_threads);

    for(i = 0; i < num_threads; i++) {
        threads.emplace_back(std::thread(&ThreadPool::thread_loop, this));
    }
}

void ThreadPool::queue_job(std::function<void()> job) {
    printf("Adding job\n");
    queue_lock.lock();
    work_queue.push(job);
    queue_lock.unlock();
}

void ThreadPool::teardown() {
    printf("Tearing down\n");

    interrupt = true;
    for(std::thread &thread : threads) {
        thread.join();
    }
    threads.clear();
    while(!work_queue.empty())
        work_queue.pop();
}

void ThreadPool::thread_loop() {
    std::function<void()> job;

    while(!interrupt) {
        queue_lock.lock();
        if(work_queue.empty()) {
            sleep(1);
            queue_lock.unlock();
            continue;
        }

        printf("I got the job!\n");
        job = work_queue.front();
        work_queue.pop();
        queue_lock.unlock();

        printf("Starting job!\n");
        job();
    }
}