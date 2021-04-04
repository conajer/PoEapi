/*
* MinimapSymbol.cpp, 11/6/2020 3:12 PM
*/

#include <regex>
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

    std::unordered_set<int> ignored_entities;
    Vector3 player_pos;

    // overlapped map 
    float shift_x, shift_y;
    float scale;
    float factor = 6.9f;
    bool is_clipped = false;

    // monsters
    bool show_monsters = true;
    bool show_corpses = false;
    int rarity = 0;
    bool show_packs = false;
    std::vector<MonsterPack> monster_packs;

    // delve chests
    bool show_delve_chests = true;
    std::wregex ignored_delve_chests;

    // heist chests
    bool show_heist_chests = true;
    std::wregex heist_regex;

    bool show_player = true;
    bool show_npc = true;
    bool show_minions = true;

    // minimal size of the symbols
    int min_size = 4;
    float opacity = .8;

    ID2D1Bitmap* textures[16] = {};
    bool use_texture = true;
    bool texture_loaded = false;

    int entity_colors[16] = {0xfefefe, 0x5882fe, 0xfefe76, 0xb57741,    // monster
                             0x7f7f7f, 0x2c417f, 0x7f7f3b, 0x57280e,    // corpse
                             0x00fe00,                                  // minion
                             0xe0ffff,                                  // NPC
                             0xfe00fe};                                 // player
    
    std::map<wstring, int> chest_colors = {{L"AzuriteVein", 0xff}, 
                                           {L"Resonator", 0xff7f},
                                           {L"Fossil", 0xffff},
                                           {L"Currency", 0xffffff},
                                           {L"Map", 0xffffff},
                                           {L"SuppliesDynamite", 0x7f0000},
                                           {L"SuppliesFlares", 0xff0000},
                                           {L"Unique", 0xffff}};

    MinimapSymbol() : PoEPlugin(L"MinimapSymbol", "0.10"),
        ignored_delve_chests(L"Armour|Weapon|Generic|NoDrops|Encounter"),
        heist_regex(L"HeistChest(Secondary|RewardRoom)(.*)(Military|Robot|Science|Thug)")
    {
        add_property(L"showMonsters", &show_monsters, AhkBool);
        add_property(L"showCorpses", &show_corpses, AhkBool);
        add_property(L"rarity", &rarity, AhkInt);
        add_property(L"showPacks", &show_packs, AhkBool);
        add_property(L"showDelveChests", &show_delve_chests, AhkBool);
        add_property(L"showHeistChests", &show_heist_chests, AhkBool);
        add_property(L"showPlayer", &show_player, AhkBool);
        add_property(L"showNPC", &show_npc, AhkBool);
        add_property(L"showMinions", &show_minions, AhkBool);
        add_property(L"minSize", &min_size, AhkInt);
        add_property(L"opacity", &opacity, AhkFloat);
        add_property(L"useTexture", &use_texture, AhkBool);

        add_method(L"getPacks", this, (MethodType)&MinimapSymbol::get_packs, AhkObject);
        add_method(L"setIgnoredDelveChests", this, (MethodType)&MinimapSymbol::set_ignored_delve_chests, AhkVoid, ParamList{AhkWString});
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
        PoEPlugin::reset();
        clear();
    }

    void clear() {
        if (poe->hud) {
            poe->hud->begin_draw();
            poe->hud->clear();
            poe->hud->end_draw();
        }
    }

    void load_textures() {
        if (poe->hud) {
            ID2D1BitmapRenderTarget* bitmap_render;
            ID2D1SolidColorBrush* brush;

            for (int i = 0; i < 11; ++i) {
                float size = min_size + ((i < 8) ? (i & 0x3) : (1 << (i & 0x3)));
                if ((i & 0x3) == 0x3)
                    size += 2;

                float x = size, y = size;
                poe->hud->render->CreateCompatibleRenderTarget(D2D1::SizeF(size * 2, size * 2), &bitmap_render);
                bitmap_render->BeginDraw();
                bitmap_render->CreateSolidColorBrush(D2D1::ColorF(0), &brush);
                if ((i & 0x3) == 0x3) {
                    brush->SetColor(D2D1::ColorF(0xff0000, opacity));
                    bitmap_render->DrawEllipse({{x, y}, size, size}, brush, 2);
                    size -= 2;
                }
                brush->SetColor(D2D1::ColorF(entity_colors[i], opacity));
                bitmap_render->FillEllipse({{x, y}, size, size}, brush);
                bitmap_render->EndDraw();
                bitmap_render->GetBitmap(&textures[i]);
                brush->Release();
                bitmap_render->Release();
            }

            texture_loaded = true;
        }
    }

    void initialize() {
        if (use_texture && !texture_loaded)
            load_textures();

        Render* render = player->get_component<Render>();
        if (render) {
            player_pos = render->position();
            player_pos.z = 0.0f;

            Vector3 pos = player_pos;
            OverlayMap* map = poe->in_game_ui->get_overlay_map();
            Rect r = map->get_rect();
            if (r.w > 0 || r.h > 0) {
                if (!is_clipped) {
                    poe->hud->end_draw();
                    poe->hud->begin_draw();
                    poe->hud->push_rectangle_clip(r.x, r.y, r.x + r.w, r.y + r.h);
                    is_clipped = true;
                }
            } else {
                if (is_clipped) {
                    poe->hud->pop_rectangle_clip();
                    is_clipped = false;
                }
            }
            poe->in_game_state->transform(pos);
            Point p = map->get_pos();
            shift_x = p.x - pos.x + map->shift_x();
            shift_y = p.y - pos.y + map->shift_y();
            scale = 1. / factor * map->zoom();
        }
    }

    void draw_entity(Entity* e, int index, int size) {
        Render* render = e->get_component<Render>();
        if (render) {
            Vector3 pos = render->position();
            pos.x = player_pos.x + (pos.x - player_pos.x) * scale;
            pos.y = player_pos.y + (pos.y - player_pos.y) * scale;
            pos.z = pos.z * scale;
            poe->in_game_state->transform(pos);

            int x = pos.x + shift_x;
            int y = pos.y + shift_y;
            if (use_texture) {
                if (e->rarity == 3 && !e->is_npc)
                    size += 2;
                poe->hud->draw_bitmap(textures[index], x - size, y - size, x + size, y + size);
            } else {
                poe->hud->fill_circle(x, y, size, entity_colors[index], opacity);
                if (e->rarity == 3)
                    poe->hud->draw_circle(x, y, size + 2, 0xff0000, 2);
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
        if (!targetable || !targetable->is_targetable())
            return;

        Render* render = e->get_component<Render>();
        if (render) {
            Vector3 pos = render->position();
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
            poe->hud->fill_circle(pos.x, pos.y, min_size + 7, 0xffffff, 0.8);
            poe->hud->fill_circle(pos.x, pos.y, min_size + 4, color, 0.8);
        }
    }

    void draw_heist_chests(Entity* e) {
        Targetable* targetable = e->get_component<Targetable>();
        if (!targetable || !targetable->is_targetable())
            return;

        Render* render = e->get_component<Render>();
        if (render) {
            Vector3 pos = render->position();
            Vector3 bound = render->position();
            pos.z += 2 * bound.z;
            poe->in_game_state->transform(pos);

            std::wsmatch match;
            if (std::regex_search(e->path, match, heist_regex) && match.size() > 0)
                poe->hud->draw_text(match[2].str(), pos.x, pos.y, 0xffffff, 0xad1616, 1.0, 1);
        }
    }

    void on_area_changed(AreaTemplate* world_area, int hash_code, LocalPlayer* player) {
        ignored_entities.clear();
    }

    void on_entity_changed(EntityList& entities, EntityList& removed, EntityList& added) {
        if (!poe->hud)
            return;

        if (poe->in_game_ui->has_active_panel()) {
            clear();
            return;
        }

        poe->hud->begin_draw();
        poe->hud->clear();
        monster_packs.clear();
        initialize();
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                poe->hud->clear();
                poe->hud->end_draw();
                return;
            }

            if (ignored_entities.find(i.second->id) != ignored_entities.end())
                continue;

            Entity* entity = i.second.get();
            if (entity->is_npc) {
                if (show_npc)
                    draw_entity(entity, 9, min_size + 2);
            } else if (entity->is_monster) {
                if (show_monsters) {
                    if (entity->is_dead()) {
                        if (show_corpses)
                            draw_entity(entity, 4 + entity->rarity, min_size + entity->rarity);
                        else
                            ignored_entities.insert(i.second->id);
                    } else if (entity->is_neutral) {
                        if (show_minions)
                            draw_entity(entity, 8, min_size + 1);
                        else
                            ignored_entities.insert(i.second->id);
                    } else if (entity->rarity >= rarity) {
                        draw_entity(entity, entity->rarity, min_size + entity->rarity);
                    } else {
                        ignored_entities.insert(i.second->id);
                    }
                }
            } else if (entity->is_player) {
                if (show_player)
                    draw_entity(entity, 10, min_size + 4);
            } else if (entity->has_component("Chest")) {
                if (show_delve_chests && entity->path.find(L"/DelveChests") != wstring::npos)
                    draw_delve_chests(entity);
                else if (show_heist_chests && entity->path.find(L"/HeistChest") != wstring::npos)
                    draw_heist_chests(entity);
                else
                    ignored_entities.insert(i.second->id);
            } else {
                ignored_entities.insert(i.second->id);
            }
        }

        if (show_packs) {
            draw_monster_packs();
            monster_packs.clear();
        }

        poe->hud->end_draw();
    }
};
