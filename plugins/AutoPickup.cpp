/*
* AutoPickup.cpp, 9/18/2020 11:11 PM
*/

class AutoPickup : public PoEPlugin {
public:

    std::vector<string> entity_types = {
        "Chest",
        "WorldItem",
    };

    LocalPlayer* player;
    Entity* selected_item;
    std::vector<shared_ptr<Entity>> selected_items;
    int range, last_pickup;
    bool enabled;

    std::wregex generic_item_filter;
    std::wregex rare_item_filter;

    AutoPickup() : PoEPlugin("AutoPickup", "0.1"), player(nullptr) {
        range = 50;
        enabled = false;
        generic_item_filter.assign(L"Incubator|Scarab$|Quicksilver|Diamond|Basalt|Quartz");
        rare_item_filter.assign(L"Jewels|Amulet|Rings|Belts");
    }

    bool check_item(addrtype address) {
        Item item(address);

        if (item.has_component("Stack") || item.has_component("Map"))
            return true;

        if (std::regex_search(item.base_name(), generic_item_filter))
            return true;

        if (item.has_component("SkillGem"))
            return (item.get_quality() > 5 || item.get_level() > 12);

        if (item.get_rarity() > 1) {
            if (item.get_rarity() == 3)
                return true;

            if (!item.is_identified() && std::regex_search(item.path, rare_item_filter))
                return true;
        }

        if (item.get_sockets() == 6 || item.is_rgb())
            return true;

        return false;
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        player = local_player;
    }

    void on_area_changed(AreaTemplate* world_area, int hash_code) {
        selected_items.clear();
    }

    void on_labeled_entity_changed(EntityList& entities) {
        if (!enabled)
            return;

        int min_dist = range;
        Entity* nearest_item = nullptr;
        for (auto& i : entities) {
            int index = i.second->has_component(entity_types);
            if (index < 0)
                continue;

            switch (index)
            {
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
            
            default:
                ;
            }

            int dist = player->dist(*i.second);
            if (dist < min_dist) {
                nearest_item = i.second.get();
                min_dist = dist;
            }
        }

        if (!nearest_item) {
            enabled = false;
            selected_item = nullptr;
            return;
        }
        
        if (nearest_item == selected_item) {
            if (GetTickCount() - last_pickup > 4000) {
                enabled = false;
                selected_item = nullptr;
            }
            return;
        }

        selected_item = nearest_item;
        Point& pos = selected_item->get_pos();
        PostThreadMessage(thread_id, WM_PICKUP, (WPARAM)pos.x, (LPARAM)pos.y);
        last_pickup = GetTickCount();
    }
};
