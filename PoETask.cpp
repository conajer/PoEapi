/*
* PoETask.cpp, 8/21/2020 10:07 PM
*/

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "PoE.cpp"
#include "PoEapi.c"
#include "Task.cpp"
#include "PoEPlugin.cpp"
#include "plugins/AutoOpen.cpp"
#include "plugins/DelveChest.cpp"
#include "plugins/PlayerStatus.cpp"
#include "plugins/KillCounter.cpp"

extern "C" BOOL SetProcessDPIAware();


class PoETask : public PoE, public Task {
public:
    
    EntitySet entities;
    int area_hash;

    std::vector<shared_ptr<PoEPlugin>> plugins;
    std::wregex ignored_exp;

    PoETask() : Task(), ignored_exp(L"Doodad|Effect|WorldItem") {
    }

    void add_plugin(PoEPlugin* plugin) {
        plugins.push_back(shared_ptr<PoEPlugin>(plugin));
        plugin->on_load(*this, owner_thread_id);

        log(L"added plugin %s %s", plugin->name, plugin->version);
    }

    void check_player() {
        if (!is_in_game())
            return;

        InGameData* in_game_data = in_game_state->in_game_data();
        LocalPlayer* local_player = in_game_data->local_player();
        if (local_player) {
            for (auto i : plugins)
                i->on_player(local_player, in_game_state);
        }

        if (in_game_data->area_hash() != area_hash) {
            area_hash = in_game_data->area_hash();
            AreaTemplate* world_area = in_game_data->world_area();
            if (!world_area->name().empty()) {
                for (auto i : plugins)
                    i->on_area_changed(in_game_data->world_area(), area_hash);
            }
        }
    }

    void check_entities() {
        if (!is_in_game() || GetForegroundWindow() != hwnd)
            return;

        InGameData* in_game_data = in_game_state->in_game_data();
        in_game_data->get_all_entities(entities, ignored_exp);

        for (auto i : plugins)
            i->on_entity_changed(entities.all, entities.removed, entities.added);
    }

    void run() {
        /* yield the execution to make sure the CreateThread() return,
           otherwise log() function may fail. */
        Sleep(300);

        log(L"PoEapi v%d.%d.%d (supported Path of Exile %s).",
            major_version, minor_version, patch_level, supported_PoE_version);
        
        /* add plugins */
        add_plugin(new AutoOpen());
        add_plugin(new DelveChest());
        add_plugin(new PlayerStatus());
        add_plugin(new KillCounter());

        /* create jobs */
        start_job(77, [=] {this->check_player();});
        start_job(97, [=] {this->check_entities();});

        log(L"PoE task started (%d jobs).",  jobs.size());
        join(); /* wait for the jobs finish */
    }
};

/* Global PoE task object. */
PoETask ptask;

DLLEXPORT void poeapi_toggle_maphack() {
    int flag = ptask.toggle_maphack();
    ptask.log(L"Maphack %S.", flag == -1 ? L"failed" : (flag ? L"enabled" : L"disabled"));
}

extern "C" WINAPI
BOOL DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        SetProcessDPIAware();
        /* Start the main task of PoEapi */
        return ptask.start();

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        ptask.stop();
        break;
    }

    return true;
}

int main(int argc, char* argv[]) {
    SetProcessDPIAware();
    /* Start the main task of PoEapi */
    ptask.start();
    ptask.join(INFINITE);
}
