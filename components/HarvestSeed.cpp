/*
* HarvestSeed.cpp, 8/10/2020 11:23 PM
*/

/* HarvestSeed component offsets */

static std::map<string, int> harvestseed_component_offsets {
    {"internal",     0x10},
        {"base",     0x18},
            {"tier", 0x48},
    {"level",        0x1c},
};

class HarvestSeed : public Component {
public:

    HarvestSeed(addrtype address)
        : Component(address, "HarvestSeed", &harvestseed_component_offsets)
    {
    }

    int level() {
        return read<byte>("level");
    }

    int tier() {
        return PoEMemory::read<byte>(read<addrtype>("internal", "base") + (*offsets)["tier"]);
    }

    void to_print() {
        Component::to_print();
        printf("\t\t! T%d, %d", tier(), level());
    }
};
