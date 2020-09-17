/*
* AutoOpen.cpp, 9/9/2020 3:08 PM
*/

class AutoOpen : public PoEPlugin {
public:

    LocalPlayer* player;
    Actor* actor;
    std::vector<string> entity_types = {
        "Chest",
        "MinimapIcon",          // DelveMineralVein, shrine etc.
        "TriggerableBlockage",  // Door or switch
        "Transitionable",       // Switch, lever, standing stone, lodestone etc.
    };
    wstring entity_names = L"Standing Stone, Lodestone, DelveMineralVein, Shrine";
    wstring ignored_chests = L"Barrel";
    int total_opened;

    AutoOpen() : PoEPlugin("AutoOpen", "0.1"), player(nullptr) {
        total_opened = 0;
    }

    void try_open(Entity* entity) {
        POINT old_pos;
        int x, y, is_pressed;

        poe->get_pos(entity, x, y);
        GetCursorPos (&old_pos);
        is_pressed = GetAsyncKeyState(VK_LBUTTON) & 0x8000;
        log(L"[%03d] Open '%S' at %d, %d", ++total_opened, entity->name().c_str(), x, y);

        poe->mouse_click(x, y);
        poe->get_pos(entity, x, y);
        poe->mouse_click_and_return(x, y, old_pos, actor->is_moving(), is_pressed);
        Sleep(500);
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        player = local_player;
        actor = player->get_component<Actor>();
	
    }

    void on_labeled_entity_changed(EntityList& entities) {
        for (auto& i : entities) {
            int dist = player->dist(*i.second);
            if (dist > 15)
                continue;

            int index = i.second->has_component(entity_types);
            switch (index) {
            case 0: { // Chest
                if (ignored_chests.find(i.second->name()) == string::npos) {
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
                    if (entity_names.find(minimap_icon->name()) != string::npos)
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
                if (entity_names.find(i.second->name()) != string::npos)
                    try_open(i.second.get());
                break;

            default:
                ;
            }
        };
    }
};
