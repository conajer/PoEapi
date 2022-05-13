/*
* PoETask.cpp, 8/21/2020 10:07 PM
*/

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "PoE.cpp"
#include "PoEapi.c"
#include "Task.cpp"
#include "PoEPlugin.cpp"
#include "sqlite3.cpp"
#include "curl.cpp"
#include "plugins/AutoFlask.cpp"
#include "plugins/AutoOpen.cpp"
#include "plugins/AutoPickup.cpp"
#include "plugins/Messenger.cpp"
#include "plugins/MinimapSymbol.cpp"
#include "plugins/PlayerStatus.cpp"
#include "plugins/KillCounter.cpp"

struct FileIndex {
    __int64 __padding_0;
    addrtype node_ptr;
    int capacity;
    int __padding_1[3];
    int size;
    int __padding_2;
};

struct FileNode {
    __int64 __padding_0;
    addrtype name;
    __int64 __padding_1;
    int size;
    int __padding_2;
    int capacity;
    int __padding_3[5];
    int area_index;

};

static std::map<wstring, std::map<string, int>&> g_offsets = {
    {L"GameStates", poe_offsets},
    {L"IngameState", in_game_state_offsets},
    {L"IngameData", in_game_data_offsets},
    {L"IngameUI", in_game_ui_offsets},
    {L"ServerData", server_data_offsets},
    {L"Entity", entity_offsets},
    {L"Element", element_offsets},
    {L"StashTab", stash_tab_offsets},
    {L"Inventory", inventory_offsets},
};

class PoETask : public PoE, public Task {
public:
    
    std::mutex task_mutex;
    std::mutex entities_mutex;
    EntitySet entities;
    EntityList labeled_entities, labeled_removed;
    int area_hash;
    wstring league;
    bool in_map = false;

    std::map<wstring, shared_ptr<PoEPlugin>> plugins;
    bool is_attached = false;
    bool is_active = false;
    shared_ptr<Element> hovered_element;
    shared_ptr<Item> hovered_item;

