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
    std::wregex entity_names, ignored_chests;
    wstring default_ignored_chests;
    RECT bounds;
    int range = 15;
    int total_opened;

    AutoOpen() : PoEPlugin(L"AutoOpen", "0.4"),
        entity_names(L"Standing Stone|Lodestone|DelveMineralVein|Shrine|CraftingUnlock"),
        default_ignored_chests(L"Barrel|Basket|Bloom|Bone (Chest|Pile)|Boulder|Cairn|Crate|Pot|Urn|Vase|Izaro")
    {
        total_opened = 0;
        add_property(L"range", &range, AhkInt);
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

        GetClientRect(poe->hwnd, &bounds);
        bounds.left = bounds.right / 2 - 200;
        bounds.top = bounds.bottom / 2 - 150;
        bounds.right = bounds.left + 400;
        bounds.bottom = bounds.top + 300;
    }

    void try_open(Entity* entity) {
        Point pos, old_pos;
        if (player->actor->target_address == entity->address)
            return;

        pos = poe->get_pos(entity);
        if (!PtInRect(&bounds, {pos.x, pos.y}))
            return;

        GetCursorPos ((POINT*)&old_pos);
        bool is_button_pressed = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
        bool is_moving = player->is_moving();

        if (is_button_pressed)
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

        poe->mouse_click(pos);
        poe->mouse_click(poe->get_pos(entity));

        SetCursorPos (old_pos.x, old_pos.y);
        Sleep(30);
        if (is_button_pressed) {
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        } else if (is_moving) {
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
        }
        Sleep(500);
    }

    void on_entity_changed(EntityList& entities, EntityList& removed, EntityList& add) {
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                return;
            }

            int dist = player->dist(*i.second);
            if (dist > range)
                continue;

            Targetable* targetable = i.second->get_component<Targetable>();
            if (!targetable || !targetable->is_targetable())
                continue;

            int index = i.second->has_component(entity_types);
            switch (index) {
            case 0: { // Chest
                if ((i.second->path.find(L"HeistChest") == wstring::npos)
                    && (i.second->has_component("MinimapIcon")
                        || !std::regex_search(i.second->name(), ignored_chests)))
                {
                    Chest* chest = i.second->get_component<Chest>();
                    if (!chest->is_opened() && !chest->is_locked())
                        try_open(i.second.get());
                }
                break;
            }

            case 1: { // MinimapIcon
                MinimapIcon* minimap_icon = i.second->get_component<MinimapIcon>();
                if (std::regex_search(minimap_icon->name(), entity_names))
                    try_open(i.second.get());
                break;
            }

            case 2: { // TriggerableBlockage
                TriggerableBlockage* blockage = i.second->get_component<TriggerableBlockage>();
                if (blockage->is_closed() && i.second->path.find(L"Labyrinth/Objects") == string::npos)
                    try_open(i.second.get());
                break;
            }

            case 3: // Transitionable
                if (std::regex_search(i.second->name(), entity_names))
                    try_open(i.second.get());
                break;

            default:
                ;
            }
        };
    }
};
