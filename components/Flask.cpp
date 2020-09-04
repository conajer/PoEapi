/*
* Flask.cpp, 8/11/2020 9:04 PM
*/

/* Flask component offsets */

static std::map<string, int> flask_component_offsets {
    {"internal",            0x28},
        {"base",            0x18},
            {"life_per_use", 0x4},
            {"mana_per_use", 0xc},
            {"duration",    0x14},
};

class Flask : public Component {
public:

    Flask(addrtype address) : Component(address, "Flask", &flask_component_offsets) {
    }

    int life() {
        return PoEMemory::read<int>(address + 0x28, {0x18, 0x4});
    }

    int mana() {
        return PoEMemory::read<int>(address + 0x28, {0x18, 0xc});
    }

    int duration() {
        return PoEMemory::read<int>(address + 0x28, {0x18, 0x14});
    }

    void to_print() {
        Component::to_print();
        printf("\t\t\t! %d", duration());
    }
};
