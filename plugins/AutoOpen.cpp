/*
* AutoOpen.cpp, 9/9/2020 3:08 PM
*/

class AutoOpen : public PoEPlugin {
public:

    std::vector<string> entity_types = {
        "Chest",
        "MinimapIcon",          // DelveMineralVein, shrine etc.
        "TriggerableBlockage",  // Door or switch
        "Transitionable",       // Switch, lever, standing stone, lodestone etc.
    };
    std::unordered_set<int> ignored_entities;
    std::wregex entity_names, ignored_chests;
    wstring default_ignored_chests;
    RECT bounds;
    int range = 15;
    int total_opened;
    bool chest_enabled = true;
    bool delve_chest_only = true;
    bool door_enabled = true;

    AutoOpen() : PoEPlugin(L"AutoOpen", "0.7"),
        entity_names(L"Standing Stone|Lodestone|DelveMineralVein|Shrine|CraftingUnlock"),
        default_ignored_chests(L"Barrel|Basket|Bloom|Bone (Chest|Pile)|Boulder|Cairn|Crate|Pot|Urn|Vase|Izaro")
    {
        total_opened = 0;
        add_property(L"range", &range, AhkInt);
        add_property(L"chest", &chest_enabled, AhkBool);
        add_property(L"delveChestOnly", &delve_chest_only, AhkBool);
        add_property(L"door", &door_enabled, AhkBool);
        add_method(L"setIgnoredChests", this, (MethodType)&AutoOpen::set_ignored_chests, AhkVoid, ParamList{AhkWString});
        set_ignored_chests();
    }

    void set_ignored_chests(const wchar_t* regex_string = nullptr) {
        if (regex_string)
            ignored_chests.assign(default_ignored_chests + L"|" + regex_string);
        else
            ignored_chests.assign(default_ignored_chests);
    }

    void reset() {
        PoEPlugin::reset();

        GetClientRect(poe->window, &bounds);
        bounds.left = bounds.right / 2 - 200;
        bounds.top = bounds.bottom / 2 - 300;
        bounds.right = bounds.left + 400;
        bounds.bottom = bounds.top + 400;
    }

    void try_open(Entity* entity) {
        Vector3 pos = entity->pos;
        poe->in_game_state->transform(pos);

        Point old_pos;
        GetCursorPos((POINT*)&old_pos);
        bool is_button_pressed = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
        bool is_moving = player->is_moving();

        log(L"Tring to open '%S' at (%d, %d)", entity->name().c_str(), old_pos.x, old_pos.y);
        if (is_button_pressed) {
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            Sleep(10);
        }
        poe->mouse_click({(int)pos.x, (int)pos.y});
        SetCursorPos (old_pos.x, old_pos.y);
        Sleep(30);
        if (is_button_pressed) {
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        } else if (is_moving) {
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        }
        Sleep(100);
    }

    void on_area_changed(AreaTemplate* world_area, int hash_code, LocalPlayer* player) {
        ignored_entities.clear();
    }

    void on_entity_changed(EntityList& entities, EntityList& removed, EntityList& add) {
        for (auto& i : entities) {
            if (ignored_entities.find(i.second->id) != ignored_entities.end())
                continue;

            int index = i.second->has_component(entity_types);
            if (index < 0) {
                ignored_entities.insert(i.second->id);
                return;
            }

            int dist = player->dist(*i.second);
            if (dist > 4 * range)
                continue;

            Targetable* targetable = i.second->get_component<Targetable>();
            if (!targetable || !targetable->is_targetable())
                continue;

            switch (index) {
            case 0: { // Chest
                if (dist <= range && chest_enabled) {
                    if (std::regex_search(i.second->name(), ignored_chests))
                        break;

                    if (delve_chest_only) {
                        if ((i.second->path.find(L"DelveChest") == wstring::npos))
                            break;
                    } else if (i.second->path.find(L"HeistChest") != wstring::npos) {
                        break;
                    }

                    Chest* chest = i.second->get_component<Chest>();
                    if (!chest->is_opened() && !chest->is_locked())
                        try_open(i.second.get());
                }
                break;
            }

            case 1: { // MinimapIcon
                MinimapIcon* minimap_icon = i.second->get_component<MinimapIcon>();
                if (dist <= 2 * range && std::regex_search(minimap_icon->name(), entity_names))
                    try_open(i.second.get());
                break;
            }

            case 2: { // TriggerableBlockage
                if (door_enabled) {
                    TriggerableBlockage* blockage = i.second->get_component<TriggerableBlockage>();
                    if (blockage->is_closed() && i.second->path.find(L"Labyrinth/Objects") == string::npos)
                        try_open(i.second.get());
                }
                break;
            }

            case 3: // Transitionable
                if (dist <= 2 * range && std::regex_search(i.second->name(), entity_names))
                    try_open(i.second.get());
                break;
            }
        };
    }
};
