#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threadsNumber) {
    Start(threadsNumber);
}

ThreadPool::~ThreadPool() {
    Stop();
}

void ThreadPool::Start(size_t threadNumber) {
    for (auto i = 0; i < threadNumber; i++) {
        threads.emplace_back([=] {
            while (true) {
                Task task;
                {
                    std::unique_lock<std::mutex> lock(eventMutex);
                    eventVar.wait(lock, [=] { return isStopping || !tasks.empty(); });

                    if (isStopping) {
                        break;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

void ThreadPool::Stop() noexcept {
    {
        std::unique_lock<std::mutex> lock(eventMutex);
        isStopping = true;
    }

    eventVar.notify_all();

    for (auto &thread: threads) {
        thread.join();
    }
}