    PoETask() : Task(L"PoETask") {
        /* add jobs */
        add_job(L"GameStateJob", 333, [&] {this->check_game();});
        add_job(L"PlayerStatusJob", 99, [&] {this->check_player();});
        add_job(L"EntityJob", 133, [&] {this->check_entities();});
        add_job(L"LabeledEntityJob", 66, [&] {this->check_labeled_entities();});

        /* add plugins */
        add_plugin(new PlayerStatus());
        add_plugin(new AutoFlask());
        add_plugin(new AutoOpen());
        add_plugin(new Messenger(), true);
        add_plugin(new MinimapSymbol());
        add_plugin(new KillCounter());
        add_plugin(new AutoPickup());

        add_property(L"isReady", &is_ready, AhkBool);
        add_property(L"isAttached", &is_attached, AhkBool);
        add_property(L"isActive", &is_active, AhkBool);

        add_method(L"start", (Task*)this, (MethodType)&Task::start, AhkInt);
        add_method(L"stop", this, (MethodType)&PoETask::stop);
        add_method(L"reset", this, (MethodType)&PoETask::reset);
        add_method(L"getVersion", this, (MethodType)&PoE::get_version, AhkString);
        add_method(L"getLoadedFiles", this, (MethodType)&PoETask::get_loaded_files, AhkObject, ParamList{AhkInt, AhkWString});
        add_method(L"getLatency", this, (MethodType)&PoETask::get_latency);
        add_method(L"getNearestEntity", this, (MethodType)&PoETask::get_nearest_entity, AhkObject, ParamList{AhkWString});
        add_method(L"getPartyStatus", this, (MethodType)&PoETask::get_party_status);
        add_method(L"getInventory", this, (MethodType)&PoETask::get_inventory, AhkObject);
        add_method(L"getInventorySlots", this, (MethodType)&PoETask::get_inventory_slots, AhkObject);
        add_method(L"getIngameUI", this, (MethodType)&PoETask::get_ingame_ui, AhkObject);
        add_method(L"getStash", this, (MethodType)&PoETask::get_stash, AhkObject);
        add_method(L"getStashTabs", this, (MethodType)&PoETask::get_stash_tabs, AhkObject);
        add_method(L"getVendor", this, (MethodType)&PoETask::get_vendor, AhkObject);
        add_method(L"getPurchase", this, (MethodType)&PoETask::get_purchase, AhkObject);
        add_method(L"getSell", this, (MethodType)&PoETask::get_sell, AhkObject);
        add_method(L"getTrade", this, (MethodType)&PoETask::get_trade, AhkObject);
        add_method(L"getChat", this, (MethodType)&PoETask::get_chat, AhkObject);
        add_method(L"getPassiveSkills", this, (MethodType)&PoETask::get_passive_skills, AhkObject);
        add_method(L"getJobs", this, (MethodType)&PoETask::get_jobs, AhkObject);
        add_method(L"setJob", this, (MethodType)&PoETask::set_job, AhkVoid, ParamList{AhkWString, AhkInt});
        add_method(L"getPlugin", this, (MethodType)&PoETask::get_plugin, AhkObject, ParamList{AhkWString});
        add_method(L"getPlugins", this, (MethodType)&PoETask::get_plugins, AhkObject);
        add_method(L"getEntities", this, (MethodType)&PoETask::get_entities, AhkObject, ParamList{AhkWString});
        add_method(L"getPlayer", this, (MethodType)&PoETask::get_player, AhkObject);
        add_method(L"getTerrain", this, (MethodType)&PoETask::get_terrain, AhkObject);
        add_method(L"getHoveredElement", this, (MethodType)&PoETask::get_hovered_element, AhkObject);
        add_method(L"getHoveredItem", this, (MethodType)&PoETask::get_hovered_item, AhkObject);
        add_method(L"setOffset", this, (MethodType)&PoETask::set_offset, AhkVoid, ParamList{AhkWString, AhkString, AhkInt});
        add_method(L"toggleMaphack", this, (MethodType)&PoETask::toggle_maphack, AhkBool);
        add_method(L"getBuffs", this, (MethodType)&PoETask::get_buffs, AhkObject);
        add_method(L"getBuff", this, (MethodType)&PoETask::get_buff, AhkObject, ParamList{AhkWString});
        add_method(L"hasBuff", this, (MethodType)&PoETask::has_buff, AhkInt, ParamList{AhkWString});
        add_method(L"__logout", (PoE*)this, (MethodType)&PoE::logout, AhkVoid);
    }

    ~PoETask() {
        stop();
    }

    void set_job(const wchar_t* name, int period) {
        if (jobs.find(name) != jobs.end())
            jobs[name]->delay = period;
    }

    void add_plugin(PoEPlugin* plugin, bool enabled = false) {
        plugins[plugin->name] = shared_ptr<PoEPlugin>(plugin);
        plugin->enabled = enabled;
        plugin->on_load(*this, owner_thread_id);

        log(L"added plugin %S %s", plugin->name.c_str(), plugin->version.c_str());
    }

    AhkObjRef* get_loaded_files(int area_index, const wchar_t* filter) {
        const char pattern[] = "48 8b 08 48 8d 3d ?? ?? ?? ?? 41";
        addrtype root_ptr;
        FileIndex indices[16];
        wregex regex_filter;

        root_ptr = find_pattern(pattern);
        if (!root_ptr)
            return nullptr;

        if (area_index == 0)
            area_index = in_game_data->area_index();

        AhkTempObj loaded_files;
        regex_filter.assign(filter);
        root_ptr += PoEMemory::read<int>(root_ptr + 0x6) + 0xa;
        PoEMemory::read<FileIndex>(root_ptr, indices, 16);
        for (auto&  i : indices) {
            for (int j = 0; j < (i.capacity + 1) / 8; ++j) {
                FileNode node;
                byte flags[8];
                
                addrtype entry_ptr = i.node_ptr + j * 0xc8;
                PoEMemory::read<byte>(entry_ptr, flags, 8);
                for (int k = 0; k < 8; ++k) {
                    if (flags[k] == 0xff)
                        continue;

                    addrtype file_ptr = PoEMemory::read<addrtype>(entry_ptr + 0x8 + k * 0x18 + 0x8);
                    PoEMemory::read<FileNode>(file_ptr, &node, 1);
                    if (node.area_index == area_index) {
                        wchar_t filename[256];
                        PoEMemory::read<wchar_t>(node.name, filename, 256);
                        if (std::regex_search(filename, regex_filter))
                            loaded_files.__set(L"", filename, AhkWString, nullptr);
                    }
                }
            }
        }

        return loaded_files;
    }

