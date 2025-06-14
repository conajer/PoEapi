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
    int x, y, w, h;

    InventoryCell(addrtype address)
        : RemoteMemoryObject(address, &inventory_cell_offsets)
    {
        x = read<int>("l");
        y = read<int>("t");
        w = read<int>("r") - x;
        h = read<int>("b") - y;
    }

    shared_ptr<Item> get_item() {
        addrtype addr = read<addrtype>("item");
        if (!item || item->address != addr)
            item = shared_ptr<Item>(new Item(addr));

        return item;
    }

    void to_print() {
        wprintf(L"    %llx: (%d, %d), %S\n", address, x, y, item->name().c_str());
        item->list_components();
    }
};

std::map<string, int> inventory_offsets {
    {"type",        0x138},
    {"sub_type",    0x13c},
    {"cols",        0x144},
    {"rows",        0x148},
    {"cells",       0x168},
    {"count",       0x188},
};

class InventorySlot : public RemoteMemoryObject, public AhkObj {
private:

    AhkObjRef* __get_item_by_index(int index) {
        shared_ptr<InventoryCell> cell = get_cell(index);
        if (cell) {
            shared_ptr<Item> item = cell->get_item();
            item->__set(L"index", index, AhkInt,
                        L"left", cell->x + 1, AhkInt,
                        L"top", cell->y + 1, AhkInt,
                        L"width", cell->w, AhkInt,
                        L"height", cell->h, AhkInt,
                        nullptr);

            return *item;
        }

        return nullptr;
    }

    AhkObjRef* __get_items() {
        AhkObj items;
        for (auto& i : get_cells()) {
            shared_ptr<Item> item = i.second->get_item();
            item->__set(L"index", i.first, AhkInt,
                        L"left", i.second->x + 1, AhkInt,
                        L"top", i.second->y + 1, AhkInt,
                        L"width", i.second->w, AhkInt,
                        L"height", i.second->h, AhkInt,
                        nullptr);
            items.__set(std::to_wstring(i.first).c_str(),
                        (AhkObjRef*)*item, AhkObject, nullptr);
        }
        __set(L"items", (AhkObjRef*)items, AhkObject, nullptr);

        return items;
    }

public:

    std::unordered_map<int, shared_ptr<InventoryCell>> cells;
    int id, type, sub_type, cols, rows;

    InventorySlot(int id, addrtype address) : RemoteMemoryObject(address, &inventory_offsets) {
        this->id = id;
        type = read<byte>("type");
        sub_type = read<byte>("sub_type");
        cols = read<byte>("cols");
        rows = read<byte>("rows");

        add_method(L"count", this, (MethodType)&InventorySlot::count, AhkInt);
        add_method(L"freeCells", this, (MethodType)&InventorySlot::free_cells, AhkInt);
        add_method(L"nextCell", this, (MethodType)&InventorySlot::next_cell, AhkInt, ParamList{AhkInt, AhkInt});
        add_method(L"getLayout", this, (MethodType)&InventorySlot::get_layout, AhkObject);
        add_method(L"getItemByIndex", this, (MethodType)&InventorySlot::__get_item_by_index, AhkObject, ParamList{AhkInt});
        add_method(L"getItems", this, (MethodType)&InventorySlot::__get_items, AhkObject);
    }

