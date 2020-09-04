/*
* ServerData.cpp, 8/18/2020 6:43 PM
*/

#include <algorithm>
#include <unordered_map>

static std::map<string, int> inventory_cell_offsets {
    {"item", 0x0},
    {"x",    0x8},
    {"y",    0xc},
};

class InventoryCell : public RemoteMemoryObject {
public:

    Item item;
    int index, x, y;

    InventoryCell(addrtype address)
        : RemoteMemoryObject(address, &inventory_cell_offsets), item(read<addrtype>("item"))
    {
        x = read<int>("x");
        y = read<int>("y");
    }

    Item& get_item() {
        item.address != read<addrtype>("item");

        return item;
    }

    void to_print() {
        wprintf(L"    %llx: (%d, %d), %S\n", address, x, y, item.name().c_str());
        item.list_components();
    }
};

static std::map<string, int> inventory_offsets {
    {"id",               0x0},
    {"internal",         0x8},
        {"type",         0x0},
        {"sub_type",     0x4},
        {"is_requested", 0x4},
        {"cols",         0xc},
        {"rows",        0x10},
        {"items",       0x30},
        {"count",       0x50},
};

class Inventory : public RemoteMemoryObject {
public:

    std::unordered_map<int, InventoryCell> cells;
    int id, type, sub_type, cols, rows;

    Inventory(addrtype address) : RemoteMemoryObject(address, &inventory_offsets) {
        id = read<byte>("id");
        this->address = read<addrtype>("internal");

        type = read<byte>("type");
        sub_type = read<byte>("sub_type");
        cols = read<byte>("cols");
        rows = read<byte>("rows");
    }

    int count() {
        return read<byte>("count");
    }

    std::unordered_map<int, InventoryCell>& get_items() {
        if (count() > 0) {
            for (auto addr : read_array<addrtype>("items", 0x0, 8) ) {
                if (addr > 0) {
                    InventoryCell cell(addr);
                    cells.insert(std::make_pair(cell.x * rows + cols, cell));
                }
            }
        }

        return cells;
    }

    void to_print() {
        printf("    %llx %3d %3d  %3d  %4d\n", address, id, rows, cols, count());
        if (verbose) {
            printf("    ----------- --- ---- ---- -----\n");
            for (auto i : get_items()) {
                i.second.to_print();
            }
        }
    }
};

static const wchar_t* stash_tab_types[] = {
    L"Normal",
    L"Premium",
    L"",
    L"Currency",
    L"",
    L"Map",
    L"Divination",
    L"Quad",
    L"Essence",
    L"Fragment",
    L"",
    L"",
    L"Delve",
    L"Blight",
    L"Metamorph",
    L"Delirium",
};

enum StashTabFlags {
    RemoveOnly = 0x1,
    IsPremium  = 0x4,
    IsPublic   = 0x20,
    IsMap      = 0x40,
    IsHidden   = 0x80,
};

static std::map<string, int> stash_tab_offsets {
    {"name",          0x8},
    {"inventory_id", 0x28},
    {"type",         0x34},
    {"index",        0x38},
    {"flags",        0x3d},
};

class StashTab : public RemoteMemoryObject {
public:

    wstring name;
    int inventory_id, type, index, flags;

    StashTab(addrtype address) : RemoteMemoryObject(address, &stash_tab_offsets) {
        name = read<wstring>("name");
        inventory_id = read<byte>("inventory_id");
        type = read<byte>("type");
        index = read<byte>("index");
        flags = read<byte>("flags");
    }

    int get_inventory_id() {
        return inventory_id = read<byte>("inventory_id");
    }

    void to_print() {
        wprintf(L"    %llx %2d %3d %4x  %02d|%-10S %S\n",
                address, index, inventory_id, flags, type, stash_tab_types[type], name.c_str());
    }

    bool operator< (StashTab& tab) {
        return index < tab.index;
    }
};

static std::map<string, int> server_data_offsets {
    {"league",       0x7350},
    {"latency",      0x73c8},
    {"party_status", 0x7500},
    {"stash_tabs",   0x73d8},
    {"inventories",  0x76f0},
};

class ServerData : public RemoteMemoryObject {
public:

    std::vector<Inventory> inventories;
    std::vector<StashTab> stash_tabs;

    ServerData(addrtype address) : RemoteMemoryObject(address, &server_data_offsets) {
    }

    wstring league() {
        return read<wstring>("league");
    }

    int latency() {
        return read<int>("latency");
    }

    int party_status() {
        return read<byte>("party_status");
    }

    std::vector<StashTab>& get_stash_tabs() {
        stash_tabs = read_array<StashTab>("stash_tabs", 0x40);
        for (auto i = stash_tabs.end() - 1; i >= stash_tabs.begin(); --i) {
            if (i->flags & IsHidden)
                stash_tabs.erase(i);
        }
        std::sort(stash_tabs.begin(), stash_tabs.end());

        return stash_tabs;
    }

    void list_stash_tabs() {
        printf("%llx: Stash Tabs\n", read<addrtype>("stash_tabs"));
        printf("    Address      #  Id Flags Type          Name\n");
        printf("    ----------- -- --- ----- ------------- ------------------------\n");
        for (auto tab : get_stash_tabs()) {
            tab.to_print();
        }
    }

    std::vector<Inventory>& get_inventories() {
        return inventories = read_array<Inventory>("inventories", 0x20);
    }

    void list_inventorie(int id = 0) {
        printf("%llx: Inventorys\n", read<addrtype>("inventories"));
        printf("    Address      Id Rows Cols Items\n");
        printf("    ----------- --- ---- ---- -----\n");
        for (auto i : get_inventories()) {
            if (id == 0 || i.id == id) {
                i.to_print();
                break;
            }
        }
    }
};