    int get_party_status() {
        return in_game_state->server_data()->party_status();
    }

    int get_latency() {
        return in_game_state->server_data()->latency();
    }

    AhkObjRef* get_nearest_entity(const wchar_t* text) {
        if (is_ready) {
            shared_ptr<Entity>& entity = in_game_ui->get_nearest_entity(*local_player, text);
            if (entity)
                return (AhkObjRef*)*entity;
        }

        return nullptr;
    }

    AhkObjRef* get_ingame_ui() {
        if (is_ready) {
            __set(L"ingameUI", (AhkObjRef*)*in_game_ui, AhkObject, nullptr);
            return *in_game_ui;
        }
        __set(L"ingameUI", nullptr, AhkObject, nullptr);

        return nullptr;
    }

    AhkObjRef* get_inventory() {
        if (is_ready) {
           if (Inventory* inventory = in_game_ui->get_inventory()) {
                __set(L"inventory", (AhkObjRef*)*inventory, AhkObject, nullptr);
                return *inventory;
           }
        }
        __set(L"inventory", nullptr, AhkObject, nullptr);

        return nullptr;
    }

    AhkObjRef* get_inventory_slots() {
        if (is_ready) {
            AhkObj inventory_slots;
            for (auto& i : server_data->get_inventory_slots()) {
                InventorySlot* slot = i.second.get();
                inventory_slots.__set(std::to_wstring(slot->id).c_str(),
                                      (AhkObjRef*)*slot, AhkObject, nullptr);
            }
            __set(L"inventories", (AhkObjRef*)inventory_slots, AhkObject, nullptr);
            return inventory_slots;
        }
        __set(L"inventories", nullptr, AhkObject, nullptr);

        return nullptr;
    }

    AhkObjRef* get_stash() {
        if (is_ready) {
            if (Stash* stash = in_game_ui->get_stash()) {
                stash->__set(L"tabs", nullptr, AhkObject, nullptr);
                __set(L"stash", (AhkObjRef*)*stash, AhkObject, nullptr);
                return *stash;
            }
        }
        __set(L"stash", nullptr, AhkObject, nullptr);

        return nullptr;
    }

    AhkObjRef* get_vendor() {
        if (is_ready) {
            Vendor* vendor = in_game_ui->get_vendor();
            return (AhkObjRef*)*vendor;
        }

        return nullptr;
    }

    AhkObjRef* get_purchase() {
        if (is_ready) {
            Purchase* purchase = in_game_ui->get_purchase();
            return purchase ? (AhkObjRef*)*purchase : nullptr;
        }

        return nullptr;
    }

    AhkObjRef* get_sell() {
        if (is_ready) {
            Sell* sell = in_game_ui->get_sell();
            return sell ? (AhkObjRef*)*sell : nullptr;
        }

        return nullptr;
    }

    AhkObjRef* get_trade() {
        if (is_ready) {
            Trade* trade = in_game_ui->get_trade();
            return (AhkObjRef*)*trade;
        }

        return nullptr;
    }

    AhkObjRef* get_chat() {
        if (is_ready) {
            Chat* chat = in_game_ui->get_chat();
            return (AhkObjRef*)*chat;
        }

        return nullptr;
    }

    AhkObjRef* get_passive_skills() {
        if (is_ready) {
            AhkObj passive_skills;
            
            for (auto i : server_data->get_passive_skills())
                passive_skills.__set(L"", i, AhkInt, nullptr);
            __set(L"passiveSkills", (AhkObjRef*)passive_skills, AhkObject, nullptr);
            return passive_skills;
        }

        return nullptr;
    }

