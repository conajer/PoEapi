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
    shared_ptr<Entity> selected_item;
    RECT bounds;
    unsigned int range = 50, last_pickup = 0;
    int try_again;
    bool is_picking = false;

    std::wregex generic_item_filter;
    std::wregex rare_item_filter;

    AutoPickup() : PoEPlugin(L"AutoPickup", "0.2") {
        add_property(L"range", &range, AhkInt);
        add_method(L"setGenericItemFilter", this,(MethodType)&AutoPickup::set_generic_item_filter, AhkVoid, ParamList{AhkWString});
        add_method(L"setRareItemFilter", this, (MethodType)&AutoPickup::set_rare_item_filter, AhkVoid, ParamList{AhkWString});

        set_generic_item_filter(L"Incubator|Scarab$|Quicksilver|Diamond|Basalt|Quartz");
        set_rare_item_filter(L"Jewels|Amulet|Rings|Belts");
    }

    void set_generic_item_filter(const wchar_t* regex_string) {
        generic_item_filter.assign(regex_string);
    }

    void set_rare_item_filter(const wchar_t* regex_string) {
        rare_item_filter.assign(regex_string);
    }

    void begin_pickup() {
        stop_pickup();
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
        ignored_entities.clear();
        selected_item.reset();
        is_picking = false;
    }

    bool check_item(addrtype address) {
        Item item(address);

        if (item.has_component(item_types) >= 0)
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

    void on_labeled_entity_changed(EntityList& entities) {
        if (!is_picking || !player)
            return;

        shared_ptr<Entity> nearest_item;
        int min_dist = range;
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                return;
            }

            int index = i.second->has_component(entity_types);
            if (index < 0)
                continue;

            if (ignored_entities.find(i.second->id) != ignored_entities.end())
                continue;

            switch (index) {
            case 0:
                {
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
                }
                break;
            }

            int dist = player->dist(*i.second);
            if (dist < min_dist) {
                nearest_item = i.second;
                min_dist = dist;
            }
        }

        if (GetTickCount() - last_pickup > 3000) {
            stop_pickup();
            return;
        }

        if (!nearest_item)
            return;

        if (nearest_item == selected_item) {
            addrtype target_address = player->actor->target_address;
            if (target_address == selected_item->address || GetTickCount() - last_pickup < 500)
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
        Point& pos = selected_item->get_pos();
        if (!PtInRect(&bounds, {pos.x, pos.y}))
            return;

        PostThreadMessage(thread_id, WM_PICKUP, (WPARAM)pos.x, (LPARAM)pos.y);
        log(L"%x: %S, %d, %d\n",
            selected_item->id,
            selected_item->name().c_str(),
            pos.x, pos.y);
        last_pickup = GetTickCount();
    }
};
