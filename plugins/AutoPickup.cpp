/*
* AutoPickup.cpp, 9/18/2020 11:11 PM
*/

class AutoPickup : public PoEPlugin {
public:

    std::vector<string> entity_types = {
        "Chest",
        "WorldItem",
    };

    std::vector<string> item_types = {
        "Stack",                // Currency/DivinationCard items
        "Map",                  // Maps
        "Quest",                // Quest items
        "HeistObjective",       // Heist objective item
    };

    std::map<int, shared_ptr<Entity>> ignored_entities;
    std::map<int, shared_ptr<Item>> dropped_items;
    shared_ptr<Entity> selected_item;
    RECT bounds;
    unsigned int range = 50, last_pickup = 0;
    int try_again;
    bool ignore_chests = false;
    bool is_picking = false;
    bool event_enabled = false;

    std::wregex generic_item_filter;
    std::wregex rare_item_filter;

    AutoPickup() : PoEPlugin(L"AutoPickup", "0.9") {
        add_property(L"range", &range, AhkInt);
        add_property(L"ignoreChests", &ignore_chests, AhkBool);
        add_property(L"eventEnabled", &event_enabled, AhkBool);

        add_method(L"setGenericItemFilter", this,(MethodType)&AutoPickup::set_generic_item_filter, AhkVoid, ParamList{AhkWString});
        add_method(L"setRareItemFilter", this, (MethodType)&AutoPickup::set_rare_item_filter, AhkVoid, ParamList{AhkWString});
        add_method(L"beginPickup", this, (MethodType)&AutoPickup::begin_pickup);
        add_method(L"stopPickup", this, (MethodType)&AutoPickup::stop_pickup);
        add_method(L"getDroppedItems", this, (MethodType)&AutoPickup::get_dropped_items, AhkObject);
        add_method(L"getItem", this, (MethodType)&AutoPickup::get_item, AhkObject, ParamList{AhkInt});

        set_generic_item_filter(L"Incubator|Scarab$|Quicksilver|Diamond|Basalt|Quartz");
        set_rare_item_filter(L"Jewels|Amulet|Rings|Belts");
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

    void begin_pickup() {
        if (!player || is_picking)
            return;

        ignored_entities.clear();
        selected_item.reset();
        is_picking = true;
        last_pickup = GetTickCount();
        try_again = 0;

        GetClientRect(poe->hwnd, &bounds);
        bounds.left += 200;
        bounds.top += 150;
        bounds.right -= 200;
        bounds.bottom -= 150;
    }

    void stop_pickup() {
        is_picking = false;
    }

    bool check_item(addrtype address) {
        Item item(address);

        if (item.get_influence_type() || item.has_component(item_types) >= 0)
            return true;

        if (std::regex_search(item.base_name(), generic_item_filter))
            return true;

        if (item.has_component("SkillGem"))
            return (item.get_quality() >= 5 || item.get_level() > 12);

        int rarity = item.get_rarity();
        if (rarity > 1) {
            if (rarity == 3)
                return true;

            int ilvl = item.get_item_level();
            if (!item.is_identified()
                && ((ilvl >= 60 && ilvl < 75)
                    || std::regex_search(item.path, rare_item_filter)))
                return true;
        }

        if (item.get_sockets() == 6 || item.is_rgb())
            return true;

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

    void on_labeled_entity_changed(EntityList& entities) {
        shared_ptr<Entity> nearest_item;
        if (!is_picking && !event_enabled)
            return;

        int min_dist = range;
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                return;
            }

            int index = i.second->has_component(entity_types);
            if (index < 0)
                continue;

            if (is_picking && ignored_entities.find(i.second->id) != ignored_entities.end())
                continue;

            switch (index) {
            case 0:
                {
                    if (ignore_chests || i.second->path.find(L"IzaroChest") != wstring::npos)
                        continue;

                    Chest* chest = i.second->get_component<Chest>();
                    if (!chest || chest->is_locked())
                        continue;
                }
                break;

            case 1:
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

        if (!is_picking)
            return;

        if (GetTickCount() - last_pickup > 3000) {
            stop_pickup();
            return;
        }

        if (!nearest_item)
            return;

        if (nearest_item == selected_item) {
            addrtype target_address = player->actor->target_address;
            if (target_address == selected_item->address || GetTickCount() - last_pickup < 200)
                return;

            if (++try_again > 3) {
                ignored_entities[selected_item->id] = selected_item;
                try_again = 0;
                return;
            }
        } else {
            try_again = 0;
        }

        selected_item = nearest_item;
        Point pos = selected_item->label->get_pos();
        if (PtInRect(&bounds, {pos.x, pos.y})) {
            poe->mouse_click(pos);
            last_pickup = GetTickCount();
        }
    }
};
