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

    Vector3 player_pos;
    float shift_x, shift_y;
    float scale, factor = 6.9f;
    bool is_clipping = false;

    int entity_colors[16] = {0xfefefe, 0x5882fe, 0xfefe76, 0xaf5f1c,    // normal, magic, rare, unique
                             0x7f7f7f, 0x2c417f, 0x7f7f3b, 0x57280e,    // is dead
                             0xfe00, 0, 0, 0,                           // is minion
                             0x7f00, 0, 0, 0};
    
    KillCounter() : PoEPlugin(L"KillCounter", "0.2"), current_area(nullptr) {
        num_of_monsters = num_of_minions = 0;
        total_monsters = num_of_killed = 0;
    }
    
    void reset_minimap_icons() {
        poe->hud->begin_draw();
        poe->hud->clear();

        Render* render = player->get_component<Render>();
        if (render) {
            Vector3 bounds = render->bounds();
            player_pos = render->position();
            player_pos.z = 0.0f;

            Vector3 pos = player_pos;
            OverlayMap* map = poe->in_game_state->in_game_ui()->get_overlay_map();
            Rect r = map->get_rect();
            if (r.w > 0 || r.h > 0) {
                if (!is_clipping) {
                    poe->hud->end_draw();
                    poe->hud->begin_draw();
                    poe->hud->push_rectangle_clip(r.x, r.y, r.x + r.w, r.y + r.h);
                    is_clipping = true;
                }
            } else {
                if (is_clipping) {
                    poe->hud->pop_rectangle_clip();
                    is_clipping = false;
                }
            }
            poe->in_game_state->transform(pos);
            Point p = map->get_pos();
            shift_x = p.x - pos.x + map->shift_x();
            shift_y = p.y - pos.y + map->shift_y();
            scale = 1. / factor * map->zoom();
        }
    }

    void draw_entity(Entity* e) {
        if (!poe->hud)
            return;

        int index = e->rarity | (e->is_dead() ? 4 : 0) | (e->is_neutral ? 8 : 0);
        int w = e->rarity + 2 + (e->is_neutral ? 1 : 0) + (!e->is_monster ? 2 : 0);
        Render* render = e->get_component<Render>();
        if (render) {
            Vector3 pos = render->position();
            Vector3 bounds = render->bounds();
            pos.x = player_pos.x + (pos.x - player_pos.x) * scale;
            pos.y = player_pos.y + (pos.y - player_pos.y) * scale;
            pos.z = 0.0f;
            poe->in_game_state->transform(pos);

            pos.x += shift_x;
            pos.y += shift_y;
            poe->hud->fill_rect(pos.x - w, pos.y - w, pos.x + w, pos.y + w, entity_colors[index]);
            if (e->rarity == 3) {
                w += 1;
                poe->hud->draw_rect(pos.x - w, pos.y - w, pos.x + w, pos.y + w, 0xff0000, 2);
            }
        }
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

        if (poe->hud)
            reset_minimap_icons();

        nearby_monsters.clear();
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                return;
            }

            Entity* entity = i.second.get();
            poe->get_pos(entity);

            if (entity->has_component("NPC")) {
                draw_entity(entity);
                continue;
            }

            if (entity->is_monster) {
                draw_entity(entity);
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

        if (poe->hud)
            poe->hud->end_draw();

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
