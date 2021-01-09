/*
* PlayerStatus.cpp, 8/31/2020 11:37 AM
*/

class PlayerStatus : public PoEPlugin {
public:

    wstring player_name;
    wstring league;
    int life, mana, energy_shield;
    int last_action_id;
    bool is_dead;
    unsigned int threshold_percentage = 20;
    bool min_level = 90;

    PlayerStatus() : PoEPlugin(L"PlayerStatus", "0.4") {
        life = mana = 0;
        is_dead = false;

        add_property(L"autoQuitThresholdPercentage", &threshold_percentage, AhkInt);
        add_property(L"autoQuitMinLevel", &min_level, AhkInt);
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        if (!player || *player != *local_player) {
            player = local_player;

            if (player_name != player->name()) {
                player_name = player->name();
                PostThreadMessage(thread_id, WM_PLAYER_CHANGED, (WPARAM)player_name.c_str(), (LPARAM)0);
            }
        }

        /* life, mana and energy shield */
        int maximum, reserved;

        int current_life = local_player->life->life();
        if (current_life != this->life || current_life < (maximum - reserved)) {
            this->life = local_player->life->life(&maximum, &reserved);
            PostThreadMessage(thread_id,
                              WM_PLAYER_LIFE,
                              (WPARAM)this->life,
                              (LPARAM)(maximum | (reserved << 16)));

            if (is_dead) {
                is_dead = (current_life == 0);
            }
        }

        if (local_player->level() >= min_level && current_life * 100 / maximum < threshold_percentage)
            poe->logout();

        int current_mana = local_player->life->mana();
        if (current_mana != this->mana) {
            this->mana = local_player->life->mana(&maximum, &reserved);
            PostThreadMessage(thread_id,
                              WM_PLAYER_MANA,
                              (WPARAM)this->mana,
                              (LPARAM)(maximum | (reserved << 16)));
        }

        int current_es = local_player->life->energy_shield(&maximum);
        if (current_es != this->energy_shield) {
            PostThreadMessage(thread_id,
                              WM_PLAYER_ENERGY_SHIELD,
                              (WPARAM)current_es,
                              (LPARAM)maximum);
            this->energy_shield = current_es;
        }

        if (life == 1 && local_player->level() >= min_level && current_es * 100 / maximum < threshold_percentage)
            poe->logout();

        /* action */
        Actor* actor = player->get_component<Actor>();
        int action_id = actor->action_id();
        if (action_id & ACTION_MOVING)
            PostThreadMessage(thread_id, WM_PLAYER_MOVE, 0, 0);
        else if (action_id & ACTION_DEAD) {
            if (!is_dead)
                PostThreadMessage(thread_id, WM_PLAYER_DIED, 0, 0);
            is_dead = true;
        }
        
        if (actor->skill) {
            PostThreadMessage(thread_id,
                              WM_PLAYER_USE_SKILL,
                              (WPARAM)actor->skill->name.c_str(),
                              (LPARAM)actor->target_address);
        }
    }

    void on_area_changed(AreaTemplate* world_area, int hash_code, LocalPlayer* player) {
        bool is_town = world_area->is_town();
        PostThreadMessage(thread_id,
                          WM_AREA_CHANGED,
                          (WPARAM)world_area->name().c_str(),
                          (LPARAM)world_area->level() | (is_town ? 0x100 : 0));

        log(L"You have entered %S", world_area->name().c_str());
    }
};