    AhkObjRef* get_stash_tabs() {
        if (is_ready) {
            AhkObj stash_tabs;
            for (auto& i : server_data->get_stash_tabs()) {
                if (i->folder_id == -1) {
                    stash_tabs.__set(L"", (AhkObjRef*)*i, AhkObject, nullptr);
                    if (i->type == 16) {
                        AhkObj tabs;
                        for (auto& t : i->tabs)
                            tabs.__set(L"", (AhkObjRef*)*t, AhkObject, nullptr);
                        i->__set(L"tabs", (AhkObjRef*)tabs, AhkObject, nullptr);
                    }
                }
            }
            __set(L"stashTabs", (AhkObjRef*)stash_tabs, AhkObject, nullptr);
            return stash_tabs.obj_ref;
        }
        __set(L"stashTabs", nullptr, AhkObject, nullptr);

        return nullptr;
    }

    AhkObjRef* get_jobs() {
        AhkTempObj temp_jobs;
        for (auto& i : jobs) {
            AhkObj job;
            job.__set(L"name", i.second->name.c_str(), AhkWString,
                      L"id", i.second->id, AhkUInt,
                      L"delay", i.second->delay, AhkInt,
                      nullptr);
            temp_jobs.__set(i.first.c_str(), (AhkObjRef*)job, AhkObject, nullptr);
        }
        return temp_jobs;
    }

    AhkObjRef* get_plugin(wchar_t* name) {
        if (plugins.find(name) != plugins.end())
            return *plugins[name];
        return nullptr;
    }

    AhkObjRef* get_plugins() {
        AhkObj temp_plugins;
        for (auto& i : plugins) {
            temp_plugins.__set(i.second->name.c_str(), (AhkObjRef*)*i.second, AhkObject, nullptr);
        }
        __set(L"plugins", (AhkObjRef*)temp_plugins, AhkObject, nullptr);

        return temp_plugins;
    }

    AhkObjRef* get_entities(const wchar_t* types) {
        AhkTempObj temp_entities;
        std::wregex types_exp(types);
        for (auto& i : entities.all) {
            if (std::regex_search(i.second->path, types_exp))
                temp_entities.__set(L"", (AhkObjRef*)*i.second, AhkObject, nullptr);
        }

        return temp_entities;
    }

    AhkObjRef* get_player() {
        if (local_player)
            return (AhkObjRef*)*local_player;
        return nullptr;
    }

    AhkObjRef* get_terrain() {
        if (is_ready)
            return *in_game_data->get_terrain();
        return nullptr;
    }

    AhkObjRef* get_hovered_element() {
        hovered_element = in_game_state->get_hovered_element();
        return hovered_element ? (AhkObjRef*)*hovered_element : nullptr;
    }

    AhkObjRef* get_hovered_item() {
        hovered_item = in_game_state->get_hovered_item();
        return hovered_item ? (AhkObjRef*)*hovered_item : nullptr;
    }

    void set_offset(wchar_t* catalog, char* key, int value) {
        auto i = g_offsets.find(catalog);
        if (i != g_offsets.end())
            i->second[key] = value;
    }

    void reset() {
        std::unique_lock<std::mutex> lock(task_mutex);

        // reset plugins.
        for (auto& i : plugins)
            i.second->reset();

        // clear cached entities.
        entities.all.clear();
        labeled_entities.clear();

        is_ready = false;
        if (in_game_flag && !is_loading) {
            in_game_state->reset();
            in_game_ui = in_game_state->in_game_ui();
            in_game_data = in_game_state->in_game_data();
            server_data = in_game_state->server_data();
            if (!in_game_ui || !in_game_data || !server_data)
                return;

            AreaTemplate* world_area = in_game_data->world_area();
            local_player = in_game_data->local_player();
            if (world_area->name().empty() || !local_player)
                return;

            is_ready = true;
            league  = in_game_state->server_data()->league();
            AreaTemplate* current_area = in_game_data->world_area();
            in_map = current_area->is_map();
            __set(L"league", league.c_str(), AhkWString,
                  L"areaName", current_area->name().c_str(), AhkWString,
                  L"areaLevel", current_area->level(), AhkInt,
                  nullptr);

            get_inventory_slots();
            get_stash_tabs();
            get_stash();
            get_inventory();
        }
    }

