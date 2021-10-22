/*
* InGameData.cpp, 8/18/2020 6:44 PM
*/

#include <regex>
#include <unordered_map>
#include <unordered_set>

#include "Terrain.cpp"

using EntityList = std::unordered_map<int, shared_ptr<Entity>>;

class EntitySet {
public:

    EntityList all, removed, added;

    friend ostream& operator<<(ostream& os, EntitySet& entities)
    {
        if (entities.added.size() > 0) {
            for (auto i : entities.added)
                cout << *i.second << endl;
        }
        os << entities.all.size() << ", -" << entities.removed.size() << ", +" << entities.added.size();

        return os;
    }
};

std::map<string, int> in_game_data_offsets {
    {"world_area",         0x68},
    {"area_level",         0x80},
    {"area_hash",          0xe4},
    {"local_player",      0x408},
    {"entity_list",       0x490},
        {"root",            0x8},
    {"entity_list_count", 0x498},
    {"terrain",           0x750},
};

class InGameData : public RemoteMemoryObject {
protected:

    std::unordered_map<int, shared_ptr<Entity>> entity_list;
    std::unordered_map<int, bool> entity_list_snapshot;
    std::unordered_map<addrtype, addrtype> entity_list_index;

    std::wregex ignored_entity_exp;
    std::unordered_set<addrtype> ignored_entities;

    const __int64 mask = 0xffffff0000000000;

    wstring get_entity_path(addrtype address) {
        return PoEMemory::read<wstring>(PoEMemory::read<addrtype>(address + 0x8) + 0x8);
    }

    void foreach(int entity_id, addrtype entity_address) {
        if (ignored_entities.count(entity_id))
            return;

        if (entity_list_snapshot.count(entity_id)) {
            entity_list_snapshot[entity_id] = true;
            return;
        }

        wstring path = get_entity_path(entity_address);
        if (path[0] != L'M' || std::regex_search(path, ignored_entity_exp)) {
            ignored_entities.insert(entity_id);
            return;
        }

        std::shared_ptr<Entity> entity(new Entity(entity_address));
        entity_list[entity_id] = entity;
    }

    void traverse_entity_list(addrtype root, addrtype node) {
        if (force_reset)
            return;

        addrtype buffer[8];
        if (PoEMemory::read<addrtype>(node, buffer, 8)) {
            int entity_id = buffer[4];
            addrtype entity_address = buffer[5];

            if (entity_id > 0)
                entity_list_index[node] = entity_address;

            for (int i : (int[]){0, 2}) {
                if ((buffer[i] == root) || (buffer[i] & mask) ^ (node & mask))
                    continue;
                traverse_entity_list(root, buffer[i]);
            }
        }
    }

public:

    shared_ptr<AreaTemplate> area;
    shared_ptr<LocalPlayer> player;
    shared_ptr<Terrain> terrain;
    bool force_reset = false;

    InGameData(addrtype address) :
        RemoteMemoryObject(address, &in_game_data_offsets),
        ignored_entity_exp(L"WorldItem|Barrel|Basket|Bloom|BonePile|Boulder|Cairn|Crate|Pot|Urn|Vase"
                           "|BlightFoundation|BlightTower|Effects")
    {
    }

    int area_hash() {
        return read<int>("area_hash");
    }

    AreaTemplate* world_area() {
        addrtype addr = read<addrtype>("world_area");
        if (!area || area->address != addr) {
            area.reset(new AreaTemplate(addr));
        }

        return area.get();
    }

    LocalPlayer* local_player() {
        addrtype addr = read<addrtype>("local_player");
        if (!player || player->address != addr) {
            wchar_t path_0 = PoEMemory::read<wchar_t>(addr + 0x8, {8, 0});
            if (path_0 == L'M')
                player.reset(new LocalPlayer(addr));
        }

        return player.get();
    }

    Terrain* get_terrain() {
        if (!terrain)
            terrain = shared_ptr<Terrain>(new Terrain(address + (*offsets)["terrain"]));
        return terrain.get();
    }

    int get_all_entities(EntitySet& entities) {
        entities.removed.clear();
        entities.removed.swap(entities.all);
        entities.added.clear();

        // take a snapshot of the entity list
        for (auto i : entity_list)
            entity_list_snapshot[i.first] = false;

        addrtype root = read<addrtype>("entity_list");
        addrtype node = read<addrtype>("entity_list", "root");
        entity_list_index.clear();
        traverse_entity_list(root, node);

        // parse entity list
        for (auto i : entity_list_index)
            foreach(i.first, i.second);

        // removed non-existent entities
        for (auto i : entity_list_snapshot) {
            if (!i.second)  // invalid entity
                entity_list.erase(i.first);
        }
        entity_list_snapshot.clear();

        // update entities
        for (auto& i : entity_list) {
            if (force_reset)
                break;

            if (entities.removed.count(i.first)) {
                entities.all.insert(i);
                entities.removed.erase(i.first);
            } else {
                entities.all.insert(i);
                entities.added.insert(i);
            }
        }

        return entities.all.size();
    }
};
