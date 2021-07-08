/*
*  PoE.cpp, 8/4/2020 9:11 PM
*/

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>
#include <psapi.h>

#include <iostream>
#include <map>
#include <memory>
#include <queue>

using namespace std;

typedef unsigned __int64 addrtype;

#include "ahkpp"
#include "Canvas.cpp"
#include "PoEMemory.cpp"
#include "RemoteMemoryObject.cpp"

class PoE : public PoEMemory, public AhkObj {
protected:

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
            HWND hwnd = FindWindow("POEWindowClass", "Path of Exile");
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
        byte pattern[256];
        byte mask[256];
        int len = parse_pattern(pattern_str, pattern, mask);

        addrtype end_address = address + size_of_image;
        byte* buffer = new byte[size_of_image];
        
        if (read(address, buffer, size_of_image)) {
            for (int i = 0; i < size_of_image - len; i++) {
                if (compare(pattern, &buffer[i], mask, len)) {
                    delete[] buffer;
                    return address + i;
                }
            }
        }
        delete[] buffer;

        return 0;
    }

public:

    const char* executable_names[2] = {"PathOfExile.exe", "PathOfExile_x64Steam.exe"};
    addrtype address;
    int size_of_image;
    int process_id;
    HWND hwnd;
    GameStateController* game_state_controller;
    GameState *active_game_state;
    InGameState* in_game_state;
    InGameUI* in_game_ui;
    InGameData* in_game_data;
    ServerData* server_data;
    LocalPlayer *local_player;
    bool is_ready = false;
    unique_ptr<Canvas> hud;

    PoE() : game_state_controller(0) {
    }

    GameStateController* get_game_state_controller() {
        char pattern[] = "48 8b f1 33 ed 48 39 2d";
        addrtype addr = find_pattern(pattern);
        if (!addr)
            return nullptr;

        addr = read<addrtype>(addr + read<int>(addr + 8) + 12);
        if (!addr)
            return nullptr;
        delete game_state_controller;

        return new GameStateController(addr);
    }

    GameState* get_active_game_state() {
        if (game_state_controller) {
            GameState *game_state = game_state_controller->get_active_game_state();
            return game_state;
        }

        return nullptr;
    }

    void reset() {
        if (is_in_game()) {
            in_game_ui = in_game_state->in_game_ui();
            in_game_data = in_game_state->in_game_data();
            server_data = in_game_state->server_data();
            if (!in_game_ui || !in_game_data || !server_data)
                return;

            local_player = in_game_data->local_player();
            if (!local_player)
                return;

            is_ready = true;
        }
    }

    bool is_in_game() {
        if (!IsWindowVisible(hwnd) && !open_target_process())
            return false;   // Path of Exile is not running!

        GameState* game_state = get_active_game_state();
        if (game_state && game_state->is(L"InGameState")) {
            in_game_state = (InGameState*)game_state;
            return true;
        }

        return false;
    }

    bool open_target_process() {
        for (auto i : executable_names) {
            if (process_id = get_process_by_name(i))
                break;
        }

        process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, process_id);
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

        // Reset hwnd
        hwnd = 0;

        return false;
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

    Point get_pos(Entity* entity) {
        if (in_game_state && entity) {
            Render* render = entity->get_component<Render>();
            if (render) {
                Vector3 vec = render->position();
                in_game_state->transform(vec);

                return {(int)vec.x, (int)vec.y};
            }
        }

        return {0, 0};
    }
    
    void mouse_click(Point pos) {
        ClientToScreen(hwnd, (LPPOINT)&pos);
        SetCursorPos (pos.x, pos.y);
        Sleep(30);

        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }

    void mouse_move(Point pos) {
        ClientToScreen(hwnd, (LPPOINT)&pos);
        SetCursorPos (pos.x, pos.y);
        Sleep(30);
    }

    void bind_hud(HWND hwnd) {
        if (hwnd && IsWindow(hwnd)) {
            if (!hud)
                hud = unique_ptr<Canvas>(new Canvas());
            hud->bind(hwnd);
        }
    }

    void logout() {
        HANDLE token;
        LUID luid;
        PMIB_TCPTABLE_OWNER_PID  tcp_table;
        DWORD size = 0;

        OpenProcessToken(process_handle, TOKEN_ADJUST_PRIVILEGES , &token);
        LookupPrivilegeValue(0, "SeDebugPrivilege", &luid);
        TOKEN_PRIVILEGES token_privileges = {1, {luid, SE_PRIVILEGE_REMOVED}};
        AdjustTokenPrivileges(token, false, &token_privileges, 0, 0, 0);
        CloseHandle(token);

        GetExtendedTcpTable(tcp_table, &size, 0, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
        tcp_table = (PMIB_TCPTABLE_OWNER_PID)malloc(size);
        GetExtendedTcpTable(tcp_table, &size, 0, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
        if (tcp_table->dwNumEntries > 0) {
            for (int i = 0; i < tcp_table->dwNumEntries; ++i) {
                if (tcp_table->table[i].dwOwningPid == process_id) {
                    tcp_table->table[i].dwState = MIB_TCP_STATE_DELETE_TCB;
                    SetTcpEntry((PMIB_TCPROW)&tcp_table->table[i]);
                }
            }
        }
        free(tcp_table);
    }
};
