/*
*  PoE.cpp, 8/4/2020 9:11 PM
*
*  Path of Exile application interface.
*/

#include <windows.h>
#include <psapi.h>

#include <iostream>
#include <map>
#include <memory>
#include <queue>

using namespace std;

typedef unsigned __int64 addrtype;

#include "ahkpp"
#include "PoEMemory.cpp"
#include "RemoteMemoryObject.cpp"

class PoE : public PoEMemory {
private:

    int get_process_by_name(const char* name) {
        DWORD processes[1024], size;

        if (!EnumProcesses(processes, sizeof(processes), &size))
            return 0;

        for (int i = 0; i < size / sizeof(DWORD); i++) {
            char module_name[MAX_PATH] = "";

            HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processes[i]);
            if (handle) {
                HMODULE module;
                DWORD size;

                if (EnumProcessModules(handle, &module, sizeof(module), &size)) {
                    GetModuleBaseName(handle, module, module_name, MAX_PATH);
                    CloseHandle(handle);

                    if (!strcasecmp(module_name, name))
                        return processes[i];
                }
            }
        }

        return 0;
    }

    HWND get_hwnd() {
        DWORD pid;

        while (1) {
            HWND hwnd = FindWindow(nullptr, "Path of Exile");
            if (hwnd == nullptr)
                break;

            GetWindowThreadProcessId(hwnd, &pid);
            if (pid == process_id)
                return hwnd;
        }

        return 0;
    }

    int parse_pattern(const char* pattern_str, byte* pattern, byte* mask) {
        int len = 0;

        while (*pattern_str) {
            if (*pattern_str == ' ') {
                pattern_str++;
                continue;
            }

            if (*pattern_str == 'x' || *pattern_str == '?') {
                pattern_str += 2;
                mask[len++] = '?';
                continue;
            }

            int c, p = 0;
            for (int i = 0; i < 2; i++) {
                c = pattern_str[i];
                if (c >= '0' && c <= '9')
                    p = (p << 4) | (c - 0x30);
                else if (c >= 'a' && c <= 'f')
                    p = (p << 4) | (c - 0x57);
                else if (c >= 'A' && c <= 'F')
                    p = (p << 4) | (c - 0x37);
            }
            pattern[len] = p;
            mask[len++] = 'x';
            pattern_str += 2;
        }

        return len;
    }

    int compare(byte* pattern, byte* data, byte* mask, int len) {
        int matched = true;

        for (int i = 0; i < len; i++) {
            if (mask[i] == 'x' && pattern[i] != data[i]) {
                matched = false;
                break;
            }
        }

        return matched;
    }

    addrtype find_pattern(const char* pattern_str) {
        byte* pattern = new byte[256];
        byte* mask = new byte[256];
        int len = parse_pattern(pattern_str, pattern, mask);

        addrtype end_address = address + size_of_image;
        byte* buffer = new byte[size_of_image];
        
        if (read(address, buffer, size_of_image)) {
            for (int i = 0; i < size_of_image - len; i++) {
                if (compare(pattern, &buffer[i], mask, len))
                    return address + i;
            }
        }

        return 0;
    }

public:

    const char* executable_name = "PathOfExile_x64.exe";
    addrtype address;
    int size_of_image;
    int process_id;
    HWND hwnd;
    GameState *active_game_state;
    InGameState* in_game_state;

    GameStateController* game_state_controller;

    PoE() : game_state_controller(0) {
        open_target_process();
    }

    GameStateController* get_game_state_controller() {
        char pattern[] = "48 83 ec 50 48 c7 44 24 ?? ?? ?? ?? ?? 48 89 9c 24 ?? ?? ?? ?? 48 8b f9 33 ed 48 39";
        addrtype addr = find_pattern(pattern);
        if (!addr)
            return nullptr;

        addr = read<addrtype>(addr + read<int>(addr + 29) + 33);
        if (!addr)
            return nullptr;
        delete game_state_controller;

        return new GameStateController(addr);
    }

    GameState* get_active_game_state() {
        in_game_state = nullptr;
        if (game_state_controller) {
            GameState *game_state = game_state_controller->get_active_game_state();
            if (game_state && game_state->is(L"InGameState"))
                in_game_state = (InGameState*)game_state;

            return game_state;
        }

        return nullptr;
    }

    bool is_in_game() {
        if (!IsWindowVisible(hwnd) && !open_target_process())
            return false;   // Path of Exile is not running!

        get_active_game_state();
        return in_game_state != nullptr;
    }

    bool open_target_process() {
        process_id = get_process_by_name(executable_name);
        process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, process_id);
        if (process_handle) {
            HMODULE module;
            DWORD size;

            hwnd = get_hwnd();
            if (EnumProcessModules(process_handle, &module, sizeof(module), &size)) {
                MODULEINFO module_info;
                GetModuleInformation(process_handle, module, &module_info, sizeof(MODULEINFO));
                address = (addrtype)module_info.lpBaseOfDll;
                size_of_image = module_info.SizeOfImage;
            }
            game_state_controller = get_game_state_controller();
            active_game_state = get_active_game_state();

            return true;
        }

        return false;
    }

    Point get_position(Entity *entity) {
        if (!in_game_state || !entity || !entity->has_component("Render"))
            return Point();

        Vector3 vec = entity->get_component<Render>()->get_position();
        in_game_state->transform(vec);

        return Point(vec.x, vec.y);
    }

    void list_game_states() {
        GameState *active_game_state = get_active_game_state();

        if (!active_game_state)
            return;

        wprintf(L"%llx: GameState Controller\n", game_state_controller->address);
        for (auto  i : game_state_controller->get_all_game_states()) {
            printf("    %llx => %c", i.second.address, (*active_game_state == i.second) ? '*' : ' ');
            wprintf(L"%S\n", i.first.c_str());
        }
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

            return buffer[5];
        }

        return -1;
    }
};
