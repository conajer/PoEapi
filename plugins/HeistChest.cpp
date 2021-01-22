/*
* HeistChest.cpp, 9/26/2020 2:41 PM
*/

class HeistChest : public PoEPlugin {
public:

    std::vector<shared_ptr<Entity>> chests;
    bool no_chest_found = true;

    HeistChest() : PoEPlugin(L"HeistChest", "0.1") {
    }

    void on_labeled_entity_changed(EntityList& entities) {
        chests.clear();
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                return;
            }

            wstring& path = i.second->path;
            if (path.find(L"HeistChest") == string::npos)
                continue;
            
            if (player->dist(*i.second) > 200)
                continue;

            chests.push_back(i.second);
        }

        if (chests.size() > 0) {
            for (auto& i : chests) {
                Rect r = i->label->get_rect();
                PostThreadMessage(thread_id,
                    WM_HEIST_CHEST,
                    (WPARAM)&i->path[i->path.rfind(L'/') + 1],
                    (LPARAM)(((__int64)r.w << 48) | ((__int64)r.h << 32) | (r.x & 0xffff) << 16) | (r.y & 0xffff));
            }
            PostThreadMessage(thread_id, WM_HEIST_CHEST, (WPARAM)0, (LPARAM)0);
            no_chest_found = false;
        } else if (!no_chest_found) {
            PostThreadMessage(thread_id, WM_HEIST_CHEST, (WPARAM)0, (LPARAM)0);
            no_chest_found = true;
        }
    }
};
