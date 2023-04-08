/*
* Life.cpp, 8/7/2020 10:32 PM
*/

/* Life component offsets */

static FieldOffsets life_component_offsets {
    {"maximum_life",          0x1a4},
    {"life",                  0x1a8},
    {"reserved_life_flat",    0x188},
    {"reserved_life_percent", 0x18c},
    {"maximum_mana",          0x1f4},
    {"mana",                  0x1f8},
    {"reserved_mana_flat",    0x1d8},
    {"reserved_mana_percent", 0x1dc},
    {"maximum_energy_shield", 0x22c},
    {"energy_shield",         0x230},
};

class Life : public Component {
public:

    std::map<wstring, Buff> buffs;
    int last_check_buff = 0;

    Life(addrtype address) : Component(address, "Life", &life_component_offsets) {
    }

    int life(int* maximum = 0, int* reserved = 0) {
        if (maximum) {
            *maximum = read<int>("maximum_life");;
            if (reserved) {
                float reserved_percent = read<int>("reserved_life_percent") / 100.;
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
                float reserved_percent = read<int>("reserved_mana_percent") / 100.;
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

    void to_print() {
        Component::to_print();
        printf("\t\t\t! %d", life());
    }
};
