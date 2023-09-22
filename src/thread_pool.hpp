#include <bits/types/sig_atomic_t.h>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
    private:
        std::queue<std::function<void()>> work_queue;
        std::vector<std::thread> threads;
        sig_atomic_t interrupt;
        std::mutex queue_lock;
        void thread_loop();
    public:
        ThreadPool();
        void queue_job(std::function<void()>);
        void teardown();
};