    void check_window_state() {
        if (!window) {
            if (is_attached) {
                is_attached = false;
                PostThreadMessage(owner_thread_id, WM_PTASK_ATTACHED, (WPARAM)0, (LPARAM)0);
            }
            area_hash = 0;
            return;
        }

        if (!is_attached) {
            is_attached = true;
            PostThreadMessage(owner_thread_id, WM_PTASK_ATTACHED, (WPARAM)window, (LPARAM)0);
            Sleep(100);
        }

        HANDLE active_window = GetForegroundWindow();
        if (active_window != window) {
            if (is_active) {
                is_active = false;
                PostThreadMessage(owner_thread_id, WM_PTASK_ACTIVE, (WPARAM)active_window, (LPARAM)0);
            }
        } else {
            if (!is_active) {
                is_active = true;
                PostThreadMessage(owner_thread_id, WM_PTASK_ACTIVE, (WPARAM)active_window, (LPARAM)0);
            }
        }
    }

    enum {
        INVENTORY,
        STASH,
        SELL,
        PURCHASE,
        TRADE
    };

    void check_stash_and_inventory() {
        static bool screen_flags[5];
        static int stash_tab_index = 0;

        if (!in_game_flag || is_loading) {
            for (int i = 0; i < 5; ++i)
                screen_flags[i] = 0;
            stash_tab_index = 0;
        } else if (is_ready) {
            // inventory
            if (in_game_ui->inventory && in_game_ui->inventory->is_visible() ^ screen_flags[INVENTORY]) {
                screen_flags[INVENTORY] = !screen_flags[INVENTORY];
                if (screen_flags[INVENTORY])
                    PostThreadMessage(owner_thread_id, WM_INVENTORY_OPENED, (WPARAM)0, (LPARAM)0);
            }

            // stash
            if (in_game_ui->stash) {
                bool stash_opened = in_game_ui->stash->is_visible();
                int tab_index = stash_opened ? in_game_ui->stash->active_tab_index() : -1;

                if ((stash_opened ^ screen_flags[STASH]) || (stash_opened && tab_index != stash_tab_index)) {
                    screen_flags[STASH] = stash_opened;
                    stash_tab_index = tab_index;
                    PostThreadMessage(owner_thread_id, WM_STASH_CHANGED, (WPARAM)screen_flags[STASH], (LPARAM)stash_tab_index + 1);
                }
            }

            // trade
            if (in_game_ui->childs[106] && (in_game_ui->childs[106]->is_visible() ^ screen_flags[TRADE])) {
                screen_flags[TRADE] = !screen_flags[TRADE];
                if (screen_flags[TRADE])
                    PostThreadMessage(owner_thread_id, WM_TRADE, (WPARAM)screen_flags[TRADE], (LPARAM)0);
            }
        }
    }

    void check_game() {
        if (!task_mutex.try_lock())
            return;

        check_game_state();
        check_window_state();
        check_stash_and_inventory();

        if (!in_game_flag) {
            if (is_ready) {
                in_game_data->force_reset = true;
                is_ready = false;
                PostThreadMessage(owner_thread_id, WM_PTASK_EXIT, (WPARAM)0, (LPARAM)0);
            }
            Sleep(500);
        } else {
            if (is_loading) {
                is_ready = false;

                // wait for loading the game instance.
                while (is_loading) {
                    check_game_state();
                }
            }

            if (!is_ready)
                PostThreadMessage(owner_thread_id, WM_PTASK_LOADED, (WPARAM)0, (LPARAM)0);
        }
        task_mutex.unlock();
    }

    void check_player() {
        std::unique_lock<std::mutex> lock(task_mutex);
        if (!is_ready)
            return;

        if (in_game_data->area_hash() != area_hash) {
            area_hash = in_game_data->area_hash();
            AreaTemplate* world_area = in_game_data->world_area();
            if (!world_area->name().empty()) {
                for (auto i : plugins)
                    i.second->on_area_changed(in_game_data->world_area(), area_hash, local_player);
            }
        }

        for (auto& i : plugins) {
            if (is_ready && i.second->enabled)
                i.second->on_player(local_player, in_game_state.get());
        }
    }

    void check_entities() {
        std::unique_lock<std::mutex> lock(task_mutex);
        if (!is_ready || !is_active)
            return;

        {
            std::unique_lock<std::mutex> lock(entities_mutex);
            in_game_data->get_all_entities(entities);
            for (auto& i : plugins) {
                if (is_ready && i.second->enabled && i.second->player)
                    i.second->on_entity_changed(entities.all, entities.removed, entities.added);
            }
        }
    }

