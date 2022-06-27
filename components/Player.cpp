/*
* Player.cpp, 8/8/2020 11:10 AM
*/

/* Player component offsets */

static std::map<string, int> player_component_offsets {
    {"class", 0x158},
    {"name",  0x160},
    {"exp",   0x184},
    {"level", 0x1a3},
};

class Player : public Component {
public:

    Player(addrtype address) : Component(address, "Player", &player_component_offsets) {
    }

    void __new() {
        Component::__new();
        __set(L"name", name().c_str(), AhkWString,
              L"class", class_name().c_str(), AhkWString,
              L"level", level(), AhkInt,
              nullptr);
    }

    wstring name() {
        return read<wstring>("name");;
    }

    wstring class_name() {
        addrtype addr = read<addrtype>("class");
        return PoEMemory::read<wstring>(addr + 0x10, 16);
    }

    unsigned long exp() {
        return read<unsigned long>("exp");
    }

    int level() {
        return read<byte>("level");
    }

    void to_print() {
        Component::to_print();
        wprintf(L"\t\t\t! %S, %d", name().c_str(), level());
    }
};
