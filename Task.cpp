/*
* Task.cpp, 8/17/2020 11:05 PM
*/

#include <windows.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <thread>
#include <unordered_map>
#include <queue>

template <typename T> class buffer {
protected:

    std::queue<shared_ptr<T>> pool;
    std::queue<shared_ptr<T>> requested;
    int default_size;
    DWORD last_use_time;

public:

    buffer(int size) : default_size(size), last_use_time(0) {
    }

    operator T*() {
        if (pool.empty()) {
            if (requested.size() && GetTickCount() - last_use_time > 1000)
                pool.swap(requested);
            else
                pool.push(shared_ptr<T>(new T[default_size]));
        }

        shared_ptr<T> t = pool.front();
        pool.pop();

        requested.push(t);
        last_use_time = GetTickCount();

        return t.get();
    }

    operator const T*() {
        return (requested.size() > 0) ? requested.back().get() : nullptr;
    }

    operator __int64() {
        return (requested.size() > 0) ? (__int64)requested.back().get() : 0;
    }
};

class Job {
private:

    HANDLE join_event;
    HANDLE stop_event;
    bool stopped = true;

    void run() {
        ResetEvent(join_event);
        id = GetCurrentThreadId();
        stopped = false;
        ResetEvent(stop_event);
        while (!stopped) {
            func();
            WaitForSingleObject(stop_event, delay);
        }
        SetEvent(join_event);
    }

public:

    wstring name;
    UINT id;
    UINT delay;
    std::function<void()> func;

    Job(wstring name, UINT delay, std::function<void()> func)
        : name(name), delay(delay), func(func)
    {
        join_event = CreateEvent(nullptr, true, false, nullptr);
        stop_event = CreateEvent(nullptr, true, false, nullptr);
    }

    ~Job() {
        stop();
    }

    void start() {
        if (!stopped)
            return;

        std::thread t(&Job::run, std::ref(*this));
        t.detach();
    }

    void join() {
        if (!stopped)
            WaitForSingleObject(join_event, INFINITE);
    }

    void stop() {
        stopped = true;
        SetEvent(stop_event);
    }
};

class Task {
protected:

    std::map<wstring, shared_ptr<Job>> jobs;
    int owner_thread_id;
    HANDLE stop_event;
    buffer<wchar_t> log_buffer;

public:

    wstring name;

    Task(wstring name) : name(name), log_buffer(128) {
        owner_thread_id = GetCurrentThreadId();
        stop_event = CreateEvent(nullptr, true, true, nullptr);
    }

    virtual ~Task() {
        stop();
        join();
    }

    void add_job(shared_ptr<Job>& job) {
        jobs[job->name] = job;
    }

    void add_job(wstring name, UINT delay, std::function<void ()> func) {
        jobs[name] = shared_ptr<Job>(new Job(name, delay, func));
    }

    virtual void run() {
        if (!jobs.empty()) {
            for (auto& i : jobs) {
                shared_ptr<Job>& job = i.second;
                if (job->delay > 0) {
                    job->start();
                    Sleep(0);
                    log(L"new job %S@%x, delay %d ms", job->name.c_str(), job->id, job->delay);
                }
            }

            // wait for the jobs to finish
            for (auto& i : jobs)
                i.second->join();
            SetEvent(stop_event);
        }
    }

    virtual bool start() {
        ResetEvent(stop_event);
        std::thread t(&Task::run, std::ref(*this));
        t.detach();

        return true;
    }

    void join(int milliseconds = INFINITE) {
        WaitForSingleObject(stop_event, milliseconds);
    }

    void stop() {
        for (auto& i : jobs) {
            shared_ptr<Job>& job = i.second;
            job->stop();

            log(L"job %S@%x stopped", job->name.c_str(), job->id);
        }
    }

    void log(const wchar_t* format, ...) {
        va_list args;
        wchar_t* buffer = log_buffer;

        buffer += swprintf(buffer, L"[%S] ", this->name.c_str());
        va_start(args, format);
        vswprintf(buffer, format, args);
        va_end(args);
        PostThreadMessage(owner_thread_id, WM_POEAPI_LOG, (WPARAM)log_buffer, 0);

        std::wcout << (const wchar_t*)log_buffer << std::endl;
    }
};
