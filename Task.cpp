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
public:

    wstring name;
    UINT id = 0;
    UINT delay;
    UINT resolution;
    std::function<void()> func;

    Job(wstring name, UINT delay, std::function<void()> func)
        : name(name), delay(delay), func(func)
    {
    }
};

class Task {
private:

    static std::unordered_map<UINT, shared_ptr<Job>> all_jobs;

    static void dispatcher_proc(UINT job_id) {
        all_jobs[job_id]->func();
    }

protected:

    std::map<wstring, shared_ptr<Job>> jobs;
    int owner_thread_id;
    HANDLE stopped_event;
    buffer<wchar_t> log_buffer;

    int start_job(shared_ptr<Job>& job) {
        if (job->delay <= 0)
            return 0;

        TIMECAPS tc;
        timeGetDevCaps(&tc, sizeof(TIMECAPS));
        UINT resolution = std::min(std::max(tc.wPeriodMin, job->delay / 3), tc.wPeriodMax);
        timeBeginPeriod(resolution);
        int job_id = timeSetEvent(job->delay, resolution, (LPTIMECALLBACK)dispatcher_proc, 0, TIME_PERIODIC);

        if (job_id) {
            job->id = job_id;
            job->resolution = resolution;
            all_jobs[job_id] = job;

            log(L"new job %S@%x, delay %d ms", job->name.c_str(), job_id, job->delay);
        }

        return job_id;
    }

    void stop_job(shared_ptr<Job>& job) {
        auto i = jobs.find(job->name);
        if (i != jobs.end()) {
            timeKillEvent(job->id);
            timeEndPeriod(job->resolution);

            /* remove from global task id list */
            all_jobs.erase(job->id);
            
            log(L"job %S@%x stopped", job->name, job->id);
        }
    }

public:

    wstring name;

    Task(wstring name) : name(name), log_buffer(128) {
        owner_thread_id = GetCurrentThreadId();
    }

    virtual ~Task() {
        stop();
    }

    void add_job(shared_ptr<Job>& job) {
        jobs[job->name] = job;
    }

    void add_job(wstring name, UINT delay, std::function<void ()> func) {
        jobs[name] = shared_ptr<Job>(new Job(name, delay, func));
    }

    virtual void run() {
        if (!jobs.empty()) {
            for (auto& i : jobs)
                start_job(i.second);
            join(); /* wait for the jobs finish */
        }
    }

    virtual bool start() {
        stopped_event = CreateEvent(0, true, false, 0);
        std::thread t(&Task::run, std::ref(*this));
        t.detach();

        return true;
    }

    void join(int milliseconds = INFINITE) {
        WaitForSingleObject(stopped_event, milliseconds);
    }

    void stop() {
        for (auto i = jobs.begin(); i != jobs.end();) {
            stop_job(i->second);
            i = jobs.erase(i);
        }
        SetEvent(stopped_event);
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

std::unordered_map<UINT, shared_ptr<Job>> Task::all_jobs;
