/*
* Mods.cpp, 8/10/2020 11:38 PM
*/

static std::map<string, int> modifier_offsets {
    {"id",       0x0},
    {"level",   0x1c},
    {"stat_id", 0x28},
    {"domain",  0x60},
    {"name",    0x64},
    {"type",    0x6c},
    {"group",   0x70},
    {"minimum", 0x78},
    {"maximum", 0x7c},
    {"tier",   0x1c5},
};

class Modifier : RemoteMemoryObject {
public:

    wstring id, name, group;
    int level, domain, type, tier;

    Modifier(addrtype address) : RemoteMemoryObject(address, &modifier_offsets) {
        group = read<byte>("group");
        id = PoEMemory::read<wstring>(address, 128);
        name = PoEMemory::read<wstring>(address + 0x64, 32);
        level = read<int>("level");
        domain = read<int>("domain");
        type = read<int>("type");
    }

    void to_print() {
        wprintf(L"%llx: %2d %2d %2d %-48S %S\n", address, level, domain, type, id.c_str(), name.c_str());
    }
};

static std::map<string, int> stat_offsets {
    {"text",     0x0},
    {"unkonw1",  0x8},
    {"unkonw2",  0x9},
    {"unkonw3", 0x10},
    {"unkonw4", 0x18},
};

class Stat : RemoteMemoryObject {
public:

    wstring text;

    Stat(addrtype address) : RemoteMemoryObject(address, &stat_offsets) {
        text = read<wstring>("text");
    }

    void to_print(string type = "") {
        wprintf(L"%llx: %S %s\n", address, text.c_str(), type.empty() ? "" : type.c_str());
    }
};

/* Mods component offsets */

static std::map<string, int> mods_component_offsets {
    {"unique_name",      0x30},
    {"is_identified",    0xa8},
    {"rarity",           0xac},
    {"implicit_mods",    0xb0},
    {"explicit_mods",    0xc8},
    {"enchant_mods",     0xe0},
    {"implicit_stats",  0x1a8},
    {"enchant_stats",   0x1c0},
    {"explicit_stats",  0x1d8},
    {"crafted_stats",   0x1f0},
    {"fractured_stats", 0x208},
    {"is_synthesised",  0x437},
    {"item_level",      0x46c},
    {"is_mirrored",     0x475},
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
    std::vector<Stat> implicit_stats;
    std::vector<Stat> enchant_stats;
    std::vector<Stat> explicit_stats;
    std::vector<Stat> crafted_stats;
    std::vector<Stat> fractured_stats;

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
            for (auto i : read_array<Modifier>("explicit_mods", 0x20, 0x28)) {
                if (i.type == 1)
                    unique_name = i.name + L" " + unique_name;
                if (i.type == 2)
                    unique_name += L" " + i.name;
            }
            break;

        case 2:
        case 3:
            for (auto addr : read_array<addrtype>("unique_name", 0x8, 0x10))
                unique_name += PoEMemory::read<wstring>(addr + 0x30, 32);
            unique_name += L" " + base_name;
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
        implicit_mods = read_array<Modifier>("implicit_mods", 0x20, 0x28);
        enchant_mods = read_array<Modifier>("enchant_mods", 0x20, 0x28);
        explicit_mods = read_array<Modifier>("explicit_mods", 0x20, 0x28);
    }

    void list_mods() {
        get_mods();
        printf("\n");
        if (!enchant_mods.empty()) {
            for (auto i : enchant_mods)
                i.to_print();
            printf("----------------------------------------------------------------------------------------------------\n");
        }

        if (!implicit_mods.empty()) {
            for (auto i : implicit_mods)
                i.to_print();
            printf("----------------------------------------------------------------------------------------------------\n");
        }

        for (auto i : explicit_mods)
            i.to_print();
    }

    void get_stats() {
        implicit_stats = read_array<Stat>("implicit_stats", 0x20);
        enchant_stats = read_array<Stat>("enchant_stats", 0x20);
        explicit_stats = read_array<Stat>("explicit_stats", 0x20);
        crafted_stats = read_array<Stat>("crafted_stats", 0x20);
        fractured_stats = read_array<Stat>("fractured_stat", 0x20);
    }

    void list_stats() {
        get_stats();
        printf("\n");
        if (!enchant_stats.empty()) {
            for (auto i : enchant_stats)
                i.to_print("(enchant)");
            printf("----------------------------------------------------------------------------------------------------\n");
        }

        if (!implicit_stats.empty()) {
            for (auto i : implicit_stats)
                i.to_print();
            printf("----------------------------------------------------------------------------------------------------\n");
        }

        for (auto i : explicit_stats)
            i.to_print();

        for (auto i : crafted_stats)
            i.to_print("(crafted)");

        for (auto i : fractured_stats)
            i.to_print("(fractured)");
    }

    void to_print() {
        const char* rarity_names[] = {"Normal", "Magic", "Rare", "Unique"};

        Component::to_print();
        printf("\t\t\t! %s, ilvl: %d", rarity_names[rarity], item_level);
    }
};
