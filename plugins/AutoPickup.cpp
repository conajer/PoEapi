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

    LocalPlayer* player;
    std::map<int, shared_ptr<Entity>> ignored_entities;
    shared_ptr<Entity> selected_item;
    RECT bound;
    int range, last_pickup;
    int try_again;
    bool is_picking = false;

    std::wregex generic_item_filter;
    std::wregex rare_item_filter;

    AutoPickup() : PoEPlugin(L"AutoPickup", "0.1"), player(nullptr), range(50) {
        generic_item_filter.assign(L"Incubator|Scarab$|Quicksilver|Diamond|Basalt|Quartz");
        rare_item_filter.assign(L"Jewels|Amulet|Rings|Belts");
    }

    void begin_pickup() {
        stop_pickup();
        try_again = 0;
        last_pickup = GetTickCount();
        is_picking = true;

        GetClientRect(poe->hwnd, &bound);
        bound.left += 200;
        bound.top += 150;
        bound.right -= 200;
        bound.bottom -= 150;
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

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        player = local_player;
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

        if (!nearest_item || GetTickCount() - last_pickup > 3000) {
            stop_pickup();
            return;
        }

        if (nearest_item == selected_item) {
            int action_id = player->actor->action_id();
            if (action_id > 0) {
                if (action_id == 2 && player->actor->target_address == selected_item->address) {
                    ignored_entities[selected_item->id] = selected_item;
                }
                return;
            }
                
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
        if (!PtInRect(&bound, {pos.x, pos.y}))
            return;

        PostThreadMessage(thread_id, WM_PICKUP, (WPARAM)pos.x, (LPARAM)pos.y);
        log(L"%llx: %S, %d, %d\n",
            selected_item->address,
            selected_item->name().c_str(),
            pos.x, pos.y);
        last_pickup = GetTickCount();
    }
};
