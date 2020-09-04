/*
* ObjectMagicProperties.cpp, 8/11/2020 1:53 PM
*/

/* ObjectMagicProperties component offsets */

static std::map<string, int> objectmagicproperties_component_offsets {
    {"unique_name", 0x20},
    {"rarity",      0x9c},
    {"mods",        0xb8},
    {"stats",      0x1c8},
};

class ObjectMagicProperties : public Component {
public:

    /* Modifiers */
    std::vector<Modifier> mods;

    ObjectMagicProperties(addrtype address)
        : Component(address, "ObjectMagicProperties", &objectmagicproperties_component_offsets)
    {
    }

    int rarity() {
        return read<byte>("rarity");
    }

    void list_mods() {
        printf("\n");
        for (auto i : read_array<Modifier>("mods", 0x20, 0x28))
            i.to_print();
    }

    void to_print() {
        const char* rarity_names[] = {"Normal", "Magic", "Rare", "Unique"};

        Component::to_print();
        printf("\t! %s", rarity_names[rarity()]);

        if (rarity() > 0) {
            for (auto i : read_array<addrtype>("unique_name", 0x8, 16))
                wprintf(L"%S", PoEMemory::read<wstring>(i + 0x30, 32).c_str());
            list_mods();
        }
    }
};
