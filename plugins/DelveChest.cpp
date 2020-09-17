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
    int enabled;
    std::vector<shared_ptr<Entity>> chests;

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

    void on_labeled_entity_changed(EntityList& entities) {
        if (!enabled)
            return;

        for (auto& i : entities) {
            int index = i.second->has_component(entity_types);
            Targetable* targetable = i.second->get_component<Targetable>();
            if (!targetable || !targetable->is_targetable())
                continue;

            switch (index) {
            case 0: // Chest
                if (i.second->path.find(L"DelveChests") != string::npos)
                    chests.push_back(i.second);
                break;

            case 1: { // MinimapIcon
                if (i.second->path.find(L"DelveWall") != string::npos)
                    chests.push_back(i.second);
                break;
            }

            default:
                ;
            }
        }

        for (auto& i : chests) {
            int x, y;
            poe->get_pos(i.get(), x, y);
            PostThreadMessage(thread_id,
                WM_DELVE_CHEST,
                (WPARAM)&i->path[i->path.rfind(L'/') + 1],
                (LPARAM)((__int64)i->id << 32) | ((x & 0xffff) << 16) | (y & 0xffff));
        }
    }
};
