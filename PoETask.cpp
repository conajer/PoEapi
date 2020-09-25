/*
* PoETask.cpp, 8/21/2020 10:07 PM
*/

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "PoE.cpp"
#include "PoEapi.c"
#include "Task.cpp"
#include "PoEPlugin.cpp"
#include "plugins/AutoFlask.cpp"
#include "plugins/AutoOpen.cpp"
#include "plugins/AutoPickup.cpp"
#include "plugins/DelveChest.cpp"
#include "plugins/PlayerStatus.cpp"
#include "plugins/KillCounter.cpp"

extern "C" BOOL SetProcessDPIAware();

class PoETask : public PoE, public Task {
public:
    
    EntitySet entities;
    EntityList labeled_entities, labeled_removed;
    int area_hash;
    wstring league;

    std::vector<shared_ptr<PoEPlugin>> plugins;
    LocalPlayer *local_player;
    std::wregex ignored_entity_exp;
    AutoPickup* auto_pickup;
    bool is_attached = false;

    PoETask() : Task(L"PoETask"), auto_pickup(new AutoPickup()),
        ignored_entity_exp(L"Doodad|Effect|WorldItem")
    {
        add_method(L"start", (Task*)this, (MethodType)&Task::start, AhkInt);
        add_method(L"stop", (Task*)this, (MethodType)&Task::stop);
        add_method(L"getLatency", this, (MethodType)&PoETask::get_latency);
        add_method(L"getNearestEntity", this, (MethodType)&PoETask::get_nearest_entity,
                   AhkObject, std::vector<AhkType>{AhkWString});
        add_method(L"getPartyStatus", this, (MethodType)&PoETask::get_party_status);
        add_method(L"getXP", this, (MethodType)&PoETask::get_xp, AhkUInt);
        add_method(L"getInventory", this, (MethodType)&PoETask::get_inventory, AhkObject);
        add_method(L"getInventorySlots", this, (MethodType)&PoETask::get_inventory_slots, AhkObject);
        add_method(L"getStash", this, (MethodType)&PoETask::get_stash, AhkObject);
        add_method(L"getStashTabs", this, (MethodType)&PoETask::get_stash_tabs, AhkObject);
        add_method(L"toggleMaphack", this, (MethodType)&PoETask::toggle_maphack);
        add_method(L"hasBuff", this, (MethodType)&PoETask::has_buff,
                   AhkInt, std::vector<AhkType>{AhkWString});
        add_method(L"beginPickup", this, (MethodType)&PoETask::begin_pickup);
        add_method(L"stopPickup", this, (MethodType)&PoETask::stop_pickup);
        add_method(L"setPickupRange", this, (MethodType)&PoETask::set_pickup_range,
                   AhkInt, std::vector<AhkType>{AhkInt});
        add_method(L"setGenericItemFilter", this, (MethodType)&PoETask::set_generic_item_filter,
                   AhkInt, std::vector<AhkType>{AhkWString});
        add_method(L"setRareItemFilter", this, (MethodType)&PoETask::set_rare_item_filter,
                   AhkInt, std::vector<AhkType>{AhkWString});
    }

    ~PoETask() {
        stop();
    }

    int get_party_status() {
        return in_game_state->server_data()->party_status();
    }

    int get_latency() {
        return in_game_state->server_data()->latency();
    }

    unsigned long get_xp() {
        if (local_player) {
            return local_player->get_component<Player>()->xp();
        }

        return 0;
    }

    AhkObjRef* get_nearest_entity(const wchar_t* text) {
        if (is_in_game()) {
            InGameUI* in_game_ui = in_game_state->in_game_ui();
            shared_ptr<Entity>& entity = in_game_ui->get_nearest_entity(*local_player, text);
            if (entity)
                return (AhkObjRef*)*entity;
        }

        return nullptr;
    }

    AhkObjRef* get_inventory() {
        __set(L"Inventory", nullptr, AhkObject, nullptr);
        if (is_in_game()) {
            InGameUI* in_game_ui = in_game_state->in_game_ui();
            Inventory* inventory = in_game_ui->get_inventory();
            AhkObjRef* ahkobj_ref = (AhkObjRef*)*inventory;
            __set(L"Inventory", ahkobj_ref, AhkObject, nullptr);
            return ahkobj_ref;
        }

        return nullptr;
    }

