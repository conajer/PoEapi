/*
* DelveChest.cpp, 9/9/2020 11:26 PM
*/

class DelveChest : public PoEPlugin {
public:

    std::vector<string> entity_types = {
        "Chest",                // DelveChests
        "MinimapIcon",          // DelveWall
    };

    LocalPlayer* player;
    std::vector<shared_ptr<Entity>> chests;
    bool in_azurite_mine = false;
    bool no_chest_found = true;

    DelveChest() : PoEPlugin(L"DelveChest", "0.1"), player(nullptr) {
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        player = local_player;
	}

    void on_area_changed(AreaTemplate* world_area, int hash_code) {
        chests.clear();
        in_azurite_mine = world_area->name() == L"Azurite Mine";
    }

    void on_entity_changed(EntityList& entities, EntityList& removed, EntityList& added) {
        if (!in_azurite_mine || !player)
            return;

        chests.clear();
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                return;
            }

            wstring& path = i.second->path;
            if (path.find(L"Delve") == string::npos)
                continue;

            Targetable* targetable = i.second->get_component<Targetable>();
            if (!targetable || !targetable->is_targetable())
                continue;

            int index = i.second->has_component(entity_types);
            switch (index) {
            case 0: // Chest
                if (path.find(L"DelveChests") != string::npos)
                    chests.push_back(i.second);
                break;

            case 1: { // MinimapIcon
                if (path.find(L"DelveWall") != string::npos)
                    chests.push_back(i.second);
                break;
            }

            default:
                ;
            }
        }

        if (chests.size() > 0) {
            for (auto& i : chests) {
                Point pos = poe->get_pos(i.get());
                PostThreadMessage(thread_id,
                    WM_DELVE_CHEST,
                    (WPARAM)&i->path[i->path.rfind(L'/') + 1],
                    (LPARAM)((pos.x & 0xffff) << 16) | (pos.y & 0xffff));
            }
            PostThreadMessage(thread_id, WM_DELVE_CHEST, (WPARAM)0, (LPARAM)0);
            no_chest_found = false;
        } else if (!no_chest_found) {
            PostThreadMessage(thread_id, WM_DELVE_CHEST, (WPARAM)0, (LPARAM)0);
            no_chest_found = true;
        }
    }
};
