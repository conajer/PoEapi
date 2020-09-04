/*
* Life.cpp, 8/7/2020 10:32 PM
*/

static FieldOffsets buff_offsets {
    {"id",            0x0},
    {"internal",      0x8},
        {"name",      0x8},
        {"charges",  0x30},
        {"duration", 0x10},
        {"timer",    0x14},
};

class Buff : public RemoteMemoryObject {
protected:

    int id;
    wstring buff_name;

public:
    
    Buff(addrtype address) : RemoteMemoryObject(address, &buff_offsets) {
        id = read<byte>("id");
        this->address = read<addrtype>("internal");
    }

    wstring& name() {
        if (buff_name.empty())
            buff_name = PoEMemory::read<wstring>(read<addrtype>("name"), 64);
        return buff_name;
    }

    int charges() {
        return read<byte>("charges");
    }

    float duration() {
        return read<float>("duration");
    }

    float timer() {
        return read<float>("timer");
    }

    void to_print() {
        wprintf(L"    %llx: %2d, %d, %4.2f, %4.2f, %S\n",
                address, id, charges(), duration(), timer(), name().c_str());
    }
};

/* Life component offsets */

static FieldOffsets life_component_offsets {
    {"buff",                  0x180},
    {"maximum_life",          0x254},
    {"reserved_life_flat",    0x258},
    {"life",                  0x25c},
    {"reserved_life_percent", 0x260},
    {"maximum_mana",          0x1bc},
    {"reserved_mana_flat",    0x1c0},
    {"mana",                  0x1c4},
    {"reserved_mana_percent", 0x1c8},
    {"maximum_energy_shield", 0x1f4},
    {"energy_shield",         0x1fc},
};

class Life : public Component {
public:

    std::map<wstring, Buff> buffs;

    Life(addrtype address) : Component(address, "Life", &life_component_offsets) {
    }

    int life(int* maximum = 0, int* reserved = 0) {
        if (maximum) {
            *maximum = read<int>("maximum_life");;
            if (reserved) {
                int reserved_percent = read<int>("reserved_life_percent");
                int reserved_flat = read<int>("reserved_life_flat");
                *reserved = reserved_flat + *maximum * reserved_percent / 100. + 0.99;
            }
        }

        return read<int>("life");
    }

    int mana(int* maximum = 0, int* reserved = 0) {
        if (maximum) {
            *maximum = read<int>("maximum_mana");
            if (reserved) {
                int reserved_percent = read<int>("reserved_mana_percent");
                int reserved_flat = read<int>("reserved_mana_flat");
                *reserved = reserved_flat + *maximum * reserved_percent / 100. + 0.99;
            }
        }

        return read<int>("mana");;
    }

    int energy_shield(int* maximum = 0) {
        if (maximum)
            *maximum = read<int>("maximum_energy_shield");

        return read<int>("energy_shield");;
    }

    std::map<wstring, Buff>& get_buffs() {
        if (buffs.empty()) {
            for (auto buff : read_array<Buff>("buff", 0x0, 8))
                buffs.insert(std::make_pair(buff.name(), buff));
        }

        return buffs;
    }

    void list_buffs() {
        std::vector<Buff> buffs = read_array<Buff>("buff", 0x0, 8);
        if (buffs.empty()) {
            printf("No buffs.\n");
            return;
        }

        wprintf(L"%llx: Buffs\n", read<addrtype>("buff"));
        for (auto b : buffs) {
            b.to_print();
        }
    }

    void to_print() {
        Component::to_print();
        printf("\t\t\t! %d", life());
    }
};
