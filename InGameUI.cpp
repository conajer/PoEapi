/*
* InGameUI.cpp, 8/18/2020 6:46 PM
*/

#include <unordered_map>

#include "ui/Inventory.cpp"
#include "ui/Stash.cpp"

static std::map<string, int> in_game_ui_offsets {
    {"inventory",       0x520},
    {"stash",           0x528},
    {"entity_list",     0x5b0},
        {"root",        0x2a0},
    {"purchase",        0x638},
    {"sell",            0x640},
    {"trade",             0x0},
    {"gem_level_up",    0x8c8},
};

enum {
    InventoryIndex  = 32,
    StashIndex      = 33,
};

class InGameUI : public Element {
public:

    std::unordered_map<int, shared_ptr<Entity>> entities, removed;
    unique_ptr<Inventory> inventory;
    unique_ptr<Stash> stash;

    InGameUI(addrtype address) : Element(address, &in_game_ui_offsets) {
    }

    Inventory* get_inventory() {
        if (childs.empty())
            get_childs();
        Element* element = childs[StashIndex].get();
        if (!inventory || (element && *inventory != *element)) {
            inventory = unique_ptr<Inventory>(new Inventory(*element));
        }

        return inventory.get();
    }

    Stash* get_stash() {
        #if 0
        if (childs.empty())
            get_childs();
        Element* element = childs[StashIndex].get();
        if (!stash || (element && *stash != *element)) {
            stash = unique_ptr<Stash>(new Stash(*element));
        }
        #endif
        stash = unique_ptr<Stash>(new Stash(read<addrtype>("stash")));
        return stash.get();
    }

    EntityList& get_all_entities() {
        entities.swap(removed);
        entities.clear();
        addrtype root = read<addrtype>("entity_list", "root");
        addrtype next = root;

        while (1) {
            next = PoEMemory::read<addrtype>(next);
            if (next == root)
                break;

            addrtype entity_address = PoEMemory::read<addrtype>(next + 0x10);
            int entity_id = PoEMemory::read<int>(entity_address + 0x68);
            auto i = removed.find(entity_id);
            if (i != removed.end()) {
                entities.insert(*i);
                removed.erase(i);
                continue;
            }

            addrtype label = PoEMemory::read<addrtype>(next + 0x18);
            bool is_visible = PoEMemory::read<byte>(label + 0x111) & 0x4;
            if (!is_visible)
                continue;

            std::shared_ptr<Entity> entity(new Entity(entity_address));
            entity->label = shared_ptr<Element>(new Element(label));
            entities.insert(std::make_pair(entity_id, entity));
        }

        return entities;
    }

    shared_ptr<Entity> get_nearest_entity(LocalPlayer& player, wstring text) {
        unsigned int max_dist = -1, dist;
        shared_ptr<Entity> entity;

        for (auto i : get_all_entities()) {
            if ((dist = player.dist(*i.second)) < max_dist) {
                if (i.second->name().find(text) != -1 || i.second->path.find(text) != -1) {
                    entity = i.second;
                    max_dist = dist;
                }
            }
        }

        return entity;
    }
};
