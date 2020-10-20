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
#include "plugins/HeistChest.cpp"
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
        ignored_entity_exp(L"Doodad|WorldItem")
    {
        add_method(L"start", (Task*)this, (MethodType)&Task::start, AhkInt);
        add_method(L"stop", (Task*)this, (MethodType)&Task::stop);
        add_method(L"enablePlugin", this, (MethodType)&PoETask::enable_plugin, AhkVoid, ParamList{AhkWString});
        add_method(L"disablePlugin", this, (MethodType)&PoETask::disable_plugin, AhkVoid, ParamList{AhkWString});
        add_method(L"getLatency", this, (MethodType)&PoETask::get_latency);
        add_method(L"getNearestEntity", this, (MethodType)&PoETask::get_nearest_entity, AhkObject, ParamList{AhkWString});
        add_method(L"getPartyStatus", this, (MethodType)&PoETask::get_party_status);
        add_method(L"getXP", this, (MethodType)&PoETask::get_xp, AhkUInt);
        add_method(L"getInventory", this, (MethodType)&PoETask::get_inventory, AhkObject);
        add_method(L"getInventorySlots", this, (MethodType)&PoETask::get_inventory_slots, AhkObject);
        add_method(L"getStash", this, (MethodType)&PoETask::get_stash, AhkObject);
        add_method(L"getStashTabs", this, (MethodType)&PoETask::get_stash_tabs, AhkObject);
        add_method(L"getVendor", this, (MethodType)&PoETask::get_vendor, AhkObject);
        add_method(L"getSell", this, (MethodType)&PoETask::get_sell, AhkObject);
        add_method(L"toggleMaphack", this, (MethodType)&PoETask::toggle_maphack, AhkBool);
        add_method(L"toggleHealthBar", this, (MethodType)&PoETask::toggle_health_bar, AhkBool);
        add_method(L"hasBuff", this, (MethodType)&PoETask::has_buff, AhkInt, ParamList{AhkWString});
        add_method(L"beginPickup", this, (MethodType)&PoETask::begin_pickup);
        add_method(L"stopPickup", this, (MethodType)&PoETask::stop_pickup);
        add_method(L"setPickupRange", this, (MethodType)&PoETask::set_pickup_range, AhkInt, ParamList{AhkInt});
        add_method(L"setGenericItemFilter", this, (MethodType)&PoETask::set_generic_item_filter, AhkInt, ParamList{AhkWString});
        add_method(L"setRareItemFilter", this, (MethodType)&PoETask::set_rare_item_filter, AhkInt, ParamList{AhkWString});
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

    AhkObjRef* get_vendor() {
        if (is_in_game()) {
            InGameUI* in_game_ui = in_game_state->in_game_ui();
            Vendor* vendor = in_game_ui->get_vendor();
            return (AhkObjRef*)*vendor;
       }

        return nullptr;
    }

    AhkObjRef* get_sell() {
        if (is_in_game()) {
            InGameUI* in_game_ui = in_game_state->in_game_ui();
            Sell* sell = in_game_ui->get_sell();
            return (AhkObjRef*)*sell;
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

        log(L"added plugin %S %s", plugin->name.c_str(), plugin->version.c_str());
    }

    void enable_plugin(const wchar_t* name) {
        for (auto& i : plugins) {
            if (!i->enabled && i->name == name) {
                i->enabled = true;
                log(L"enabled plugin %S", i->name.c_str());
                break;
            }
        }
    }

    void disable_plugin(const wchar_t* name) {
        for (auto& i : plugins) {
            if (i->enabled && i->name == name) {
                i->enabled = false;
                log(L"disabled plugin %S", i->name.c_str());
                break;
            }
        }
    }

    bool is_in_game() {
        bool in_game_flag = PoE::is_in_game();

        if (!in_game_flag) {
            if (hwnd && !is_attached) {
                is_attached = true;
                PostThreadMessage(owner_thread_id, WM_PTASK_ATTACHED, (WPARAM)hwnd, (LPARAM)0);
            }

            // increase the delay of timers when PoE isn't in game state.
            Sleep(1000);
        } else if (!in_game_state->unknown()) {
            // reset plugins.
            for (auto& i : plugins)
                i->reset();

            // wait for loading the game instance.
            while (!in_game_state->unknown())
                Sleep(500);

            // clear cached entities.
            entities.all.clear();
            labeled_entities.clear();
        }

        return in_game_flag;
    }

    void check_player() {
        if (!is_in_game()) {
            if (is_attached && !hwnd) {
                is_attached = false;
                PostThreadMessage(owner_thread_id, WM_PTASK_ATTACHED, (WPARAM)0, (LPARAM)0);
            }
            return;
        }

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
                if (i->enabled)
                    i->on_player(local_player, in_game_state);
        }

        if (!is_attached && hwnd) {
            is_attached = true;
            PostThreadMessage(owner_thread_id, WM_PTASK_ATTACHED, (WPARAM)hwnd, (LPARAM)0);
        }
    }

    void check_entities() {
        if (GetForegroundWindow() != hwnd || !local_player || !is_in_game())
            return;

        InGameData* in_game_data = in_game_state->in_game_data();
        in_game_data->get_all_entities(entities, ignored_entity_exp);

        for (auto i : plugins)
            if (i->enabled)
                i->on_entity_changed(entities.all, entities.removed, entities.added);
    }

    void check_labeled_entities() {
        if (GetForegroundWindow() != hwnd || !local_player || !is_in_game())
            return;

        InGameUI* in_game_ui = in_game_state->in_game_ui();
        in_game_ui->get_all_entities(labeled_entities, labeled_removed);

        for (auto i : plugins)
            if (i->enabled)
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
        add_plugin(new HeistChest());
        add_plugin(new PlayerStatus());
        add_plugin(new KillCounter());
        add_plugin(auto_pickup);

        /* create jobs */
        start_job(135, [&] {this->check_player();});
        start_job(33, [&] {this->check_labeled_entities();});
        start_job(200, [&] {this->check_entities();});

        log(L"PoE task started (%d jobs).",  jobs.size());
        join(); /* wait for the jobs finish */
        stop();
    }

    bool toggle_maphack() {
        const char pattern[] = "66 c7 ?? 78 00 ?? c6";
        if (addrtype addr = find_pattern(pattern)) {
            byte flag = !read<byte>(addr + 5);
            if (write<byte>(addr + 5, &flag, 1)) {
                log(L"Maphack <b style=\"color:blue\">%S</b>.",
                    flag ? L"Enabled" : L"Disabled");
                return true;
            }
        }
            
        return false;
    }

    bool toggle_health_bar() {
        char pattern[] = "?? ?? 44 8b 82 ?? ?? 00 00 8b 82 ?? ?? 00 00 41 0f af c0";
        if (addrtype addr = find_pattern(pattern)) {
            byte flag = read<byte>(addr);
            flag = (flag == 0x7c) ? 0xeb : 0x7c;
            if (write<byte>(addr, &flag, 1)) {
                log(L"Health bar <b style=\"color:blue\">%S</b>.",
                    (flag == 0xeb) ? L"Enabled" : L"Disabled");
                return true;
            }
        }

        return false;
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
        ahkpp_register(L"Vendor", L"Element", []()->Vendor* {return new Vendor(0);});
        ahkpp_register(L"Sell", L"Element", []()->Sell* {return new Sell(0);});
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
