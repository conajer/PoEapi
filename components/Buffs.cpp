/*
* Buffs.cpp, 4/17/2021 11:21 AM
*/

#include <unordered_set>

static FieldOffsets buff_offsets {
    {"internal",        0x8},
        {"name",        0x0},
        {"description", 0x8},
    {"duration",       0x18},
    {"timer",          0x1c},
    {"charges",        0x42},
};

class Buff : public RemoteMemoryObject {
public:

    addrtype internal;
    wstring buff_name;
    
    Buff(addrtype address) : RemoteMemoryObject(address, &buff_offsets) {
        internal = read<addrtype>("internal");
    }

    wstring& name() {
        if (buff_name.empty())
            buff_name = PoEMemory::read<wstring>(internal + (*offsets)["name"], 64);

        return buff_name;
    }

    wstring description() {
        return PoEMemory::read<wstring>(internal + (*offsets)["description"], 256);
    }

    int charges() {
        return read<short>("charges");
    }

    float duration() {
        return read<float>("duration");
    }

    float timer() {
        return read<float>("timer");
    }

    void to_print() {
        wprintf(L"    %llx: %4d, %4.2f, %4.2f, %S\n",
                address, charges(), duration(), timer(), name().c_str());
    }
};

/* Buffs component offsets */

static FieldOffsets buffs_component_offsets {
    {"buffs",     0x160},
};

class Buffs : public Component {
public:

    std::map<wstring, Buff> buffs;
    std::queue<addrtype> nodes;
    std::unordered_set<addrtype> temp_set;
    int last_checking = 0;

    Buffs(addrtype address) : Component(address, "Buffs", &buffs_component_offsets) {
    }

    std::map<wstring, Buff>& get_buffs() {
        if (GetTickCount() - last_checking > 300 || buffs.empty()) {
            buffs.clear();
            for (auto& buff : read_array<Buff>("buffs", 0x0, 8))
                buffs.insert(std::make_pair(buff.name(), buff));
            last_checking = GetTickCount();
        }

        return buffs;
    }

    int has_buff(wstring name) {
        auto i = get_buffs().find(name);
        return (i != buffs.end()) ? i->second.charges() : 0;
    }

    void list_buffs() {
        get_buffs();
        if (buffs.empty()) {
            printf("No buffs.\n");
            return;
        }

        wprintf(L"%llx: Buffs\n", read<addrtype>("buff"));
        for (auto& b : buffs) {
            b.second.to_print();
        }
    }
};
