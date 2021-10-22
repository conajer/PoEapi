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
#include "Hud.cpp"
#include "PoEMemory.cpp"
#include "RemoteMemoryObject.cpp"

std::map<string, int> poe_offsets {
    {"active_game_states", 0x20},
        {"current",         0x0},
    {"game_states",        0x48},
        {"root",            0x8},
};

class PoE : public RemoteMemoryObject, public AhkObj, public Hud {
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

    std::vector<shared_ptr<GameState>> get_all_game_states() {
        std::vector<shared_ptr<GameState>> game_states;
        if (address) {
            std::queue<addrtype> nods;
            addrtype addr = read<addrtype>("game_states", "root");
            nods.push(addr);
            while (!nods.empty()) {
                addr = nods.front();
                nods.pop();

                if (PoEMemory::read<byte>(addr + 0x19) != 0)
                    continue;

                nods.push(PoEMemory::read<addrtype>(addr));
                nods.push(PoEMemory::read<addrtype>(addr + 0x10));

                addr = PoEMemory::read<addrtype>(addr + 0x40);
                game_states.push_back(shared_ptr<GameState>(new GameState(addr)));
            }
        }

        return game_states;
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

        addrtype end_address = base_address + size_of_image;
        byte* buffer = new byte[size_of_image];

        if (PoEMemory::read(base_address, buffer, size_of_image)) {
            for (int i = 0; i < size_of_image - len; i++) {
                if (compare(pattern, &buffer[i], mask, len)) {
                    delete[] buffer;
                    return base_address + i;
                }
            }
        }
        delete[] buffer;

        return 0;
    }

public:

    const char* executable_names[3] = {
        "PathOfExile.exe",
        "PathOfExileSteam.exe",
        "PathOfExile_KG.exe"
    };

    addrtype base_address;
    int size_of_image;
    int process_id;
    HWND poe_hwnd;

    shared_ptr<GameState> active_game_state;
    InGameState* in_game_state;
    InGameUI* in_game_ui;
    InGameData* in_game_data;
    ServerData* server_data;
    LocalPlayer *local_player;
    bool is_ready = false;

    PoE() : RemoteMemoryObject(0, &poe_offsets) {
    }

    shared_ptr<GameState>& get_active_game_state() {
        if (addrtype addr = read<addrtype>("active_game_states", "current")) {
            if (!active_game_state || active_game_state->address != addr) {
                string state_name = PoEMemory::read<string>(addr + 0x10);
                active_game_state.reset(read_object<GameState>(state_name, addr));
            }
        } else {
            active_game_state.reset();
        }

        return active_game_state;
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
        if (!IsWindowVisible(poe_hwnd) || !get_active_game_state()) {
            open_target_process();
            return false;
        }

        if (in_game_state && in_game_state->address == active_game_state->address)
            return true;

        if (active_game_state->is(L"InGameState")) {
            in_game_state = (InGameState*)active_game_state.get();
            return true;
        }

        return false;
    }

    bool open_target_process() {
        for (auto i : executable_names) {
            if (process_id = get_process_by_name(i))
                break;
        }

        poe_hwnd = 0;
        process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, process_id);
        if (process_handle) {
            HMODULE module;
            DWORD size;

            poe_hwnd = get_hwnd();
            if (EnumProcessModules(process_handle, &module, sizeof(module), &size)) {
                MODULEINFO module_info;
                GetModuleInformation(process_handle, module, &module_info, sizeof(MODULEINFO));
                base_address = (addrtype)module_info.lpBaseOfDll;
                size_of_image = module_info.SizeOfImage;
            }

            if (addrtype addr = find_pattern("48 8b f1 33 ed 48 39 2d")) {
                address = PoEMemory::read<addrtype>(addr + PoEMemory::read<int>(addr + 8) + 12);
                CloseHandle(process_handle);
                process_handle = OpenProcess(PROCESS_VM_READ, false, process_id);
                bind(poe_hwnd);
                return true;
            }
        }

        return false;
    }

    void list_game_states() {
        auto game_states = get_all_game_states();
        if (!game_states.empty()) {
            wprintf(L"%llx: Game states\n", read<addrtype>("game_states"));
            for (auto&  i : game_states)
                wprintf(L"    %llx => %S\n", i->address, i->name.c_str());
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
        ClientToScreen(poe_hwnd, (LPPOINT)&pos);
        SetCursorPos (pos.x, pos.y);
        Sleep(30);

        mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
        mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
    }

    void mouse_move(Point pos) {
        ClientToScreen(poe_hwnd, (LPPOINT)&pos);
        SetCursorPos (pos.x, pos.y);
        Sleep(30);
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
