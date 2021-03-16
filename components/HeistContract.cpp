/*
* HeistContract.cpp, 12/2/2020 11:59 PM
*/

/* HeistContract component offsets */

static std::map<string, int> heist_contract_component_offsets {
    {"jobs",      0x30},
};

class HeistContract : public Component {
private:

    AhkObjRef* __get_jobs() {
        AhkObj jobs;
        for (auto& i : get_jobs())
            jobs.__set(L"", (AhkObjRef*)*i, AhkObject, nullptr);
        __set(L"jobs", (AhkObjRef*)jobs, AhkObject, nullptr);
        return jobs;
    }

public:

    std::vector<shared_ptr<HeistJob>> jobs;

    HeistContract(addrtype address)
        : Component(address, "HeistContract", &heist_contract_component_offsets)
    {
        add_method(L"getJobs", this, (MethodType)&HeistContract::__get_jobs, AhkObject);
    }

    std::vector<shared_ptr<HeistJob>>& get_jobs() {
        if (jobs.empty()) {
            for (auto addr : read_array<addrtype>("jobs", 0x18)) {
                HeistJob* job = new HeistJob(PoEMemory::read<addrtype>(addr + 0x8));
                job->level = PoEMemory::read<byte>(addr + 0x10);
                jobs.push_back(shared_ptr<HeistJob>(job));
            }
        }
        return jobs;
    }
};
