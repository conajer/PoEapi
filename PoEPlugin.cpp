/*
* PoEPlugin.cpp, 8/21/2020 9:08 PM
*/

class PoEPlugin {
public:

    const char* name;
    const char* version;
    PoE* poe;
    DWORD thread_id;
    bool to_reset = false;
    buffer<wchar_t> log_buffer;

    PoEPlugin(const char* name, const char* version_string = "0.1")
        : name(name), version(version_string), log_buffer(256)
    {
    }

    void reset() {
        to_reset = true;
    }

    virtual void on_load(PoE& poe, int ownere_thread_id) {
        this->poe = &poe;
        this->thread_id = ownere_thread_id;
    }

    virtual void on_player(LocalPlayer* player, InGameState* in_game_state) {
    }

    virtual void on_area_changed(AreaTemplate* world_area, int hash_code) {
    }

    virtual void on_entity_changed(EntityList& all, EntityList& removed, EntityList& added) {
    }

    virtual void on_labeled_entity_changed(EntityList& entities) {
    }

    void log(const wchar_t* format, ...) {
        va_list args;
        wchar_t* buffer = log_buffer;

        buffer += swprintf(buffer, L"[%s] ", (char*)name);
        va_start(args, format);
        vswprintf(buffer, format, args);
        va_end(args);
        PostThreadMessage(thread_id, WM_POEAPI_LOG, (WPARAM)log_buffer, 0);

        std::wcout << (const wchar_t*)log_buffer << std::endl;
    }
};
