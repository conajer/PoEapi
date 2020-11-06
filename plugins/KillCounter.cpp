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
    AreaMonsters* current_area;
    Monsters nearby_monsters;
    int nearby_radius = 50;
    int num_of_monsters, num_of_minions;
    int total_monsters, num_of_killed;
    int maximum_area_count = 9;

    KillCounter() : PoEPlugin(L"KillCounter", "0.2"), current_area(nullptr) {
        num_of_monsters = num_of_minions = 0;
        total_monsters = num_of_killed = 0;
    }

    void on_area_changed(AreaTemplate* world_area, int hash_code) {
        if (world_area->is_town())
            return;
        
        nearby_monsters.clear();
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
        if (!current_area || !player)
            return;

        /* treat the removed nearby monsters as killed. */
        for (auto& i : removed) {
            shared_ptr<Entity>& entity = i.second;
            if (entity->is_monster) {
                if (nearby_monsters.find(i.first) != nearby_monsters.end())
                    current_area->killed.insert(i.first);
            }
        }

        nearby_monsters.clear();
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                return;
            }

            Entity* entity = i.second.get();
            poe->get_pos(entity);

            if (entity->is_monster) {
                if (!entity->is_neutral) {
                    if (entity->has_component("DiesAfterTime"))
                        continue;

                    if (entity->is_dead()) {
                        if (current_area->total.find(i.first) != current_area->total.end())
                            current_area->killed.insert(i.first);
                        continue;
                    } else {
                        if (current_area->total.find(i.first) == current_area->total.end())
                            current_area->total.insert(i.first);
                    }
                }

                int dist = player->dist(*entity);
                if (!entity->is_neutral && dist < 4 * nearby_radius)
                    nearby_monsters.insert(i.first);

                if (dist < nearby_radius) {
                    if (entity->is_neutral) {
                        n_minions++;
                        continue;
                    }

                    n_monsters++;
                    if (entity->rarity > 0)
                        charges += 1 + (1 << (entity->rarity - 1)) * 2.5;
                    else
                        charges += 1;
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
