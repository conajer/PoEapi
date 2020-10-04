/*
* KillCounter.cpp, 8/31/2020 3:07 PM
*/

using Monsters = std::unordered_set<int>;

struct AreaMonsters {
    int index;
    Monsters total;
    Monsters killed;
};

class KillCounter : public PoEPlugin {
public:

    std::unordered_map<int, AreaMonsters*> latest_areas;
    LocalPlayer* player;
    AreaMonsters* current_area;
    Monsters near_monsters;
    bool in_town;
    int num_of_monsters, num_of_minions;
    int total_monsters, num_of_killed;
    int maximum_area_count = 9;
    
    KillCounter() : PoEPlugin(L"KillCounter", "0.1"), player(nullptr), current_area(nullptr) {
        in_town = false;
        num_of_monsters = num_of_minions = 0;
        total_monsters = num_of_killed = 0;
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        player = local_player;
    }

    void on_area_changed(AreaTemplate* world_area, int hash_code) {
        in_town = world_area->is_town();
        if (in_town)
            return;
        
        near_monsters.clear();
        if (latest_areas.find(hash_code) == latest_areas.end()) {
            // Remove the oldest area record.
            if (latest_areas.size() == maximum_area_count)
                latest_areas.erase(maximum_area_count - 1);

            current_area = new AreaMonsters();
            for (auto& i : latest_areas) {
                if (i.second->index >= 0)
                    i.second->index += 1;
            }
            latest_areas[hash_code] = current_area;
        } else {
            current_area = latest_areas[hash_code];
            for (auto& i : latest_areas) {
                if (i.second->index < current_area->index)
                    i.second->index += 1;
            }
        }
        current_area->index = 0;

        total_monsters = num_of_killed = 0;
        PostThreadMessage(thread_id, WM_KILLED, num_of_killed, total_monsters);
    }

    void on_entity_changed(EntityList& entities, EntityList& removed, EntityList& added) {
        int n_monsters = 0, charges = 0, n_minions = 0;
        
        if (in_town || !current_area || !player)
            return;

        /* treat the removed near monsters as killed. */
        for (auto& i : removed) {
            shared_ptr<Entity>& entity = i.second;
            if (entity->is_monster) {
                if (near_monsters.find(i.first) != near_monsters.end())
                    current_area->killed.insert(i.first);
            }
        }

        near_monsters.clear();
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                return;
            }

            shared_ptr<Entity>& entity = i.second;
            if (entity->is_monster) {
                if (entity->is_neutral) {
                    n_minions++;
                    continue;
                }

                current_area->total.insert(i.first);
                if (entity->is_dead()) {
                    current_area->killed.insert(i.first);
                    continue;
                }

                int dist = player->dist(*entity);
                if (dist < 100) {
                    n_monsters++;
                    if (entity->rarity > 0)
                        charges += 1 + (1 << (entity->rarity - 1)) * 2.5;
                    else
                        charges += 1;
                    near_monsters.insert(i.first);
                }
            }
        }

        if (num_of_minions != n_minions) {
            num_of_minions = n_minions;
            PostThreadMessage(thread_id, WM_MINION_CHANGED, num_of_minions, 0);
        }

        if (num_of_monsters != n_monsters) {
            num_of_monsters = n_monsters;
            PostThreadMessage(thread_id, WM_MONSTER_CHANGED, num_of_monsters, charges);
        }

        if (num_of_killed != current_area->killed.size()
            || total_monsters != current_area->total.size())
        {
            total_monsters = current_area->total.size();
            num_of_killed = current_area->killed.size();
            PostThreadMessage(thread_id, WM_KILLED, num_of_killed, total_monsters);
        }
    }
};
