/*
* PoEPlugin.cpp, 8/21/2020 9:08 PM
*/

class PoEPlugin {
public:

    const char* name;
    const char* version;
    PoE* poe;
    DWORD thread_id;

    PoEPlugin(const char* name, const char* version_string = "0.1")
        : name(name), version(version_string)
    {
    }

    virtual void on_load(PoE& poe, int ownere_thread_id) {
        this->poe = &poe;
        this->thread_id = ownere_thread_id;
    }

    virtual void on_player(LocalPlayer* player, InGameState* in_game_state) {
    }

    virtual void on_area_changed(AreaTemplate* world_area) {
    }

    virtual void on_entity_changed(EntityList& all, EntityList& removed, EntityList& added) {
    }
};
