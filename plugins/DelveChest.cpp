/*
* DelveChest.cpp, 9/9/2020 11:26 PM
*/

class DelveChest : public PoEPlugin {
public:

    LocalPlayer* player;
    std::vector<string> entity_types = {
        "Chest",                // DelveChests
        "MinimapIcon",          // DelveWall
    };
    std::unordered_map<int, shared_ptr<Entity>> chests;
    int enabled;

    DelveChest() : PoEPlugin("DelveChest", "0.1"), player(nullptr) {
        enabled = false;
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        player = local_player;
	}

    void on_area_changed(AreaTemplate* world_area, int hash_code) {
        chests.clear();
        enabled = world_area->name() == L"Azurite Mine";
    }

    void on_entity_changed(EntityList& entities, EntityList& removed, EntityList& added) {
        if (!enabled)
            return;

        for (auto& i : added) {
            int index = i.second->has_component(entity_types);
            Targetable* targetable = i.second->get_component<Targetable>();
            if (!targetable || !targetable->is_targetable())
                continue;

            switch (index) {
            case 0: // Chest
                if (i.second->path.find(L"DelveChests") != string::npos)
                    chests.insert(i);
                break;

            case 1: { // MinimapIcon
                if (i.second->path.find(L"DelveWall") != string::npos)
                    chests.insert(i);
                break;
            }

            default:
                ;
            }
        }

        for (auto& i : removed) {
            if (chests.erase(i.first) > 0) {
                PostThreadMessage(thread_id, WM_DELVE_CHEST,
                (WPARAM)i.second->path.c_str(),
                (LPARAM)((__int64)i.second->id << 32));
            }
        }

        for (auto& i : chests) {
            int x, y;
            Entity* entity = i.second.get();
            poe->get_pos(i.second.get(), x, y);
            PostThreadMessage(thread_id,
                WM_DELVE_CHEST,
                (WPARAM)i.second->path.c_str(),
                (LPARAM)((__int64)i.second->id << 32) | ((x & 0xffff) << 16) | (y & 0xffff));
        }
    }
};