    AhkObjRef* get_inventory_slots() {
        __set(L"Inventories", nullptr, AhkObject, nullptr);
        if (is_in_game()) {
            ServerData* server_data = in_game_state->server_data();
            AhkObjRef* ahkobj_ref;
            __get(L"Inventories", &ahkobj_ref, AhkObject);
            AhkObj inventory_slots(ahkobj_ref);
            for (auto& i : server_data->get_inventory_slots()) {
                InventorySlot* slot = i.second.get();
                inventory_slots.__set(std::to_wstring(slot->id).c_str(),
                                      (AhkObjRef*)*slot, AhkObject, nullptr);
            }
            return ahkobj_ref;
        }

        return nullptr;
    }

    AhkObjRef* get_stash() {
        __set(L"Stash", nullptr, AhkObject, nullptr);
        if (is_in_game()) {
            InGameUI* in_game_ui = in_game_state->in_game_ui();
            Stash* stash = in_game_ui->get_stash();
            AhkObjRef* ahkobj_ref = (AhkObjRef*)*stash;
            __set(L"Stash", ahkobj_ref, AhkObject, nullptr);
            return ahkobj_ref;
        }

        return nullptr;
    }

    AhkObjRef* get_stash_tabs() {
        __set(L"StashTabs", nullptr, AhkObject, nullptr);
        if (is_in_game()) {
            ServerData* server_data = in_game_state->server_data();
            AhkObjRef* ahkobj_ref;
            __get(L"StashTabs", &ahkobj_ref, AhkObject);
            AhkObj stash_tabs(ahkobj_ref);
            for (auto& i : server_data->get_stash_tabs())
                stash_tabs.__set(L"", (AhkObjRef*)*i, AhkObject, nullptr); 
            return ahkobj_ref;
        }

        return nullptr;
    }

    void add_plugin(PoEPlugin* plugin) {
        plugins.push_back(shared_ptr<PoEPlugin>(plugin));
        plugin->on_load(*this, owner_thread_id);

        log(L"added plugin %s %s", plugin->name, plugin->version);
    }

    bool is_in_game() {
        bool in_game_flag = PoE::is_in_game();

        // wait for loading game instance.
        while (!in_game_state->unknown()) {
            for (auto& i : plugins)
                i->reset();
            Sleep(500);
        }

        if (hwnd) {
            if (!is_attached) {
                is_attached = true;
                PostThreadMessage(owner_thread_id, WM_PTASK_ATTACHED, (WPARAM)hwnd, (LPARAM)0);
            }
        } else {
            if (is_attached) {
                is_attached = false;
                PostThreadMessage(owner_thread_id, WM_PTASK_ATTACHED, (WPARAM)0, (LPARAM)0);
            }
        }

        return in_game_flag;
    }

    void check_player() {
        if (!is_in_game())
            return;

        InGameData* in_game_data = in_game_state->in_game_data();
        if (in_game_data->area_hash() != area_hash) {
            area_hash = in_game_data->area_hash();
            AreaTemplate* world_area = in_game_data->world_area();
            if (!world_area->name().empty()) {
                league  = in_game_state->server_data()->league();
                __set(L"League", league.c_str(), AhkWString, nullptr);
                for (auto i : plugins)
                    i->on_area_changed(in_game_data->world_area(), area_hash);
            }
        }

        local_player = in_game_data->local_player();
        if (local_player) {
            for (auto i : plugins)
                i->on_player(local_player, in_game_state);
        }
    }

    void check_entities() {
        if (GetForegroundWindow() != hwnd || !local_player || !is_in_game())
            return;

        InGameData* in_game_data = in_game_state->in_game_data();
        in_game_data->get_all_entities(entities, ignored_entity_exp);

        for (auto i : plugins)
            i->on_entity_changed(entities.all, entities.removed, entities.added);
    }

    void check_labeled_entities() {
        if (GetForegroundWindow() != hwnd || !local_player || !is_in_game())
            return;

        InGameUI* in_game_ui = in_game_state->in_game_ui();
        in_game_ui->get_all_entities(labeled_entities, labeled_removed);

        for (auto i : plugins)
            i->on_labeled_entity_changed(labeled_entities);
    }

