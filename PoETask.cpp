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
#include "plugins/Messenger.cpp"
#include "plugins/MinimapSymbol.cpp"
#include "plugins/HeistChest.cpp"
#include "plugins/PlayerStatus.cpp"
#include "plugins/KillCounter.cpp"

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
    bool is_active = false;

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
        add_method(L"getIngameUI", this, (MethodType)&PoETask::get_ingame_ui, AhkObject);
        add_method(L"getStash", this, (MethodType)&PoETask::get_stash, AhkObject);
        add_method(L"getStashTabs", this, (MethodType)&PoETask::get_stash_tabs, AhkObject);
        add_method(L"getVendor", this, (MethodType)&PoETask::get_vendor, AhkObject);
        add_method(L"getSell", this, (MethodType)&PoETask::get_sell, AhkObject);
        add_method(L"getTrade", this, (MethodType)&PoETask::get_trade, AhkObject);
        add_method(L"getChat", this, (MethodType)&PoETask::get_chat, AhkObject);
        add_method(L"getPassiveSkills", this, (MethodType)&PoETask::get_passive_skills, AhkObject);
        add_method(L"getPlugins", this, (MethodType)&PoETask::get_plugins, AhkObject);
        add_method(L"getEntities", this, (MethodType)&PoETask::get_entities, AhkObject);
        add_method(L"toggleMaphack", this, (MethodType)&PoETask::toggle_maphack, AhkBool);
        add_method(L"toggleHealthBar", this, (MethodType)&PoETask::toggle_health_bar, AhkBool);
        add_method(L"hasBuff", this, (MethodType)&PoETask::has_buff, AhkInt, ParamList{AhkWString});
        add_method(L"beginPickup", this, (MethodType)&PoETask::begin_pickup);
        add_method(L"stopPickup", this, (MethodType)&PoETask::stop_pickup);
        add_method(L"setHudWindow", (PoE*)this, (MethodType)&PoE::set_hud_window, AhkVoid, ParamList{AhkUInt});
        add_method(L"__logout", (PoE*)this, (MethodType)&PoE::logout, AhkVoid);
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

    AhkObjRef* get_ingame_ui() {
        if (is_in_game()) {
            InGameUI* in_game_ui = in_game_state->in_game_ui();
            __set(L"ingameUI", (AhkObjRef*)*in_game_ui, AhkObject, nullptr);

            return in_game_ui->obj_ref;
        }
        __set(L"ingameUI", nullptr, AhkObject, nullptr);

        return nullptr;
    }

    AhkObjRef* get_inventory() {
        if (is_in_game()) {
            InGameUI* in_game_ui = in_game_state->in_game_ui();
            Inventory* inventory = in_game_ui->get_inventory();
            __set(L"inventory", (AhkObjRef*)*inventory, AhkObject, nullptr);
            return inventory->obj_ref;
        }
        __set(L"inventory", nullptr, AhkObject, nullptr);

        return nullptr;
    }

    AhkObjRef* get_inventory_slots() {
        if (is_in_game()) {
            ServerData* server_data = in_game_state->server_data();
            AhkObj inventory_slots;
            for (auto& i : server_data->get_inventory_slots()) {
                InventorySlot* slot = i.second.get();
                inventory_slots.__set(std::to_wstring(slot->id).c_str(),
                                      (AhkObjRef*)*slot, AhkObject, nullptr);
            }
            __set(L"inventories", (AhkObjRef*)inventory_slots, AhkObject, nullptr);
            return inventory_slots.obj_ref;
        }
        __set(L"inventories", nullptr, AhkObject, nullptr);

        return nullptr;
    }

    AhkObjRef* get_stash() {
        if (is_in_game()) {
            InGameUI* in_game_ui = in_game_state->in_game_ui();
            Stash* stash = in_game_ui->get_stash();
            __set(L"stash", (AhkObjRef*)*stash, AhkObject, nullptr);
            return stash->obj_ref;
        }
        __set(L"stash", nullptr, AhkObject, nullptr);

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

    AhkObjRef* get_trade() {
        if (is_in_game()) {
            InGameUI* in_game_ui = in_game_state->in_game_ui();
            Trade* trade = in_game_ui->get_trade();
            return (AhkObjRef*)*trade;
       }

        return nullptr;
    }

    AhkObjRef* get_chat() {
        if (is_in_game()) {
            InGameUI* in_game_ui = in_game_state->in_game_ui();
            Chat* chat = in_game_ui->get_chat();
            return (AhkObjRef*)*chat;
       }

        return nullptr;
    }

    AhkObjRef* get_passive_skills() {
        if (is_in_game()) {
            ServerData* server_data = in_game_state->server_data();
            AhkObj passive_skills;
            
            for (auto i : server_data->get_passive_skills())
                passive_skills.__set(L"", i, AhkInt, nullptr);
            __set(L"passiveSkills", (AhkObjRef*)passive_skills, AhkObject, nullptr);
            return passive_skills;
       }

        return nullptr;
    }

    AhkObjRef* get_stash_tabs() {
        if (is_in_game()) {
            ServerData* server_data = in_game_state->server_data();
            AhkObj stash_tabs;
            for (auto& i : server_data->get_stash_tabs()) {
                if (i->folder_id == -1)
                    stash_tabs.__set(L"", (AhkObjRef*)*i, AhkObject, nullptr);
            }
            __set(L"stashTabs", (AhkObjRef*)stash_tabs, AhkObject, nullptr);
            return stash_tabs.obj_ref;
        }
        __set(L"stashTabs", nullptr, AhkObject, nullptr);

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

    AhkObjRef* get_plugins() {
        AhkObj temp_plugins;
        for (auto& i : plugins) {
            temp_plugins.__set(i->name.c_str(), (AhkObjRef*)*i, AhkObject, nullptr);
        }
        __set(L"plugins", (AhkObjRef*)temp_plugins, AhkObject, nullptr);

        return temp_plugins;
    }

    AhkObjRef* get_entities() {
        AhkTempObj temp_entities;
        for (auto& i : entities.all) {
            temp_entities.__set(L"", (AhkObjRef*)*i.second, AhkObject, nullptr);
        }

        return temp_entities;
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
        HANDLE h = GetForegroundWindow();
        if (h != hwnd) {
            if (is_active) {
                PostThreadMessage(owner_thread_id, WM_PTASK_ACTIVE, (WPARAM)h, (LPARAM)0);
                is_active = false;
            }
        } else if (!is_active) {
            PostThreadMessage(owner_thread_id, WM_PTASK_ACTIVE, (WPARAM)h, (LPARAM)0);
            is_active = true;
        }

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
                __set(L"league", league.c_str(), AhkWString,
                      L"area", in_game_data->world_area()->name().c_str(), AhkWString,
                      nullptr);

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
        add_plugin(new HeistChest());
        add_plugin(new Messenger());
        add_plugin(new MinimapSymbol());
        add_plugin(new PlayerStatus());
        add_plugin(new KillCounter());
        add_plugin(auto_pickup);

        /* create jobs */
        start_job(135, [&] {this->check_player();});
        start_job(33, [&] {this->check_labeled_entities();});
        start_job(55, [&] {this->check_entities();});

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

    int has_buff(wchar_t* name) {
        if (local_player) {
            Life* life = local_player->get_component<Life>();
            auto& buffs = life->get_buffs();
            auto i = buffs.find(name);
            if (i != buffs.end())
                return i->second.charges();
        }

        return 0;
    }

    void begin_pickup() {
        auto_pickup->begin_pickup();
    }

    void stop_pickup() {
        auto_pickup->stop_pickup();
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
        ahkpp_register(L"InGameUI", L"Element", []()->InGameUI* {return new InGameUI(0);});
        ahkpp_register(L"Inventory", L"Element", []()->Inventory* {return new Inventory(0);});
        ahkpp_register(L"Stash", L"Element", []()->Stash* {return new Stash(0);});
        ahkpp_register(L"StashTab", L"AhkObj", []()->StashTab* {return new StashTab(0);});
        ahkpp_register(L"Vendor", L"Element", []()->Vendor* {return new Vendor(0);});
        ahkpp_register(L"Sell", L"Element", []()->Sell* {return new Sell(0);});
        ahkpp_register(L"Trade", L"Sell", []()->Trade* {return new Trade(0);});
        ahkpp_register(L"Chat", L"Element", []()->Chat* {return new Chat(0);});
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
