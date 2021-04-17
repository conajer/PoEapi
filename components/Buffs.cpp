/*
* Buffs.cpp, 4/17/2021 11:21 AM
*/

#include <unordered_set>

static FieldOffsets buff_offsets {
    {"name",      0x0},
};

class Buff : public RemoteMemoryObject {
protected:

    wstring buff_name;

public:
    
    Buff(addrtype address) : RemoteMemoryObject(address, &buff_offsets) {
    }

    wstring& name() {
        if (buff_name.empty())
            buff_name = PoEMemory::read<wstring>(address + (*offsets)["name"], 64);
        return buff_name;
    }

    void to_print() {
        wprintf(L"    %llx: %S\n", address, name().c_str());
    }
};

/* Buffs component offsets */

static FieldOffsets buffs_component_offsets {
    {"buffs",     0xd8},
        {"root",  0x60},
        {"count", 0x68},
};
class Buffs : public Component {
public:

    std::map<wstring, shared_ptr<Buff>> buffs;
    std::queue<addrtype> nodes;
    std::unordered_set<addrtype> temp_set;

    Buffs(addrtype address) : Component(address, "Buffs", &buffs_component_offsets) {
    }

    std::map<wstring, shared_ptr<Buff>>& get_buffs() {
        buffs.clear();
        temp_set.clear();
        
        addrtype root = read<addrtype>("buffs", "root");
        nodes.push(root);
        temp_set.insert(root);
        while (!nodes.empty()) {
            addrtype node = nodes.front();
            nodes.pop();

            for (int offset : (int[]){0x0, 0x8, 0x10}) {
                addrtype address = PoEMemory::read<addrtype>(node + offset);
                if (temp_set.count(address) == 0 && temp_set.size() < 128) {
                    nodes.push(address);
                    temp_set.insert(address);
                }
            }

            if (node == root)
                continue;

            addrtype address = PoEMemory::read<addrtype>(node + 0x20);
            if (address < (addrtype)0x10000000 || address > (addrtype)0x7F0000000000)
                continue;

            shared_ptr<Buff> buff = shared_ptr<Buff>(new Buff(address));
            buffs[buff->name()] = buff;
        }

        return buffs;
    }

    bool has_buff(wstring name) {
        get_buffs();
        return buffs.find(name) != buffs.end();
    }

    void list_buffs() {
        get_buffs();
        if (buffs.empty()) {
            printf("No buffs.\n");
            return;
        }

        wprintf(L"%llx: Buffs\n", read<addrtype>("buff"));
        for (auto& b : buffs) {
            b.second->to_print();
        }
    }
};
