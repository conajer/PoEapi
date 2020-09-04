/*
* InGameUI.cpp, 8/18/2020 6:46 PM
*/

static std::map<string, int> in_game_ui_offsets {
    {"inventory",       0x518},
    {"stash",           0x520},
    {"highlighted_entities", 0x5a8},
    {"purchase_window", 0x638},
    {"sell_window",     0x640},
    {"trade_window",      0x0},
    {"gem_level_up",    0x8c8},
};

class InGameUI : public RemoteMemoryObject {
public:

    InGameUI(addrtype address)
        : RemoteMemoryObject(address, &in_game_ui_offsets)
    {
    }
};
