/*
* Mods.cpp, 8/10/2020 11:38 PM
*/

static std::map<string, int> modifier_offsets {
    {"id",         0x0},
    {"type",      0x14},
    {"req_level", 0x1c},
    {"stats",     0x28},
    {"domain",    0x60},
    {"name",      0x64},
    {"gen_type",  0x6c},
    {"group",     0x70},
    {"stat_vals", 0x78},
};

class Modifier : RemoteMemoryObject {
public:

    wstring id, name;
    int domain, gen_type;

    Modifier(addrtype address) : RemoteMemoryObject(address, &modifier_offsets) {
        id = PoEMemory::read<wstring>(address, 128);
        name = PoEMemory::read<wstring>(address + (*offsets)["name"], 32);
        domain = read<int>("domain");
        gen_type = read<int>("gen_type");
    }

    wstring type() {
        return PoEMemory::read<wstring>(address  + (*offsets)["type"], 128);
    }

    int req_level() {
        return read<byte>("req_level");
    }

    wstring group() {
        return PoEMemory::read<wstring>(address  + (*offsets)["group"], 128);
    }

    void to_print() {
        wprintf(L"%llx: %2d %2d %2d %-48S %S\n", address, req_level(), domain, gen_type, id.c_str(), name.c_str());
    }
};

/* Mods component offsets */

static std::map<string, int> mods_component_offsets {
    {"unique_name",      0x30},
    {"is_identified",    0xa8},
    {"rarity",           0xac},
    {"implicit_mods",    0xb8},
    {"explicit_mods",    0xd0},
    {"enchant_mods",     0xe8},
    {"implicit_stats",  0x1c0},
    {"enchant_stats",   0x1d8},
    {"explicit_stats",  0x1f0},
    {"crafted_stats",   0x208},
    {"fractured_stats", 0x220},
    {"item_level",      0x480},
    {"required_level",  0x484},
    {"is_mirrored",     0x489},
    {"is_synthesised",  0x4a1},
};

class Mods : public Component {
public:

    wstring unique_name;
    int rarity, item_level;

    /* Modifiers */
    std::vector<Modifier> implicit_mods;
    std::vector<Modifier> enchant_mods;
    std::vector<Modifier> explicit_mods;

    /* Stats */
    std::vector<wstring> implicit_stats;
    std::vector<wstring> enchant_stats;
    std::vector<wstring> explicit_stats;
    std::vector<wstring> crafted_stats;
    std::vector<wstring> fractured_stats;

    Mods(addrtype address) : Component(address, "Mods", &mods_component_offsets) {
        rarity = read<int>("rarity");
        item_level = read<int>("item_level");
    }

    wstring& name(wstring& base_name) {
        if (!unique_name.empty())
            return unique_name;

        switch (rarity) {
        case 1:
            unique_name = base_name;
            get_mods();
            for (auto i : explicit_mods) {
                if (i.gen_type == 1)
                    unique_name = i.name + L" " + unique_name;
                if (i.gen_type == 2)
                    unique_name += L" " + i.name;
            }
            break;

        case 2:
        case 3:
            for (auto addr : read_array<addrtype>("unique_name", 0x0, 0x10))
                unique_name += PoEMemory::read<wstring>(addr + 0x30, 32);
        }

        return unique_name;
    }

    bool is_identified() {
        return read<byte>("is_identified");
    }

    bool is_synthesised() {
        return read<byte>("is_synthesised");
    }

    bool is_mirrored() {
        return read<byte>("is_mirrored");
    }

    void get_mods() {
        if (!explicit_mods.empty())
            return;

        implicit_mods = read_array<Modifier>("implicit_mods", 0x18, 0x28);
        enchant_mods = read_array<Modifier>("enchant_mods", 0x18, 0x28);
        explicit_mods = read_array<Modifier>("explicit_mods", 0x18, 0x28);
    }

    void get_stats() {
        if (!explicit_stats.empty())
            return;

        implicit_stats = read_array<wstring>("implicit_stats", 0x20);
        enchant_stats = read_array<wstring>("enchant_stats", 0x20);
        explicit_stats = read_array<wstring>("explicit_stats", 0x20);
        crafted_stats = read_array<wstring>("crafted_stats", 0x20);
        fractured_stats = read_array<wstring>("fractured_stats", 0x20);
    }

    void to_print() {
        const char* rarity_names[] = {"Normal", "Magic", "Rare", "Unique"};

        Component::to_print();
        printf("\t\t\t! %s, ilvl: %d", rarity_names[rarity], item_level);
    }
};
