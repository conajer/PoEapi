/*
* ServerData.cpp, 8/18/2020 6:43 PM
*/

#include <algorithm>
#include <unordered_map>

static std::map<string, int> inventory_cell_offsets {
    {"item", 0x0},
    {"l",    0x8},
    {"t",    0xc},
    {"r",   0x10},
    {"b",   0x14},
};

class InventoryCell : public RemoteMemoryObject {
public:

    shared_ptr<Item> item;
    addrtype item_address;
    int index, x, y;

    InventoryCell(addrtype address)
        : RemoteMemoryObject(address, &inventory_cell_offsets)
    {
        x = read<int>("l");
        y = read<int>("t");
        item_address = read<addrtype>("item");
    }

    Item& get_item() {
        addrtype addr = read<addrtype>("item");
        if (!item || item->address != addr)
            item = shared_ptr<Item>(new Item(addr));

        return *item;
    }

    void to_print() {
        wprintf(L"    %llx: (%d, %d), %S\n", address, x, y, item->name().c_str());
        item->list_components();
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
        {"cells",       0x30},
        {"count",       0x50},
};

class InventorySlot : public RemoteMemoryObject, public AhkObj {
private:

    AhkObjRef* __get_items() {
        AhkObj items;
        for (auto& i : get_items()) {
            Item& item = i.second->get_item();
            if (!item.obj_ref)
                item.__set(L"index", i.first, AhkInt,
                           L"left", i.second->x + 1, AhkInt,
                           L"top", i.second->y + 1, AhkInt,
                           nullptr);
            items.__set(std::to_wstring(i.first).c_str(),
                        (AhkObjRef*)item, AhkObject, nullptr);
        }
        __set(L"Items", (AhkObjRef*)items, AhkObject, nullptr);

        return items;
    }

public:

    std::unordered_map<int, shared_ptr<InventoryCell>> cells;
    int id, type, sub_type, cols, rows;

    InventorySlot(addrtype address) : RemoteMemoryObject(address, &inventory_offsets) {
        id = read<byte>("id");
        this->address = read<addrtype>("internal");

        type = read<byte>("type");
        sub_type = read<byte>("sub_type");
        cols = read<byte>("cols");
        rows = read<byte>("rows");

        add_method(L"getItems", this, (MethodType)&InventorySlot::__get_items, AhkObject);
    }

    void __new() {
        __set(L"id", id, AhkInt,
              L"type", type, AhkInt,
              L"subType", sub_type, AhkInt,
              L"cols", cols, AhkInt,
              L"rows", rows, AhkInt,
              L"items", nullptr, AhkObject,
              nullptr);
    }

    int count() {
        return read<byte>("count");
    }

