/*
* AutoFlask.cpp, 9/21/2020 10:14 AM
*/

class AutoFlask : public PoEPlugin {
public:

    int flask_slot_id = 12;
    addrtype flasks[5], saved_flasks[5];

    AutoFlask() : PoEPlugin(L"AutoFlask", "0.2") {
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        ServerData* server_data = in_game_state->server_data();
        auto inventory_slots = server_data->inventory_slots;
        InventorySlot* flask_slot = inventory_slots[flask_slot_id].get();
        
        if (flask_slot) {
            addrtype addr = flask_slot->read<addrtype>("cells");
            flask_slot->PoEMemory::read<addrtype>(addr, flasks, 5);
            if (memcmp(flasks, saved_flasks, sizeof(flasks))) {
                PostThreadMessage(thread_id, WM_FLASK_CHANGED, (WPARAM)0, (LPARAM)0);
                memcpy(saved_flasks, flasks, sizeof(flasks));
            }
        }
    }
};
