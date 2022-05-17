/*
* AutoPickup.cpp, 9/18/2020 11:11 PM
*/

class AutoPickup : public PoEPlugin {
public:

    std::vector<string> entity_types = {
        "Chest",
        "MinimapIcon",
        "WorldItem",
    };

    std::vector<string> item_types = {
        "Stack",                // Currency/DivinationCard items
        "Map",                  // Maps
        "Quest",                // Quest items
        "HeistContract",        // Heist contracts
        "HeistBlueprint",       // Heist blueprints
        "HeistObjective",       // Heist objective items
    };

    std::map<int, shared_ptr<Entity>> ignored_entities;
    std::map<int, shared_ptr<Item>> dropped_items;
    std::mutex selected_item_mutex;
    shared_ptr<Entity> selected_item;
    RECT bounds;
    unsigned int range = 50, last_pickup = 0;
    bool ignore_chests = false;
    bool is_picking = false;
    bool event_enabled = false;
    int strict_level = 0;

    std::wregex generic_item_filter;
    std::wregex rare_item_filter;
    std::wregex misc_entity_filter;

    AutoPickup() : PoEPlugin(L"AutoPickup", "0.19") {
        add_property(L"range", &range, AhkInt);
        add_property(L"ignoreChests", &ignore_chests, AhkBool);
        add_property(L"eventEnabled", &event_enabled, AhkBool);
        add_property(L"strictLevel", &strict_level, AhkBool);

        add_method(L"setGenericItemFilter", this,(MethodType)&AutoPickup::set_generic_item_filter, AhkVoid, ParamList{AhkWString});
        add_method(L"setRareItemFilter", this, (MethodType)&AutoPickup::set_rare_item_filter, AhkVoid, ParamList{AhkWString});
        add_method(L"setMiscEntityFilter", this, (MethodType)&AutoPickup::set_misc_entity_filter, AhkVoid, ParamList{AhkWString});
        add_method(L"beginPickup", this, (MethodType)&AutoPickup::begin_pickup);
        add_method(L"stopPickup", this, (MethodType)&AutoPickup::stop_pickup);
        add_method(L"getDroppedItems", this, (MethodType)&AutoPickup::get_dropped_items, AhkObject);
        add_method(L"getItem", this, (MethodType)&AutoPickup::get_item, AhkObject, ParamList{AhkInt});

        set_generic_item_filter(L"Incubator|Scarab$|Quicksilver|Diamond|Basalt|Quartz");
        set_rare_item_filter(L"Jewels|Amulet|Rings|Belts");
        misc_entity_filter.assign(L"Monolith|DelveMineralVein|DelveMineralChest|Switch_Once|CraftingUnlocks|AreaTransition|Heist/Objects/Level/|HeistChestRewardRoom");
    }

    void reset() {
        PoEPlugin::reset();
        ignored_entities.clear();
        selected_item.reset();
        dropped_items.clear();
    }

    void set_generic_item_filter(const wchar_t* regex_string) {
        generic_item_filter.assign(regex_string);
    }

    void set_rare_item_filter(const wchar_t* regex_string) {
        rare_item_filter.assign(regex_string);
    }

    void set_misc_entity_filter(const wchar_t* regex_string) {
        log(regex_string);
        misc_entity_filter.assign(regex_string);
    }

    void begin_pickup() {
        if (!player || is_picking)
            return;

        ignored_entities.clear();
        selected_item.reset();
        is_picking = true;
        last_pickup = GetTickCount();

        GetClientRect(poe->window, &bounds);
        bounds.left += 200;
        bounds.top += 150;
        bounds.right -= 200;
        bounds.bottom -= 150;
    }

    void stop_pickup() {
        is_picking = false;
        selected_item.reset();
    }