    void __new() {
        AhkObj::__new();
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

    int free_cells() {
        int free_cells = 0;
        for (auto addr : read_array<addrtype>("cells", 0x0, 8)) {
            if (addr == 0)
                free_cells++;
        }

        return free_cells;
    }

    AhkObjRef* get_layout() {
        AhkObj layout;

        std::unordered_map<int, shared_ptr<InventoryCell>> removed_cells;
        removed_cells.swap(cells);

        int n = 0, l = 0;
        for (auto addr : read_array<addrtype>("cells", 0x0, 8)) {
            l <<= 1;
            l |= addr ? 1 : 0;
            if (++n == cols) {
                layout.__set(L"", l, AhkInt, nullptr);
                n = 0;
                l = 0;
            }
        }
        __set(L"layout", (AhkObjRef*)layout, AhkObject, nullptr);

        return layout;
    }

    int next_cell(int width = 1, int height = 1) {
        auto all_cells = read_array<addrtype>("cells", 0x0, 8);
        for (int l = 0; l < cols - width + 1; ++l)
            for (int t = 0; t < rows - height + 1; ++t) {
                if (all_cells[t * cols + l] == 0) {
                    int w, h;

                    for (w = 0; w < width; ++w) {
                        for (h = 0; h < height; ++h) {
                            if (all_cells[(t + h) * cols + l + w] > 0)
                                break;
                        }

                        if (h < height)
                            break;
                    }

                    if (w == width)
                        return l * rows + t + 1;
                }
            }

        return 0;
    }

    std::unordered_map<int, shared_ptr<InventoryCell>>& get_cells() {
        std::unordered_map<int, shared_ptr<InventoryCell>> removed_cells;
        removed_cells.swap(cells);

        if (count() > 0) {
            for (auto addr : read_array<addrtype>("cells", 0x0, 8)) {
                if (addr > 0) {
                    shared_ptr<InventoryCell> cell(new InventoryCell(addr));
                    int index = cell->x * rows + cell->y + 1;
                    if (cells.find(index) != cells.end())
                        continue;

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

    shared_ptr<InventoryCell> get_cell(int index) {
        int n = ((index  - 1) % rows) * cols + (index  - 1) / rows;
        addrtype addr = PoEMemory::read<addrtype>(read<addrtype>("cells") + n * 8);
        if (addr > 0) {
            shared_ptr<InventoryCell> cell(new InventoryCell(addr));
            cells[index] = cell;
            return cell;
        }

        return nullptr;
    }

    void to_print() {
        printf("    %llx %3d %3d  %3d  %4d\n", address, id, rows, cols, count());
        if (verbose) {
            printf("    ----------- --- ---- ---- -----\n");
            for (auto i : get_cells()) {
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
    L"Flask",
    L"Gem",
};

enum StashTabFlags {
    RemoveOnly = 0x1,
    IsPremium  = 0x4,
    IsPublic   = 0x20,
    IsMap      = 0x40,
    IsHidden   = 0x80,
};

std::map<string, int> stash_tab_offsets {
    {"name",          0x8},
    {"inventory_id", 0x28},
    {"type",         0x34},
    {"index",        0x38},
    {"folder_id",    0x3a},
    {"flags",        0x3d},
    {"is_affinity",  0x3e},
    {"affinities",   0x3f},
};

class StashTab : public RemoteMemoryObject, public AhkObj {
public:

    wstring name;
    int type, index, flags;
    bool is_affinity;
    int affinities, id = 0;
    short folder_id;
    std::vector<shared_ptr<StashTab>> tabs;

    StashTab(addrtype address) : RemoteMemoryObject(address, &stash_tab_offsets) {
        name = read<wstring>("name");
        index = read<byte>("index");
        type = read<byte>("type");
        flags = read<byte>("flags");
        folder_id = read<short>("folder_id");
        is_affinity = read<byte>("is_affinity");
        affinities = read<int>("affinities");

        if (flags &  RemoveOnly)
            name = name + L" (Remove-only)";

        add_method(L"getId", this, (MethodType)&StashTab::inventory_id, AhkInt);
    }

    void __new() {
        AhkObj::__new();
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

std::map<string, int> server_data_offsets {
    {"player_data",                0x89a0},
        {"passive_skills",          0x180},
        {"player_class",            0x238},
        {"level",                   0x23c},
        {"refund_points_left",      0x240},
        {"skill_points_from_quest", 0x244},
        {"ascendancy_skill_points", 0x24c},
    {"league",                     0x89e0},
    {"latency",                    0x8a68},
    {"party_status",               0x8c40},
    {"stash_tabs",                 0x8a78},
    {"inventory_slots",            0x8eb8},
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

    vector<unsigned short> get_passive_skills() {
        addrtype addr = read<addrtype>("player_data") + (*offsets)["passive_skills"];
        return PoEMemory::read_array<unsigned short>(addr, 0x0, 0x2);
    }

    std::vector<shared_ptr<StashTab>>& get_stash_tabs() {
        stash_tabs.clear();
        for (auto addr : read_array<addrtype>("stash_tabs", 0x68))
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
            if (i->folder_id >= 0) {
                i->index = stash_tabs[i->folder_id]->tabs.size();
                stash_tabs[i->folder_id]->tabs.push_back(i);
            } else {
                if (i->index > last_index + 1)
                    i->index = last_index + 1;
                last_index = i->index;
            }
        }

        return stash_tabs;
    }

    std::map<int, shared_ptr<InventorySlot>>& get_inventory_slots() {
        for (auto addr : read_array<addrtype>("inventory_slots", 0x18)) {
            int slot_id = PoEMemory::read<int>(addr + 0x10);
            addrtype slot_ptr = PoEMemory::read<addrtype>(addr);
            shared_ptr<InventorySlot> slot(new InventorySlot(slot_id, slot_ptr));
            auto i = inventory_slots.find(slot->id);
            if (i == inventory_slots.end() || i->second->address != slot->address)
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

    void list_inventories(int id = 0) {
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
