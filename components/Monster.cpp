/*
* Monster.cpp, 8/11/2020 2:28 PM
*/

/* Monster component offsets */

static std::map<string, int> monster_component_offsets {
    {"internal",      0x18},
        {"base",      0x18},
            {"name", 0x104},
};

class Monster : public Component {
protected:

    wstring base_name;

public:

    Monster(addrtype address) : Component(address, "Monster", &monster_component_offsets) {
    }

    wstring& name() {
        if (base_name.empty()) {
            addrtype base = read<addrtype>("internal", "base");
            base_name = PoEMemory::read<wstring>(base + (*offsets)["name"], 32);
        }

        return base_name;
    }

    void to_print() {
        Component::to_print();
        printf("\t\t\t! %S", name().c_str());
    }
};