    bool check_item(addrtype address) {
        Item item(address);

        if (item.has_component(item_types) >= 0)
            return true;

        if (item.get_links() == 6)
            return true;

        int rarity = item.get_rarity();
        int ilvl = item.get_item_level();
        switch (strict_level) {
        case 0:
            if (rarity == 3 || item.is_rgb())
                return true;
            if (rarity == 2 && !item.identified && ilvl >= 60 && ilvl < 75 && item.size <= 6)
                return true;

        case 1:
            if (item.get_sockets() == 6)
                return true;
            if (item.has_component("SkillGem"))
                return (item.get_quality() >= 5);

        case 2:
            if (ilvl >= 82 && item.get_influence_type())
                return true;
            if (item.is_synthesised())
                return true;
        
        default:
            if (std::regex_search(item.base_name(), generic_item_filter))
                return true;
            if (rarity == 2 && std::regex_search(item.path, rare_item_filter))
                return true;
        }

        return false;
    }

    AhkObjRef* get_dropped_items() {
        AhkObj items;
        for (auto& i : dropped_items)
            items.__set(L"", (AhkObjRef*)*i.second, AhkObject, nullptr);
        __set(L"droppedItems", (AhkObjRef*)items, AhkObject, nullptr);

        return items;
    }

    AhkObjRef* get_item(int id) {
        auto i = dropped_items.find(id);
        if (i != dropped_items.end())
            return *i->second;

        return nullptr;
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        player = local_player;
        if (!is_picking || GetTickCount() - last_pickup < 100)
            return;

        std::lock_guard<std::mutex> guard(selected_item_mutex);
        int action_id = local_player->actor->action_id();
        if (action_id & 0x82) {
            if (selected_item && !local_player->actor->target_address) {
                stop_pickup();
            }
        }
    }

    void on_labeled_entity_changed(EntityList& entities) {
        if (!is_picking && !event_enabled)
            return;

        if (GetTickCount() - last_pickup > 3000) {
            if (selected_item) {
                ignored_entities[selected_item->id] = selected_item;
            }
            stop_pickup();
            return;
        }

        if (selected_item && entities.count(selected_item->id)) {
            if (GetTickCount() - last_pickup > 300 && player->actor->action_id() == 0) {
                Point pos = selected_item->label->get_pos();
                poe->mouse_click(pos);
                last_pickup = GetTickCount();
            }
            return;
        }

        std::lock_guard<std::mutex> guard(selected_item_mutex);
        shared_ptr<Entity> nearest_item;
        int min_dist = range;
        for (auto& i : entities) {
            int index = i.second->has_component(entity_types);
            if (index < 0 || ignored_entities.count(i.second->id))
                continue;

            switch (index) {
            case 0:
                {
                    if (ignore_chests || i.second->path.find(L"IzaroChest") != wstring::npos)
                        continue;

                    Chest* chest = i.second->get_component<Chest>();
                    if (chest && !chest->is_locked())
                        break;
                        
                    if (!i.second->has_component("MinimapIcon"))
                        continue;
                }


            case 1:
                {
                    if (!std::regex_search(i.second->path, misc_entity_filter))
                        continue;

                    Targetable* targetable = i.second->get_component<Targetable>();
                    if (!targetable || !targetable->is_targetable())
                        continue;
                }
                break;

            case 2:
                {
                    WorldItem* world_item = i.second->get_component<WorldItem>();
                    if (!world_item || !check_item(world_item->item()))
                        continue;

                    if (event_enabled && dropped_items.find(i.second->id) == dropped_items.end()) {
                        Item* item = new Item(world_item->item());
                        dropped_items[i.second->id] = shared_ptr<Item>(item);
                        PostThreadMessage(thread_id, WM_NEW_ITEM, (WPARAM)item->name().c_str(),
                                          (LPARAM)i.second->id);
                    }
                }
                break;
            }

            int dist = player->dist(*i.second);
            if (dist < min_dist) {
                nearest_item = i.second;
                min_dist = dist;
            }
        }

        if (!nearest_item) {
            if (selected_item) {
                if (selected_item->has_component("Chest"))
                    stop_pickup();
                selected_item.reset();
            } 
            return;
        }

        Point pos = nearest_item->label->get_pos();
        if (PtInRect(&bounds, {pos.x, pos.y})) {
            poe->mouse_click(pos);
            selected_item = nearest_item;
            last_pickup = GetTickCount();

            if (selected_item->path.find(L"AreaTransition") != wstring::npos)
                stop_pickup();
        } else {
            ignored_entities[nearest_item->id] = nearest_item;
            selected_item.reset();
        }
    }
};
