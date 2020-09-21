/*
* AutoFlask.cpp, 9/21/2020 10:14 AM
*/

class AutoFlask : public PoEPlugin {
public:

    int flask_slot_id = 12;
    addrtype flask_address[10];

    AutoFlask() : PoEPlugin("AutoFlask", "0.1") {
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        ServerData* server_data = in_game_state->server_data();
        auto inventory_slots = server_data->inventory_slots;
        
        InventorySlot* flask_slot = inventory_slots[flask_slot_id].get();
        if (!flask_slot)
            return;

        auto& flasks = flask_slot->get_items();
        for (int index = 1; index < 10; index += 2) {
            auto i = flasks.find(index);
            if (i != flasks.end()) {
                Item& item = i->second.get_item();
                if (flask_address[index] != item.address)
                    PostThreadMessage(thread_id,
                                      WM_FLASK_CHANGED,
                                      (WPARAM)index,
                                      (LPARAM)item.obj_ref);
                flask_address[index] = item.address;
            } else {
                if (flask_address[index])
                    PostThreadMessage(thread_id,
                                      WM_FLASK_CHANGED,
                                      (WPARAM)index,
                                      (LPARAM)0);
                flask_address[index] = 0;
            }
        }
    }
};
