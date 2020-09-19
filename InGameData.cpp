/*
* InGameData.cpp, 8/18/2020 6:44 PM
*/

#include <regex>
#include <unordered_map>
#include <unordered_set>

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

static std::map<string, int> in_game_data_offsets {
    {"world_area",         0x68},
    {"area_level",         0x80},
    {"area_hash",          0xe4},
    {"local_player",      0x408},
    {"entity_list",       0x490},
        {"root",            0x8},
    {"entity_list_count", 0x498},
};

class InGameData : public RemoteMemoryObject {
protected:

    std::unordered_set<addrtype> temp_set;
    std::queue<addrtype> nodes;

    int get_entity_id(addrtype address) {
        return PoEMemory::read<int>(address + 0x68);
    }

    wstring get_entity_path(addrtype address) {
        return PoEMemory::read<wstring>(PoEMemory::read<addrtype>(address + 0x8) + 0x8);
    }

public:

    AreaTemplate* area;
    LocalPlayer* player;

    InGameData(addrtype address) : RemoteMemoryObject(address, &in_game_data_offsets)
    {
        area = nullptr;
        player = nullptr;
    }

    int area_hash() {
        return read<int>("area_hash");
    }

    AreaTemplate* world_area() {
        addrtype addr = read<addrtype>("world_area");
        if (!area || area->address != addr) {
            delete area;
            area = new AreaTemplate(addr);
        }

        return area;
    }

    LocalPlayer* local_player() {
        addrtype addr = read<addrtype>("local_player");
        if (!player || player->address != addr) {
            wchar_t path_0 = PoEMemory::read<wchar_t>(addr + 0x8, {8, 0});
            if (path_0 == L'M') {
                player = new LocalPlayer(addr);
                if (!player->has_component("Player")) {
                    delete player;
                    player = nullptr;
                }
            }
        }

        return player;
    }

    int get_all_entities(EntitySet& entities, std::wregex& ignored_exp) {
        entities.removed.clear();
        entities.removed.swap(entities.all);
        entities.added.clear();

        addrtype addr = read<addrtype>("entity_list", "root");
        temp_set.insert(addr);
        nodes.push(addr);
        while (!nodes.empty()) {
            addrtype node = nodes.front();
            nodes.pop();

            for (int offset : (int[]){0x0, 0x10}) {
                addr = PoEMemory::read<addrtype>(node + offset);
                if (temp_set.count(addr) == 0) {
                    nodes.push(addr);
                    temp_set.insert(addr);
                }
            }

            addrtype entity_address = PoEMemory::read<addrtype>(node + 0x28);
            if ((__int64)entity_address & 0x7                   /* not 64-bit aligned */
                || entity_address < (addrtype)0x100000000       /* invalid address */
                || entity_address > (addrtype)0x7F0000000000)
                continue;

            wstring path = get_entity_path(entity_address);
            if (path[0] == L'M' && std::regex_search(path, ignored_exp))
                continue;

            int entity_id = get_entity_id(entity_address);
            auto i = entities.removed.find(entity_id);
            if (i != entities.removed.end()) {
                entities.all.insert(*i);
                entities.removed.erase(i);
                continue;
            }

            std::shared_ptr<Entity> entity(new Entity(entity_address));
            entities.all.insert(std::make_pair(entity_id, entity));
            entities.added.insert(std::make_pair(entity_id, entity));
        }
        temp_set.clear();

        return read<int>("entity_list_count");
    }

#if 0
    EntityList get_entities(std::vector<string> types = {}) {
        EntityList entities;

        get_all_entities();
        for (auto i : entity_list) {
            if (!types.empty()) {
                for (auto t : types) {
                    if (i.second.get()->has_component(t)) {
                        entities.insert(i);
                        break;
                    }
                }
            } else {
                entities.insert(i);
            }
        }

        return entities;
    }

    void list_entities() {
        cout << "Total " << read<int>("entity_list_count") << " entities:" << endl;
        for (auto i : entity_list) {
            i.second.get()->to_print();
        }
    }
#endif
};