    void check_labeled_entities() {
        std::unique_lock<std::mutex> lock(task_mutex);
        if (!is_ready || !is_active)
            return;

        in_game_ui->get_all_entities(labeled_entities, labeled_removed);
        for (auto& i : plugins) {
            if (is_ready && i.second->enabled && i.second->player)
                i.second->on_labeled_entity_changed(labeled_entities);
        }
    }

    void render() {
        // clear overlay canvas.
        clear();

        std::unique_lock<std::mutex> lock(entities_mutex);
        if (!is_ready || in_game_ui->has_active_panel())
            return;

        // update the entity data.
        for (auto& i : entities.all) {
            if (i.second->is_movable)
                i.second->get_position();
        }
        local_player->get_position();

        for (auto& i : plugins)
            if (i.second->enabled && i.second->player)
                i.second->render();
    }

    void run() {
        /* yield the execution to make sure the CreateThread() return,
           otherwise log() function may fail. */
        Sleep(50);

        log(L"PoEapi v%d.%d.%d (supported Path of Exile %s).",
            major_version, minor_version, patch_level, supported_PoE_version);

        log(L"PoE task started (%d jobs).",  jobs.size());
        Task::run();
    }

    void stop() {
        is_ready = false;
        if (in_game_data)
            in_game_data->force_reset = true;
        Hud::stop();
        Task::stop();
        std::unique_lock<std::mutex> lock(task_mutex);
    }

    bool toggle_maphack() {
        const char pattern[] = "66 C7 46 58 ?? 00 C6";

        HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, false, process_id);
        if (!handle)
            return false;

        if (addrtype addr = find_pattern(pattern)) {
            byte flag = PoEMemory::read<byte>(addr + 4) ? 0 : 2;
            if (::write<byte>(handle, addr + 4, &flag, 1)) {
                log(L"Maphack <b style=\"color:blue\">%S</b>.", flag ? L"Enabled" : L"Disabled");
                CloseHandle(handle);
                return true;
            }
        }
        CloseHandle(handle);
            
        return false;
    }

    AhkObjRef* get_buffs() {
        if (local_player) {
            Buffs* buffs = local_player->get_component<Buffs>();
            if (buffs) {
                AhkTempObj temp_buffs;
                for (auto& i : buffs->get_buffs()) {
                    AhkObj buff;
                    buff.__set(L"name", i.first.c_str(), AhkWString,
                               L"description", i.second.description().c_str(), AhkWString,
                               L"duration", i.second.duration(), AhkFloat,
                               L"timer", i.second.timer(), AhkFloat,
                               L"charges", i.second.charges(), AhkInt,
                               nullptr);
                    temp_buffs.__set(L"", (AhkObjRef*)buff, AhkObject, nullptr);
                }

                return temp_buffs;
            }
        }

        return nullptr;
    }

    AhkObjRef* get_buff(const wchar_t* name) {
        if (local_player) {
            auto& buffs = local_player->get_component<Buffs>()->get_buffs();
            auto i = buffs.find(name);
            if (i != buffs.end()) {
                AhkTempObj buff;
                buff.__set(L"name", name, AhkWString,
                           L"description", i->second.description().c_str(), AhkWString,
                           L"duration", i->second.duration(), AhkFloat,
                           L"timer", i->second.timer(), AhkFloat,
                           L"charges", i->second.charges(), AhkInt,
                           nullptr);
                return buff;
            }
        }

        return nullptr;
    }

    int has_buff(const wchar_t* name) {
        if (is_ready && local_player) {
            Buffs* buffs = local_player->get_component<Buffs>();
            return buffs->has_buff(name);
        }

        return 0;
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
        ahkpp_register(L"curl", L"AhkObj", []()->curl* {return new curl();});
        ahkpp_register(L"sqlite3", L"AhkObj", []()->sqlite3* {return new sqlite3();});
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }

    return true;
}

int main(int argc, char* argv[]) {
    SetProcessDPIAware();
    ptask.start();
    ptask.list_game_states();
    ptask.plugins[L"MinimapSymbol"]->enabled = true;
    while (1) {
        if (!ptask.is_ready) {
            if (ptask.in_game_flag && !ptask.is_loading)
                ptask.reset();
        }
        Sleep(1000);
    }
}
