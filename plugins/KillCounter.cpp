/*
* KillCounter.cpp, 8/31/2020 3:07 PM
*/

using Monsters = std::unordered_set<int>;

struct AreaMonsters {
    Monsters total;
    Monsters killed;
};

class KillCounter : public PoEPlugin {
public:

    LocalPlayer* player;
    std::unordered_map<int, AreaMonsters*> latest_areas;
    AreaMonsters* monsters;
    Monsters near_monsters;
    int num_of_monsters, num_of_minions;
    int total_monsters, num_of_killed;

    KillCounter() : PoEPlugin("KillCounter", "0.1"), player(nullptr), monsters(nullptr) {
        num_of_monsters = num_of_minions = 0;
        total_monsters = num_of_killed = 0;
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        player = local_player;
    }

    void on_area_changed(AreaTemplate* world_area, int hash_code) {
        near_monsters.clear();
        if (latest_areas.find(hash_code) == latest_areas.end()) {
            latest_areas[hash_code] = new AreaMonsters();
        }
        monsters = latest_areas[hash_code];
        total_monsters = num_of_killed = 0;
    }

    void on_entity_changed(EntityList& entities, EntityList& removed, EntityList& added) {
        int n_monsters = 0, charges = 0, n_minions = 0;
        
        /* treat the removed near monsters as killed. */
        for (auto& i : removed) {
            shared_ptr<Entity>& entity = i.second;
            if (entity->is_monster) {
                if (near_monsters.find(i.first) != near_monsters.end())
                    monsters->killed.insert(i.first);
            }
        }

        near_monsters.clear();
        for (auto& i : entities) {
            shared_ptr<Entity>& entity = i.second;
            if (entity->is_monster) {
                monsters->total.insert(i.first);

                if (entity->is_dead()) {
                    monsters->killed.insert(i.first);
                    continue;
                }

                if (entity->is_neutral) {
                    n_minions++;
                    continue;
                }

                int dist = player->dist(*entity);
                if (dist < 100) {
                    n_monsters++;
                    charges += 1 + (1 << (entity->rarity & 0x3 - 1)) * 2.5;
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

        if (num_of_killed != monsters->killed.size() || total_monsters != monsters->total.size()) {
            total_monsters = monsters->total.size();
            num_of_killed = monsters->killed.size();
            PostThreadMessage(thread_id, WM_KILLED, num_of_killed, total_monsters);
        }
    }
};
