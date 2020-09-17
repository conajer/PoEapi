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

class Task {
private:

    static std::unordered_map<UINT, std::function<void()>> all_jobs;

    static void dispatcher_proc(UINT job_id) {
        all_jobs[job_id]();
    }

protected:

    std::map<UINT, UINT> jobs;
    int owner_thread_id;
    HANDLE stopped_event;
    buffer<wchar_t> log_buffer;

    int start_job(UINT delay, std::function<void ()> func) {
        TIMECAPS tc;
        timeGetDevCaps(&tc, sizeof(TIMECAPS));
        UINT resolution = std::min(std::max(tc.wPeriodMin, delay / 3), tc.wPeriodMax);
        timeBeginPeriod(resolution);
        int job_id = timeSetEvent(delay, resolution, (LPTIMECALLBACK)dispatcher_proc, 0, TIME_PERIODIC);

        if (job_id) {
            jobs.insert(std::make_pair(job_id, resolution));
            all_jobs.insert(std::make_pair(job_id, func));

            log(L"new job @%x, delay %dms", job_id, delay);
        }

        return job_id;
    }

    void stop_job(int job_id) {
        auto i = jobs.find(job_id);
        if (i != jobs.end()) {
            timeKillEvent(job_id);
            timeEndPeriod(i->second);

            /* remove from global task id list */
            all_jobs.erase(job_id);
            
            log(L"job @%x stopped", job_id);
        }
    }

public:

    Task() : log_buffer(128) {
        owner_thread_id = GetCurrentThreadId();
    }

    virtual ~Task() {
        stop();
    }

    virtual void run() {
        stop();
    }

    virtual bool start() {
        if (jobs.empty()) {
            stopped_event = CreateEvent(0, true, false, 0);
            std::thread t(&Task::run, std::ref(*this));
            t.detach();

            return true;
        }

        return false;
    }

    void join(int milliseconds = INFINITE) {
        WaitForSingleObject(stopped_event, milliseconds);
    }

    void stop() {
        for (auto i = jobs.begin(); i != jobs.end();) {
            stop_job(i->first);
            i = jobs.erase(i);
        }
        SetEvent(stopped_event);
    }

    void log(const wchar_t* format, ...) {
        va_list args;
        wchar_t* buffer = log_buffer;

        buffer += swprintf(buffer, L"[PoEapi] ");
        va_start(args, format);
        vswprintf(buffer, format, args);
        va_end(args);
        PostThreadMessage(owner_thread_id, WM_POEAPI_LOG, (WPARAM)log_buffer, 0);

        std::wcout << (const wchar_t*)log_buffer << std::endl;
    }
};

std::unordered_map<UINT, std::function<void()>> Task::all_jobs;
