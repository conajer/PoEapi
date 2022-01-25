/*
*  PoE.cpp, 10/16/2021 4:46 PM
*/

#include <mutex>
#include <condition_variable>
#include <thread>

template <typename T>
class Parallel {
private:

    void init_tasks() {
        is_active = max_threads;
        task_finished_event = CreateEvent(nullptr, true, false, nullptr);
        for (int i = 0; i < max_threads; ++i) {
            std::thread t(&Parallel::task_routine, std::ref(*this));
            t.detach();
            n_threads++;
        }

        n_threads++;
        run();
        n_threads--;
    }

    void task_routine() {
        while (1) {
            T task;
            {
                std::unique_lock<std::mutex> lock(task_queue_mutex);
                is_active--;
                if (!is_active && task_queue.empty())
                    SetEvent(task_finished_event);

                cv.wait(lock, [this]() {
                    return task_quit || !task_queue.empty();
                });

                if (task_quit)  {
                    n_threads--;
                    return;
                }

                is_active++;
                task = task_queue.front();
                task_queue.pop();
            }
            foreach(task);
        }
    }

public:

    std::vector<std::thread> pool;
    std::mutex task_queue_mutex;
    std::queue<T> task_queue;
    std::condition_variable cv;
    HANDLE task_finished_event;
    bool task_quit = false;
    int max_threads;
    int is_active = 0;
    int n_threads = 0;

    Parallel(int n = 4) : max_threads(n) {
    }

    virtual ~Parallel() {
        stop();
    }

    void add_task(T task) {
        std::unique_lock<std::mutex> lock(task_queue_mutex);
        task_queue.push(task);
        ResetEvent(task_finished_event);
        cv.notify_one();
    }

    virtual void foreach(T& task) {
    }

    virtual void run() {
    }

    bool is_quit() {
        return task_quit;
    }

    void start() {
        if (n_threads > 0)
            return; // already started

        std::thread t(&Parallel::init_tasks, std::ref(*this));
        t.detach();
    }

    void stop() {
        {
            std::unique_lock<std::mutex> lock(task_queue_mutex);
            task_quit = true;
            cv.notify_all();
        }

        SetEvent(task_finished_event);
        while (n_threads > 0)
            SwitchToThread();
    }

    void wait() {
        WaitForSingleObject(task_finished_event, INFINITE);
    }
};
