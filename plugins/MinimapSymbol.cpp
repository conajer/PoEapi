/*
* MinimapSymbol.cpp, 11/6/2020 3:12 PM
*/

#include <map>

class MinimapSymbol : public PoEPlugin {
public:

    Vector3 player_pos;
    float shift_x, shift_y;
    float scale;
    float factor = 6.9f;
    bool is_clipping = false;

    int entity_colors[16] = {0xfefefe, 0x5882fe, 0xfefe76, 0xaf5f1c,    // normal, magic, rare, unique
                             0x7f7f7f, 0x2c417f, 0x7f7f3b, 0x57280e,    // is dead
                             0xfe00, 0, 0, 0,                           // is minion
                             0x7f00, 0, 0, 0};
    
    std::map<wstring, int> chest_colors = {{L"AzuriteVein", 0xff}, 
                                           {L"Resonator", 0xff7f},
                                           {L"Fossil", 0xffff},
                                           {L"Currency|Map", 0xffffff},
                                           {L"SuppliesDynamite", 0x7f0000},
                                           {L"SuppliesFlares", 0xff0000},
                                           {L"Unique", 0xffff}};

    MinimapSymbol() : PoEPlugin(L"MinimapSymbol", "0.1") {
    }
    
    void reset() {
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

    void draw_delve_chests(Entity* e) {
        Targetable* targetable = e->get_component<Targetable>();
        if (!targetable || !targetable->is_targetable() || !e->has_component("Chest"))
            return;

        Render* render = e->get_component<Render>();
        if (render) {
            int w1 = 9, w2 = 6;
            Vector3 pos = render->position();
            Vector3 bounds = render->bounds();
            pos.x = player_pos.x + (pos.x - player_pos.x) * scale;
            pos.y = player_pos.y + (pos.y - player_pos.y) * scale;
            pos.z = 0.0f;
            poe->in_game_state->transform(pos);

            int color = 0x7f7f7f;
            for (auto& i : chest_colors) {
                if (e->path.find(i.first) != string::npos) {
                    color = i.second;
                    break;
                }
            }

            pos.x += shift_x;
            pos.y += shift_y;
            poe->hud->fill_rounded_rect(pos.x - w1, pos.y - w1, pos.x + w1, pos.y + w1, w1, w1, 0xffffff);
            poe->hud->fill_rounded_rect(pos.x - w2, pos.y - w2, pos.x + w2, pos.y + w2, w2, w2, color);
        }
    }

    void on_entity_changed(EntityList& entities, EntityList& removed, EntityList& added) {
        if (!player || !poe->hud)
            return;

        reset();
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                return;
            }

            Entity* entity = i.second.get();
            poe->get_pos(entity);
            if (entity->is_monster || entity->has_component("NPC"))
                draw_entity(entity);
            else if (entity->path.find(L"Delve") != string::npos)
                draw_delve_chests(entity);
        }
        poe->hud->end_draw();
    }
};
