/*
* Life.cpp, 8/7/2020 10:32 PM
*/

/* Life component offsets */

static FieldOffsets life_component_offsets {
    {"maximum_life",          0x24c},
    {"life",                  0x250},
    {"reserved_life_flat",    0x254},
    {"reserved_life_percent", 0x258},
    {"maximum_mana",          0x1b4},
    {"mana",                  0x1b8},
    {"reserved_mana_flat",    0x1bc},
    {"reserved_mana_percent", 0x1c0},
    {"maximum_energy_shield", 0x1ec},
    {"energy_shield",         0x1f0},
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
