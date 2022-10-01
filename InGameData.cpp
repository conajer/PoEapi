/*
* InGameData.cpp, 8/18/2020 6:44 PM
*/

#include <regex>
#include <unordered_map>
#include <unordered_set>

#include "Parallel.cpp"
#include "Terrain.cpp"

using EntityList = std::unordered_map<int, shared_ptr<Entity>>;

class EntitySet {
public:

    EntityList all, removed, added;

    void clear() {
        all.clear();
        removed.clear();
        added.clear();
    }

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
    {"world_area",         0x80},
    {"area_data",          0x88},
        {"area_index",     0x38},
    {"area_level",         0xa0},
    {"area_hash",         0x104},
    {"local_player",      0x6f8},
    {"entity_list",       0x7a8},
        {"root",            0x8},
    {"entity_list_count", 0x7b0},
    {"terrain",           0x740},
};

class InGameData : public RemoteMemoryObject, Parallel<addrtype> {
protected:

    std::unordered_map<int, shared_ptr<Entity>> entity_list;
    std::unordered_map<int, bool> entity_list_snapshot;
    std::unordered_map<addrtype, addrtype> entity_list_index;
    std::unordered_set<addrtype> entity_list_nodes;

    std::wregex ignored_entity_exp;
    std::unordered_set<addrtype> ignored_entities;

    std::mutex entity_list_mutex;
    std::mutex ignored_entities_mutex;

    addrtype root;
    int count;
    int new_entities;

    void foreach(int entity_id, addrtype entity_address) {
        {
            std::unique_lock<std::mutex> lock(ignored_entities_mutex);
            if (ignored_entities.count(entity_id))
                return;
        }

        if (entity_list_snapshot.count(entity_id)) {
            entity_list_snapshot[entity_id] = true;
            return;
        }

        addrtype entity_internal = PoEMemory::read<addrtype>(entity_address + 0x8);
        if (!entity_internal)
            return;

        wstring path = PoEMemory::read<wstring>(entity_internal + 0x8);
        if (path[0] != L'M' || std::regex_search(path, ignored_entity_exp)) {
            std::unique_lock<std::mutex> lock(ignored_entities_mutex);
            ignored_entities.insert(entity_id);
            return;
        }

        if (new_entities++ <= 64) {
            std::shared_ptr<Entity> entity(new Entity(entity_address, path.c_str()));
            std::unique_lock<std::mutex> lock(entity_list_mutex);
            entity_list[entity_id] = entity;
        }
    }

    void foreach(addrtype& node) {
        traverse_entity_list(node);
    }

    void traverse_entity_list(addrtype node) {
        if (force_reset)
            return;

        {
            std::unique_lock<std::mutex> lock(entity_list_mutex);
            if (entity_list_nodes.size() > count || entity_list_nodes.count(node))
                return;
            entity_list_nodes.insert(node);
        }

        addrtype buffer[8];
        if (!PoEMemory::read<addrtype>(node, buffer, 8))
            return;

        addrtype entity_address = buffer[5];
        if (buffer[0] != root)
            Parallel::add_task(buffer[0]);
        if (buffer[2] != root)
            Parallel::add_task(buffer[2]);

        int entity_id = buffer[4];
        if (entity_id > 0)
            foreach(entity_id, entity_address);
    }

public:

    shared_ptr<AreaTemplate> area;
    shared_ptr<LocalPlayer> player;
    shared_ptr<Terrain> terrain;
    bool force_reset = false;

    InGameData(addrtype address) :
        RemoteMemoryObject(address, &in_game_data_offsets),
        ignored_entity_exp(L"WorldItem|Barrel|Basket|Bloom|BonePile|Boulder|Cairn|Crate|Pot|Urn|Vase"
                           "|BlightFoundation|BlightTower|Projectiles|Effects|Doodad|Daemon|Volatile")
    {
        Parallel::start();
    }

    int area_index() {
        return read<int>("area_data", "area_index");
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
            wchar_t path_0 = PoEMemory::read<wchar_t>(addr, {0x8, 0x8, 0});
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

        root = read<addrtype>("entity_list");
        count = std::min(read<int>("entity_list_count"), 1024);

        // take a snapshot of the entity list
        for (auto i : entity_list)
            entity_list_snapshot[i.first] = false;
        entity_list_nodes.clear();
        entity_list_index.clear();

        addrtype node = read<addrtype>("entity_list", "root");
        new_entities = 0;
        Parallel::add_task(node);
        Parallel::wait();

        // remove non-existent entities
        for (auto i : entity_list_snapshot) {
            if (!i.second)
                entity_list.erase(i.first);
        }
        entity_list_snapshot.clear();

        // update entities
        for (auto& i : entity_list) {
            if (force_reset)
                break;

            entities.all.insert(i);
            if (entities.removed.count(i.first))
                entities.removed.erase(i.first);
            else
                entities.added.insert(i);
        }

        return entities.all.size();
    }
};
