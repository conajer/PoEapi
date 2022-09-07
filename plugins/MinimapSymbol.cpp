/*
* MinimapSymbol.cpp, 11/6/2020 3:12 PM
*/

#include <regex>
#include <list>
#include <map>
#include <math.h>

#define RAND(min, max) (min + (static_cast<float>(rand()) / RAND_MAX * (max - min)))

struct BubbleText {
    wstring text;
    int color;
    float x;
    float y;
    float speed_x;
    float speed_y;
    float opacity;
};

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
    const wchar_t* font_name = L"Fontin SmallCaps";
    int font_size = 12;
    std::wregex heist_regex;
    std::wregex ignored_heist_chests;

    // strongbox
    std::wregex valuable_strongbox;

    // damage numbers
    int min_damage = 10000;
    float speed_x = 1.0;
    float speed_y = 1.0;
    int style = 0;
    int max_damage = 0;
    int max_damage_count = 0;
    int decay_time;
    std::list<BubbleText> damage_numbers;

    bool show_player = true;
    bool show_npc = true;
    bool show_minions = true;
    bool show_damage = true;

    // minimal size of the symbols
    int min_size = 4;
    float opacity = .8;

    int entity_colors[16] = {0xfefefe, 0x5882fe, 0xfefe16, 0xf28930,    // monster
                             0x7f7f7f, 0x2c417f, 0x7f7f3b, 0x794498,    // corpse
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

    MinimapSymbol() : PoEPlugin(L"MinimapSymbol", "0.24"),
        ignored_delve_chests(L"Armour|Weapon|Generic|NoDrops|Encounter"),
        heist_regex(L"HeistChest(Secondary|RewardRoom(Agility|BruteForce|CounterThaumaturge|Deception|Demolition|Engineering|LockPicking|Perception|TrapDisarmament|))(.*)(Military|Robot|Science|Thug)"),
        ignored_heist_chests(L"Armour|Weapons|Corrupted|Gems|Jewellery|Jewels|QualityCurrency|Talisman|Trinkets|Uniques"),
        valuable_strongbox(L"Cartographer|Divination|Scarab|VaalTempleChest")
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
        add_property(L"showDamage", &show_damage, AhkBool);
        add_property(L"minDamage", &min_damage, AhkInt);
        add_property(L"style", &style, AhkInt);
        add_property(L"speedX", &speed_x, AhkFloat);
        add_property(L"speedY", &speed_y, AhkFloat);

        add_method(L"setFontSize", this, (MethodType)&MinimapSymbol::set_font_size, AhkVoid, ParamList{AhkInt});
        add_method(L"setIgnoredDelveChests", this, (MethodType)&MinimapSymbol::set_ignored_delve_chests, AhkVoid, ParamList{AhkWString});
        add_method(L"setIgnoredHeistChests", this, (MethodType)&MinimapSymbol::set_ignored_heist_chests, AhkVoid, ParamList{AhkWString});
    }

    void set_ignored_delve_chests(const wchar_t* regex_string) {
        ignored_delve_chests.assign(regex_string);
    }

    void set_ignored_heist_chests(const wchar_t* regex_string) {
        ignored_heist_chests.assign(regex_string);
    }

    void set_font_size(int font_size) {
        this->font_size = font_size;
        poe->set_font(font_name, font_size);
    }

    void init_params() {
        OverlayMap* minimap = poe->in_game_ui->get_overlay_map();
        Rect r = minimap->get_rect();
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

        Vector3 player_pos = player->pos;
        player_pos.z = 0.0f;
        poe->in_game_state->transform(player_pos);

        Point pos = minimap->get_pos();
        shift_x = pos.x - player_pos.x + minimap->shift_x();
        shift_y = pos.y - player_pos.y + minimap->shift_y();
        scale = 1. / factor * minimap->zoom();
    }

    void draw_entity(Entity* e, int index, int size) {
        Vector3 pos = e->pos;
        pos.x = player->pos.x + (pos.x - player->pos.x) * scale;
        pos.y = player->pos.y + (pos.y - player->pos.y) * scale;
        pos.z = pos.z * scale;
        poe->in_game_state->transform(pos);

        int x = pos.x + shift_x;
        int y = pos.y + shift_y;
        if (e->rarity == 2 && e->is_beast) {
            poe->draw_text(e->name(), x, y, 0xffff52, 0x0c0c0c, 1.0, 1);
        } else {
            poe->fill_circle(x, y, size, entity_colors[index], opacity);
            if (min_size >= 4 && e->rarity >= 2)
                poe->draw_circle(x, y, size + 2, entity_colors[index], 1);
        }

        if (show_damage && e->damage_taken > min_damage) {
            BubbleText dmg_num;
            pos = e->pos;
            pos.z -= e->bounds.z;
            poe->in_game_state->transform(pos);

            wchar_t buffer[32];
            if (e->damage_taken < 1000)
                swprintf(buffer, L"%d", e->damage_taken);
            else if (e->damage_taken < 100000)
                swprintf(buffer, L"%.1fK", e->damage_taken / 1000.);
            else if (e->damage_taken < 10000000)
                swprintf(buffer, L"%dK", e->damage_taken / 1000);
            else if (e->damage_taken > 100000000)
                swprintf(buffer, L"%.1fM", e->damage_taken / 1000000.);
            else
                swprintf(buffer, L"%dM", e->damage_taken / 1000000);

            dmg_num.text = buffer;
            dmg_num.color = (e->damage_taken < 1000000) ? entity_colors[0] : entity_colors[2];
            if (e->damage_taken > max_damage) {
                if (max_damage_count++ > 10)
                    dmg_num.color = entity_colors[3];
                decay_time = GetTickCount() + 1000;
                max_damage = e->damage_taken;
            }
            dmg_num.x = pos.x;
            dmg_num.y = pos.y;
            switch (style) {
                case 1:
                    dmg_num.speed_x = RAND(-1, 1) * 3.0 * speed_x;
                    dmg_num.speed_y = RAND(0, 1) * 2.0 * speed_y;
                    break;

                case 2:
                    dmg_num.speed_x = RAND(-1, 1) * 3.0 * speed_x;
                    dmg_num.speed_y = RAND(-1, 1) * 2.0 * speed_y;
                    break;

                default:
                    dmg_num.speed_x = 0;
                    dmg_num.speed_y = 2.0 * speed_y;
            }
            dmg_num.opacity = 1.0;

            damage_numbers.push_back(dmg_num);
        }
        e->damage_taken = 0;
    }

    void draw_delve_chest(Entity* e) {
        if (std::regex_search(e->path, ignored_delve_chests))
            return;

        Targetable* targetable = e->get_component<Targetable>();
        if (!targetable || !targetable->is_targetable())
            return;

        Vector3 pos = e->pos;
        pos.x = player->pos.x + (pos.x - player->pos.x) * scale;
        pos.y = player->pos.y + (pos.y - player->pos.y) * scale;
        pos.z = 0.0f;
        poe->in_game_state->transform(pos);

        int color = 0x7f7f7f;
        for (auto& i : chest_colors) {
            if (e->path.find(i.first) != string::npos) {
                color = i.second;
                break;
            }
        }

        int x = pos.x + shift_x;
        int y = pos.y + shift_y;
        poe->fill_circle(x, y, min_size + 7, 0xffffff, 0.8);
        poe->fill_circle(x, y, min_size + 4, color, 0.8);
    }

    void draw_heist_chest(Entity* e) {
        Targetable* targetable = e->get_component<Targetable>();
        if (!targetable || !targetable->is_targetable())
            return;

        std::wsmatch match;
        if (std::regex_search(e->path, match, heist_regex) && match.size() > 0) {
            Vector3 pos = e->pos;
            pos.x = player->pos.x + (pos.x - player->pos.x) * scale;
            pos.y = player->pos.y + (pos.y - player->pos.y) * scale;
            pos.z = pos.z * scale;

            int x = pos.x + shift_x;
            int y = pos.y + shift_y;
            poe->in_game_state->transform(pos);

            wstring heist_type = match[3].str();
            if (std::regex_search(heist_type, ignored_heist_chests))
                poe->draw_text(heist_type, pos.x, pos.y, 0xadadad, 0x0c0c0c, .8, 1);
            else
                poe->draw_text(heist_type, pos.x, pos.y, 0xffff52, 0x0c0c0c, 1.0, 1);
        }
    }

    void draw_strongbox(Entity* e) {
        Targetable* targetable = e->get_component<Targetable>();
        if (!targetable || !targetable->is_targetable())
            return;

        Vector3 pos = e->pos;
        pos.x = player->pos.x + (pos.x - player->pos.x) * scale;
        pos.y = player->pos.y + (pos.y - player->pos.y) * scale;
        pos.z = 0.0f;
        poe->in_game_state->transform(pos);

        int x = pos.x + shift_x;
        int y = pos.y + shift_y;
        float dw = min_size * 2;
        float dh = min_size * 1.5;

        if (std::regex_search(e->path, valuable_strongbox))
            poe->fill_rect(x - dw, y - dh, x + dw, y + dh, 0xC70039, 0.8);
        else
            poe->fill_rect(x - dw, y - dh, x + dw, y + dh, 0xf1c40f, 0.8);
        poe->draw_rect(x - dw, y - dh, x + dw, y + dh, 0x0c0c0c, 0.8);
        poe->draw_rect(x - dw, y - dh, x + dw, y - dh / 3, 0x0c0c0c, 0.8);
    }

    void render() {
        init_params();
        std::lock_guard<std::mutex> guard(drawn_entities_mutex);
        for (auto& i : drawn_entities) {
            if (!i.second)
                continue;

            Entity* entity = i.second.get();
            if (entity->is_npc) {
                if (show_npc)
                    draw_entity(entity, 9, min_size + 2);
            } else if (entity->is_monster) {
                if (show_monsters) {
                    if (entity->is_dead() && entity->damage_taken <= min_damage) {
                        if (show_corpses)
                            draw_entity(entity, 4 + entity->rarity, min_size + entity->rarity);
                        else
                            ignored_entities.push_back(i.first);
                    } else if (entity->is_minion) {
                        if (show_minions)
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
                    draw_delve_chest(entity);
                else if (show_heist_chests && entity->path.find(L"/HeistChest") != wstring::npos)
                    draw_heist_chest(entity);
                else if (entity->path.find(L"/StrongBoxes") != wstring::npos)
                    draw_strongbox(entity);
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

        if (show_damage) {
            for (auto i = damage_numbers.begin(); i != damage_numbers.end(); ++i) {
                if (i->opacity > 0) {
                    poe->draw_big_text(i->text, i->x, i->y, i->color, 0, i->opacity, 1);
                    i->x += i->speed_x;
                    i->y -= i->speed_y;
                    i->opacity -= 0.005 * (1 + fabs(speed_y));
                } else {
                    i = damage_numbers.erase(i);
                }
            }

            if (max_damage > 0 && GetTickCount() > decay_time) {
                max_damage *= 0.95;
                decay_time = decay_time + 1000;
                if (max_damage < min_damage) {
                    max_damage = 0;
                    max_damage_count = 0;
                }
            }
        }
    }

    void on_load(PoE& poe, int owner_thread_id) {
        PoEPlugin::on_load(poe, owner_thread_id);
        poe.set_font(font_name, font_size);
    }

    void on_area_changed(AreaTemplate* world_area, int hash_code, LocalPlayer* player) {
        std::lock_guard<std::mutex> guard(drawn_entities_mutex);
        drawn_entities.clear();
        ignored_entities.clear();
        damage_numbers.clear();
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
