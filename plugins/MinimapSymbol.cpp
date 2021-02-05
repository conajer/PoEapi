/*
* MinimapSymbol.cpp, 11/6/2020 3:12 PM
*/

#include <map>
#include <math.h>

class MonsterPack {
public:

    int margin = 5;
    float max_radius = 500.;
    int count;
    int rarity;
    int l, t, r, b;
    float cx, cy;

    MonsterPack(Entity* monster, int x, int y) : cx(x), cy(y) {
        l = x - margin;
        t = y - margin;
        r = x + margin;
        b = y + margin;

        count = 1;
        rarity = monster->rarity;
    }

    bool add(Entity* monster, int x, int y) {
        float dist = sqrtf((x - cx) * (x - cx) + (y - cy) * (y - cy));
        if (dist > max_radius)
            return false;

        count++;
        if (rarity < monster->rarity)
            rarity = monster->rarity;

        if (x < l) l = x - margin;
        if (x > r) r = x + margin;
        if (y < t) t = y - margin;
        if (y > b) b = y + margin;

        cx = (l + r) / 2;
        cy = (t + b) / 2;

        return true;
    }
};

class MinimapSymbol : public PoEPlugin {
public:

    Vector3 player_pos;
    float shift_x, shift_y;
    float scale;
    float factor = 6.9f;
    bool is_clipping = false;
    bool show_delve_chests = true;
    int size = 8;
    int border_color = 0xffffff;
    std::wregex ignored_delve_chests;
    bool show_monsters = true;
    bool show_minions = true;
    bool show_corpses = false;
    int rarity = 0;
    std::vector<MonsterPack> monster_packs;
    bool show_packs = false;

    int entity_colors[16] = {0xfefefe, 0x5882fe, 0xfefe76, 0xaf5f1c,    // normal, magic, rare, unique
                             0x7f7f7f, 0x2c417f, 0x7f7f3b, 0x57280e,    // is dead
                             0xfe00, 0, 0, 0,                           // is minion
                             0x7f00, 0, 0, 0};
    
    std::map<wstring, int> chest_colors = {{L"AzuriteVein", 0xff}, 
                                           {L"Resonator", 0xff7f},
                                           {L"Fossil", 0xffff},
                                           {L"Currency", 0xffffff},
                                           {L"Map", 0xffffff},
                                           {L"SuppliesDynamite", 0x7f0000},
                                           {L"SuppliesFlares", 0xff0000},
                                           {L"Unique", 0xffff}};

    MinimapSymbol() : PoEPlugin(L"MinimapSymbol", "0.4") {
        add_property(L"showDelveChests", &show_delve_chests, AhkBool);
        add_property(L"size", &size, AhkInt);
        add_property(L"borderColor", &border_color, AhkInt);
        add_property(L"showMonsters", &show_monsters, AhkBool);
        add_property(L"showMinions", &show_minions, AhkBool);
        add_property(L"showCorpses", &show_corpses, AhkBool);
        add_property(L"rarity", &rarity, AhkInt);
        add_property(L"showPacks", &show_packs, AhkBool);
        add_method(L"setIgnoredDelveChests", this, (MethodType)&MinimapSymbol::set_ignored_delve_chests, AhkVoid, ParamList{AhkWString});
        add_method(L"getPacks", this, (MethodType)&MinimapSymbol::get_packs, AhkObject);

        set_ignored_delve_chests(L"Armour|Weapon|Generic|NoDrops|Encounter");
    }

    void set_ignored_delve_chests(const wchar_t* regex_string) {
        ignored_delve_chests.assign(regex_string);
    }

    AhkObjRef* get_packs() {
        AhkTempObj temp_packs;
        for (auto& p : monster_packs) {
            AhkObj pack;
            pack.__set(L"x", p.cx, AhkInt, L"y", p.cy, AhkInt,
                L"l", p.l, AhkInt, L"t", p.t, AhkInt, L"r", p.r, AhkInt, L"b", p.b, AhkInt,
                L"count", p.count, AhkInt, L"rarity", p.rarity, AhkInt,
                nullptr);
            temp_packs.__set(L"", (AhkObjRef*)pack, AhkObject, nullptr);
        }

        return temp_packs;
    }
    
    void reset() {
        poe->hud->begin_draw();
        poe->hud->clear();
        monster_packs.clear();

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
        if (e->is_monster && ((!e->is_neutral && e->rarity < rarity)
                              || (!show_corpses && e->is_dead()) || (!show_minions && e->is_neutral)))
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

            if (show_packs && (e->is_monster && !e->is_neutral)) {
                Point p = poe->get_pos(e);
                for (auto& pack : monster_packs) {
                    if (pack.add(e, p.x, p.y))
                        return;
                }
                monster_packs.push_back(MonsterPack(e, p.x, p.y));
            }
        }
    }

    void draw_monster_packs() {
        for (auto pack : monster_packs)
            poe->hud->draw_rect(pack.l, pack.t, pack.r, pack.b, 0xff00, 1);
    }

    void draw_delve_chests(Entity* e) {
        if (std::regex_search(e->path, ignored_delve_chests))
            return;

        Targetable* targetable = e->get_component<Targetable>();
        if (!targetable || !targetable->is_targetable() || !e->has_component("Chest"))
            return;

        Render* render = e->get_component<Render>();
        if (render) {
            int w1 = size + 3, w2 = size;
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
            poe->hud->fill_rounded_rect(pos.x - w1, pos.y - w1, pos.x + w1, pos.y + w1, w1, w1, border_color);
            poe->hud->fill_rounded_rect(pos.x - w2, pos.y - w2, pos.x + w2, pos.y + w2, w2, w2, color);
        }
    }

    void on_entity_changed(EntityList& entities, EntityList& removed, EntityList& added) {
        if (!poe->hud)
            return;

        reset();
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                return;
            }

            Entity* entity = i.second.get();
            if (show_monsters && (entity->is_monster || entity->has_component("NPC")))
                draw_entity(entity);
            else if (show_delve_chests && entity->path.find(L"/DelveChests") != string::npos)
                draw_delve_chests(entity);
        }

        if (show_packs) {
            draw_monster_packs();
            monster_packs.clear();
        }

        poe->hud->end_draw();
    }
};