    void run() {
        /* yield the execution to make sure the CreateThread() return,
           otherwise log() function may fail. */
        Sleep(50);

        log(L"PoEapi v%d.%d.%d (supported Path of Exile %s).",
            major_version, minor_version, patch_level, supported_PoE_version);
        
        /* add plugins */
        add_plugin(new AutoFlask());
        add_plugin(new AutoOpen());
        add_plugin(new DelveChest());
        add_plugin(new PlayerStatus());
        add_plugin(new KillCounter());
        add_plugin(auto_pickup);

        /* create jobs */
        start_job(135, [&] {this->check_player();});
        start_job(105, [&] {this->check_labeled_entities();});
        start_job(200, [&] {this->check_entities();});

        log(L"PoE task started (%d jobs).",  jobs.size());
        join(); /* wait for the jobs finish */
        stop();
    }

    int toggle_maphack() {
        char pattern[] = "66 c7 ?? 78 00 ?? c6";
        addrtype addr;
        DWORD old_protect;
        byte buffer[8];

        if (addr = find_pattern(pattern)) {
            VirtualProtectEx(process_handle, (LPVOID)addr, 7, PAGE_EXECUTE_READWRITE, &old_protect);
            ReadProcessMemory(process_handle, (LPVOID)addr, buffer, 7, 0);
            buffer[5] = !buffer[5];
            WriteProcessMemory(process_handle, (LPVOID)addr, buffer, 7, 0);
            VirtualProtectEx(process_handle, (LPVOID)addr, 7, old_protect, &old_protect);
            
            log(L"Maphack %S.", buffer[5] ? L"enabled" : L"disabled");

            return buffer[5];
        }

        return -1;
    }

    int has_buff(wchar_t* buff_name) {
        if (local_player) {
            Life* life = local_player->get_component<Life>();
            auto& buffs = life->get_buffs();
            return (buffs.find(buff_name) != buffs.end());
        }

        return false;
    }

    void begin_pickup() {
        auto_pickup->begin_pickup();
    }

    void stop_pickup() {
        auto_pickup->stop_pickup();
    }

    void set_pickup_range(int range) {
        if (range > 0)
            auto_pickup->range = range;
    }

    void set_generic_item_filter(const wchar_t* regex_string) {
        auto_pickup->generic_item_filter.assign(regex_string);
    }

    void set_rare_item_filter(const wchar_t* regex_string) {
        auto_pickup->rare_item_filter.assign(regex_string);
    }
};

/* Global PoE task object. */
PoETask ptask;

extern "C" WINAPI
BOOL DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        SetProcessDPIAware();

        /* register classes in ahkpp module */
        ahkpp_register(L"PoETask", L"AhkObj", []()->PoETask* {return &ptask;});
        ahkpp_register(L"PoEObject", L"AhkObj", []()->PoEObject* {return new PoEObject(0);});
        ahkpp_register(L"Component", L"PoEObject", []()->Component* {return new Component(0);});
        ahkpp_register(L"Entity", L"PoEObject", []()->Entity* {return new Entity(0);});
        ahkpp_register(L"Element", L"PoEObject", []()->Element* {return new Element(0);});
        ahkpp_register(L"Item", L"Entity", []()->Item* {return new Item(0);});
        ahkpp_register(L"LocalPlayer", L"Entity", []()->LocalPlayer* {return new LocalPlayer(0);});
        ahkpp_register(L"Inventory", L"Element", []()->Inventory* {return new Inventory(0);});
        ahkpp_register(L"InventorySlot", L"AhkObj", []()->InventorySlot* {return new InventorySlot(0);});
        ahkpp_register(L"Stash", L"Element", []()->Stash* {return new Stash(0);});
        ahkpp_register(L"StashTab", L"AhkObj", []()->StashTab* {return new StashTab(0);});
        ahkpp_register(L"Charges", L"Component", []()->Charges* {return new Charges(0);});
        ahkpp_register(L"Flask", L"Component", []()->Flask* {return new Flask(0);});
        break;

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
    ptask.list_game_states();
    ptask.start();
    ptask.join();
}
