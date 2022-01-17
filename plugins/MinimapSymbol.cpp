/*
* MinimapSymbol.cpp, 11/6/2020 3:12 PM
*/

#include <regex>
#include <map>
#include <math.h>

class MinimapSymbol : public PoEPlugin {
public:

    std::mutex drawn_entities_mutex;
    std::unordered_map<int, shared_ptr<Entity>> drawn_entities;
    std::vector<int> ignored_entities;
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
    bool texture_enabled = true;
    bool texture_loaded = false;

    int entity_colors[16] = {0xfefefe, 0x5882fe, 0xfefe16, 0xb57741,    // monster
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

    MinimapSymbol() : PoEPlugin(L"MinimapSymbol", "0.15"),
        ignored_delve_chests(L"Armour|Weapon|Generic|NoDrops|Encounter"),
        heist_regex(L"HeistChest(Secondary|RewardRoom)(.*)(Military|Robot|Science|Thug)")
    {
        add_property(L"showMonsters", &show_monsters, AhkBool);
        add_property(L"showCorpses", &show_corpses, AhkBool);
        add_property(L"rarity", &rarity, AhkInt);
        add_property(L"showDelveChests", &show_delve_chests, AhkBool);
        add_property(L"showHeistChests", &show_heist_chests, AhkBool);
        add_property(L"showPlayer", &show_player, AhkBool);
        add_property(L"showNPC", &show_npc, AhkBool);
        add_property(L"showMinions", &show_minions, AhkBool);
        add_property(L"minSize", &min_size, AhkInt);
        add_property(L"opacity", &opacity, AhkFloat);
        add_property(L"textureEnabled", &texture_enabled, AhkBool);

        add_method(L"setIgnoredDelveChests", this, (MethodType)&MinimapSymbol::set_ignored_delve_chests, AhkVoid, ParamList{AhkWString});
    }

    void set_ignored_delve_chests(const wchar_t* regex_string) {
        ignored_delve_chests.assign(regex_string);
    }

    void load_textures() {
        ID2D1BitmapRenderTarget* bitmap_render;
        ID2D1SolidColorBrush* brush;

        for (int i = 0; i < 11; ++i) {
            float size = min_size + ((i < 8) ? (i & 0x3) : (1 << (i & 0x3)));
            if ((i & 0x3) == 0x3)
                size += 2;

            float x = size, y = size;
            poe->rt->CreateCompatibleRenderTarget(D2D1::SizeF(size * 2, size * 2), &bitmap_render);
            bitmap_render->CreateSolidColorBrush(D2D1::ColorF(0), &brush);
            bitmap_render->BeginDraw();
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

    void init_params() {
        if (Render* render = player->get_render()) {
            player_pos = render->position();
            player_pos.z = 0.0f;

            Vector3 pos = player_pos;
            OverlayMap* map = poe->in_game_ui->get_overlay_map();
            Rect r = map->get_rect();
            if (r.w > 0 || r.h > 0) {
                if (!is_clipped) {
                    poe->push_rectangle_clip(r.x, r.y, r.x + r.w, r.y + r.h);
                    is_clipped = true;
                }
            } else {
                if (is_clipped) {
                    poe->pop_rectangle_clip();
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
        if (Render* render = e->get_render()) {
            Vector3 pos = render->position();
            pos.x = player_pos.x + (pos.x - player_pos.x) * scale;
            pos.y = player_pos.y + (pos.y - player_pos.y) * scale;
            pos.z = pos.z * scale;
            poe->in_game_state->transform(pos);

            int x = pos.x + shift_x;
            int y = pos.y + shift_y;
            if (texture_enabled) {
                if (e->rarity == 3 && !e->is_npc)
                    size += 2;
                poe->draw_bitmap(textures[index], x - size, y - size, x + size, y + size);
                if (e->rarity == 2) {
                    poe->draw_circle(x, y, size + 2, 0xffff00, 2);
                    if (e->name().find(L"Undying Evangelist") != wstring::npos)
                        poe->draw_circle(x, y, size + 5, 0xffff00, 2);
                }
            } else {
                poe->fill_circle(x, y, size, entity_colors[index], opacity);
                if (e->rarity == 3)
                    poe->draw_circle(x, y, size + 2, 0xff0000, 2);
            }
        }
    }

    void draw_delve_chests(Entity* e) {
        if (std::regex_search(e->path, ignored_delve_chests))
            return;

        Targetable* targetable = e->get_component<Targetable>();
        if (!targetable || !targetable->is_targetable())
            return;

        if (Render* render = e->get_render()) {
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
            poe->fill_circle(pos.x, pos.y, min_size + 7, 0xffffff, 0.8);
            poe->fill_circle(pos.x, pos.y, min_size + 4, color, 0.8);
        }
    }

    void draw_heist_chests(Entity* e) {
        Targetable* targetable = e->get_component<Targetable>();
        if (!targetable || !targetable->is_targetable())
            return;

        if (Render* render = e->get_render()) {
            Vector3 pos = render->position();
            Vector3 bound = render->position();
            pos.z += 2 * bound.z;
            poe->in_game_state->transform(pos);

            std::wsmatch match;
            if (std::regex_search(e->path, match, heist_regex) && match.size() > 0)
                poe->draw_text(match[2].str(), pos.x, pos.y, 0xffffff, 0xad1616, 1.0, 1);
        }
    }

    void render() {
        if (!texture_loaded && texture_enabled)
            load_textures();

        init_params();
        std::lock_guard<std::mutex> guard(drawn_entities_mutex);
        for (auto& i : drawn_entities) {
            if (!i.second)
                continue;

            Entity* entity = i.second.get();
            if (entity->is_npc) {
                if (!entity->is_monster && show_npc)
                    draw_entity(entity, 9, min_size + 2);
            } else if (entity->is_monster) {
                if (show_monsters) {
                    if (entity->is_dead()) {
                        if (show_corpses)
                            draw_entity(entity, 4 + entity->rarity, min_size + entity->rarity);
                        else
                            ignored_entities.push_back(i.first);
                    } else if (entity->is_neutral) {
                        if (show_minions && entity->is_minion)
                            draw_entity(entity, 8, min_size + 1);
                        else
                            ignored_entities.push_back(i.first);
                    } else if (entity->rarity >= rarity) {
                        draw_entity(entity, entity->rarity, min_size + entity->rarity);
                    } else {
                        ignored_entities.push_back(i.first);
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
                    ignored_entities.push_back(i.first);
            } else {
                ignored_entities.push_back(i.first);
            }
        }

        for (auto i : ignored_entities)
            drawn_entities.erase(i);

        if (is_clipped) {
            poe->pop_rectangle_clip();
            is_clipped = false;
        }
    }

    void on_area_changed(AreaTemplate* world_area, int hash_code, LocalPlayer* player) {
        drawn_entities.clear();
        ignored_entities.clear();
    }

    void on_entity_changed(EntityList& entities, EntityList& removed, EntityList& added) {
        std::lock_guard<std::mutex> guard(drawn_entities_mutex);
        if (drawn_entities.empty()) {
            for (auto& i : entities)
                drawn_entities[i.first] = i.second;
        }

        for (auto& i : added)
            drawn_entities[i.first] = i.second;

        for (auto& i : removed)
            drawn_entities.erase(i.first);
    }
};
