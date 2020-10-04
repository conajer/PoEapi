/*
* AutoOpen.cpp, 9/9/2020 3:08 PM
*/

class AutoOpen : public PoEPlugin {
public:

    LocalPlayer* player;
    std::vector<string> entity_types = {
        "Chest",
        "MinimapIcon",          // DelveMineralVein, shrine etc.
        "TriggerableBlockage",  // Door or switch
        "Transitionable",       // Switch, lever, standing stone, lodestone etc.
    };
    std::wregex entity_names, ignored_chests;
    int total_opened;

    AutoOpen() : PoEPlugin(L"AutoOpen", "0.1"), player(nullptr),
        entity_names(L"Standing Stone|Lodestone|DelveMineralVein|Shrine|CraftingUnlock"),
        ignored_chests(L"Barrel|Basket|Bloom|Boulder|Cairn|Crate|Pot|Urn|Vase")
    {
        total_opened = 0;
    }

    void try_open(Entity* entity) {
        Point pos, old_pos;
        int x, y, is_pressed;

        GetCursorPos ((POINT*)&old_pos);
        is_pressed = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
        pos = poe->get_pos(entity);
        log(L"[%03d] Open '%S' at %d, %d", ++total_opened, entity->name().c_str(), pos.x, pos.y);

        poe->mouse_click(pos.x, pos.y);
        pos = poe->get_pos(entity);
        poe->mouse_click_and_return(pos, old_pos, player->is_moving(), is_pressed);
        Sleep(500);
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        player = local_player;
    }

    void on_entity_changed(EntityList& entities, EntityList& removed, EntityList& add) {
        if (!player)
            return;
            
        for (auto& i : entities) {
            if (force_reset) {
                force_reset = false;
                return;
            }

            int dist = player->dist(*i.second);
            if (dist > 15)
                continue;

            int index = i.second->has_component(entity_types);
            switch (index) {
            case 0: { // Chest
                if (!std::regex_search(i.second->name(), ignored_chests)) {
                    Chest* chest = i.second->get_component<Chest>();
                    if (!chest->is_opened() && !chest->is_locked())
                        try_open(i.second.get());
                }
                break;
            }

            case 1: { // MinimapIcon
                Targetable* targetable = i.second->get_component<Targetable>();
                MinimapIcon* minimap_icon = i.second->get_component<MinimapIcon>();
                if (targetable && targetable->is_targetable())
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
