#include <cstddef>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <future>
#include <unordered_map>

class ThreadPool {
public:
    using Task = std::function<void()>;

    explicit ThreadPool(size_t threadsNumber);

    ~ThreadPool();

    template<class T>
    auto Enqueue(T task) -> std::future<decltype(task())> {
        auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));
        {
            std::unique_lock<std::mutex> lock(eventMutex);
            tasks.emplace([=] {
                (*wrapper)();
            });
        }

        eventVar.notify_one();
        return wrapper->get_future();
    }

private:
    void Start(size_t threadNumber);

    void Stop() noexcept;

    std::vector<std::thread> threads;
    std::queue<Task> tasks;
    std::condition_variable eventVar;
    std::mutex eventMutex;
    bool isStopping = false;
};

