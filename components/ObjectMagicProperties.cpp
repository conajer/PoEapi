/*
* ObjectMagicProperties.cpp, 8/11/2020 1:53 PM
*/

/* ObjectMagicProperties component offsets */

static std::map<string, int> objectmagicproperties_component_offsets {
    {"unique_name",  0xc0},
    {"rarity",      0x13c},
    {"mods",        0x160},
};

class ObjectMagicProperties : public Component {
private:

    AhkObjRef* __get_mods() {
        AhkTempObj temp_mods;
        for (auto& i : get_mods())
            temp_mods.__set(L"", (AhkObjRef*)i, AhkObject, nullptr);
        
        return temp_mods;
    }

public:

    /* Modifiers */
    std::vector<Modifier> mods;

    ObjectMagicProperties(addrtype address)
        : Component(address, "ObjectMagicProperties", &objectmagicproperties_component_offsets)
    {
        add_method(L"getMods", this, (MethodType)&ObjectMagicProperties::__get_mods, AhkObject);
    }

    int rarity() {
        return read<byte>("rarity");
    }

    std::vector<Modifier>& get_mods() {
        return mods = read_array<Modifier>("mods", 0x38);
    }

    void to_print() {
        const char* rarity_names[] = {"Normal", "Magic", "Rare", "Unique"};

        Component::to_print();
        printf("\t! %s", rarity_names[rarity()]);

        if (rarity() > 0) {
            for (auto i : read_array<addrtype>("unique_name", 0x8, 16))
                wprintf(L"%S", PoEMemory::read<wstring>(i + 0x30, 32).c_str());
        }
    }
};