    std::unordered_map<int, shared_ptr<InventoryCell>>& get_items() {
        std::unordered_map<int, shared_ptr<InventoryCell>> removed_cells;
        removed_cells.swap(cells);

        if (count() > 0) {
            for (auto addr : read_array<addrtype>("cells", 0x0, 8) ) {
                if (addr > 0) {
                    shared_ptr<InventoryCell> cell(new InventoryCell(addr));
                    int index = cell->x * rows + cell->y + 1;
                    auto i = removed_cells.find(index);
                    if (i == removed_cells.end() || i->second->address != addr) {
                        cells[index] = cell;
                        removed_cells.erase(index);
                        continue;
                    }

                    cells.insert(*i);
                    removed_cells.erase(i);
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
                i.second->to_print();
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
    L"Folder",
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
    {"folder_id",    0x3a},
    {"flags",        0x3d},
    {"is_affinity",  0x3e},
    {"affinities",    0x3f},
};

class StashTab : public RemoteMemoryObject, public AhkObj {
public:

    wstring name;
    int type, index, flags;
    bool is_affinity;
    int affinities, id = 0;
    short folder_id;

    StashTab(addrtype address) : RemoteMemoryObject(address, &stash_tab_offsets) {
        name = read<wstring>("name");
        index = read<byte>("index");
        type = read<byte>("type");
        flags = read<byte>("flags");
        folder_id = read<short>("folder_id");
        is_affinity = read<byte>("is_affinity");
        affinities = read<int>("affinities");

        add_method(L"getId", this, (MethodType)&StashTab::inventory_id, AhkInt);
    }

    void __new() {
        __set(L"index", index, AhkInt,
              L"name", name.c_str(), AhkWString,
              L"type", type, AhkInt,
              L"flags", flags, AhkInt,
              L"folderId", folder_id, AhkInt,
              L"isAffinity", is_affinity, AhkBool,
              L"affinities", affinities, AhkInt,
              nullptr);
    }

    int inventory_id() {
        if (id == 0) {
            id = read<byte>("inventory_id");
            __set(L"id", id, AhkInt, nullptr);
        }

        return id;
    }

    void to_print() {
        wprintf(L"    %llx %2d %3d %4x  %02d|%-10S %S\n",
                address, index, inventory_id(), flags, type, stash_tab_types[type], name.c_str());
    }

};

static bool compare_stash_tab(shared_ptr<StashTab>& tab1, shared_ptr<StashTab>& tab2) {
    return tab1->index < tab2->index;
}

static std::map<string, int> server_data_offsets {
    {"league",          0x7850},
    {"latency",         0x78c8},
    {"party_status",    0x7a00},
    {"stash_tabs",      0x78d8},
    {"inventory_slots", 0x7c68},
};

class ServerData : public RemoteMemoryObject {
public:

    std::map<int, shared_ptr<InventorySlot>> inventory_slots;
    std::vector<shared_ptr<StashTab>> stash_tabs;

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

    std::vector<shared_ptr<StashTab>>& get_stash_tabs() {
        stash_tabs.clear();
        for (auto addr : read_array<addrtype>("stash_tabs", 0x48))
            stash_tabs.push_back(shared_ptr<StashTab>(new StashTab(addr)));

        for (auto& i : stash_tabs) {
            if (i->folder_id >= 0)
                i->folder_id = stash_tabs[i->folder_id]->index;
        }

        for (auto i = stash_tabs.begin(); i != stash_tabs.end();)
            i = ((*i)->flags & IsHidden) ? stash_tabs.erase(i) : i + 1;
        std::sort(stash_tabs.begin(), stash_tabs.end(), compare_stash_tab);

        int last_index = 0;
        for (auto& i : stash_tabs) {
            if (i->folder_id >= 0)
                i->index = stash_tabs[i->folder_id]->index;
            else {
                if (i->index > last_index + 1)
                    i->index = last_index + 1;
            }
            
            last_index = i->index;
        }

        return stash_tabs;
    }

    std::map<int, shared_ptr<InventorySlot>>& get_inventory_slots() {
        for (auto addr : read_array<addrtype>("inventory_slots", 0x20)) {
            shared_ptr<InventorySlot> slot(new InventorySlot(addr));
            auto i = inventory_slots.find(slot->id);
            if (i == inventory_slots.end() || i->second->address != addr)
                inventory_slots[slot->id] = shared_ptr<InventorySlot>(slot);
        }

        return inventory_slots;
    }

    void list_stash_tabs() {
        printf("%llx: Stash Tabs\n", read<addrtype>("stash_tabs"));
        printf("    Address      #  Id Flags Type          Name\n");
        printf("    ----------- -- --- ----- ------------- ------------------------\n");
        for (auto tab : get_stash_tabs())
            tab->to_print();
    }

    void list_inventorie(int id = 0) {
        printf("%llx: Inventorys\n", read<addrtype>("inventory_slots"));
        printf("    Address      Id Rows Cols Items\n");
        printf("    ----------- --- ---- ---- -----\n");
        for (auto i : get_inventory_slots()) {
            if (id == 0 || i.second->id == id) {
                i.second->to_print();
                break;
            }
        }
    }
};
