/*
* PoEPlugin.cpp, 8/21/2020 9:08 PM
*/

class PoEPlugin : public AhkObj {
public:

    wstring name;
    string version;
    PoE* poe;
    DWORD thread_id;
    bool force_reset = false;
    bool enabled = false;
    buffer<wchar_t> log_buffer;

    LocalPlayer* player;

    PoEPlugin(const wchar_t* name, const char* version_string = "0.1")
        : name(name), version(version_string), log_buffer(256)
    {
        add_property(L"enabled", &enabled, AhkBool);
    }

    void __new() {
        AhkObj::__new();
        __set(L"name", name.c_str(), AhkWString,
              L"version", version.c_str(), AhkString,
              nullptr);
    }

    virtual void reset() {
        force_reset = true;
        player = nullptr;
    }

    virtual void render() {
    }

    virtual void on_load(PoE& poe, int owner_thread_id) {
        this->poe = &poe;
        this->thread_id = owner_thread_id;
        this->player = nullptr;
    }

    virtual void on_player(LocalPlayer* player, InGameState* in_game_state) {
        this->player =  player;
    }

    virtual void on_area_changed(AreaTemplate* world_area, int hash_code, LocalPlayer* player) {
    }

    virtual void on_entity_changed(EntityList& all, EntityList& removed, EntityList& added) {
    }

    virtual void on_labeled_entity_changed(EntityList& entities) {
    }

    void log(const wchar_t* format, ...) {
        va_list args;
        wchar_t* buffer = log_buffer;

        buffer += swprintf(buffer, L"[%S] ", name.c_str());
        va_start(args, format);
        vswprintf(buffer, format, args);
        va_end(args);
        PostThreadMessage(thread_id, WM_POEAPI_LOG, (WPARAM)log_buffer, 0);

        std::wcout << (const wchar_t*)log_buffer << std::endl;
    }
};
