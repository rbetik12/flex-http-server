#include <Log.h>
#include "ThreadPool.h"
#include <sstream>

ThreadPool::ThreadPool(size_t threadsNumber) {
    Start(threadsNumber);
}

ThreadPool::~ThreadPool() {
    Stop();
}

void ThreadPool::Start(size_t threadNumber) {
    for (auto i = 0; i < threadNumber; i++) {
        threads.emplace_back([=] {
            std::stringstream ss;
            ss << std::this_thread::get_id();
            TRACE("Started worker thread with id {0}", ss.str().c_str());
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
            TRACE("Stopped worker thread with id {0}", ss.str().c_str());
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
