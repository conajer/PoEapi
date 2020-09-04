/*
* GameState.cpp, 8/8/2020 12:04 PM
*/

#include <queue> 

static std::map<string, int> game_state_offsets {
    {"name", 0x10},
};

class GameState : public RemoteMemoryObject {
public:

    wstring name;

    GameState(addrtype address, FieldOffsets* offsets = &game_state_offsets)
        : RemoteMemoryObject(address, offsets)
    {
        name = read<wstring>("name");
    }

    bool is(wstring name) {
        return !name.compare(this->name);
    }
};

static std::map<string, int> game_state_controller_offsets {
    {"active_game_states", 0x20},
        {"current",         0x0},
    {"game_states",        0x48},
        {"root",            0x8},
};

class GameStateController : public RemoteMemoryObject {
public:

    std::map<wstring, GameState> all_game_states;
    GameState* active_game_state;

    GameStateController(addrtype address)
        : RemoteMemoryObject(address, &game_state_controller_offsets),
          active_game_state(nullptr)
    {
    }

    GameState* get_active_game_state() {
        addrtype addr = read<addrtype>("active_game_states", "current");
        if (!addr)              /* no active game states */
            return nullptr;

        if (!active_game_state || active_game_state->address != addr) {
            delete active_game_state;
            string state_name = PoEMemory::read<string>(addr + 0x10);
            if (state_name == "InGameState") {
                int time_in_game = PoEMemory::read<int>(addr + 0x6d4);
                while (time_in_game != PoEMemory::read<int>(addr + 0x6d4))
                    Sleep(50);
            }

            active_game_state = read_object<GameState>(state_name, addr);
        }

        return active_game_state;
    }

    std::map<wstring, GameState>& get_all_game_states() {
        all_game_states.clear();

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

            wstring state_name = PoEMemory::read<wstring>(addr + 0x20, 32);
            addr = PoEMemory::read<addrtype>(addr + 0x40);
            all_game_states.insert(std::make_pair(state_name, addr));
        }

        return all_game_states;
    }
};

struct Matrix4x4 {
    float M[4][4];

    float* operator[] (int index) {
        return M[index];
    }
};

static std::map<string, int> in_game_state_offsets {
    {"name",          0x10},
    {"in_game_ui",    0x80},
    {"in_game_data", 0x4f8},
    {"server_data",  0x500},
    {"ui_root",      0x628},
    {"time_in_game", 0x628},
    {"camera",      0x1160},
    {"width",       0x1168},
    {"height",      0x116c},
    {"matrix",      0x1208},
};

class InGameState : public GameState {
public:

    InGameUI* igu;
    InGameData* igd;
    ServerData* sd;

    float center_x;
    float center_y;

    InGameState(addrtype address) : GameState(address, &in_game_state_offsets)
    {
        igu = 0;
        igd = 0;
        sd = 0;
        center_x = read<int>("width") / 2;
        center_y = read<int>("height") / 2;
    }

    InGameUI* in_game_ui() {
        addrtype addr = read<addrtype>("in_game_ui");
        if (!igu || igu->address != addr) {
            delete igu;
            igu = new InGameUI(addr);
        }

        return igu;
    }

    InGameData* in_game_data() {
        addrtype addr = read<addrtype>("in_game_data");
        if (!igd || igd->address != addr) {
            delete igd;
            igd = new InGameData(addr);
        }

        return igd;
    }

    ServerData* server_data() {
        addrtype addr = read<addrtype>("server_data");
        if (!sd || sd->address != addr) {
            delete sd;
            sd = new ServerData(addr);
        }

        return sd;
    }

    Vector3& transform(Vector3& vec) {
        Matrix4x4 mat = read<Matrix4x4>("matrix");
        float x = vec.x * mat[0][0] + vec.y * mat[1][0] + vec.z * mat[2][0] + mat[3][0];
        float y = vec.x * mat[0][1] + vec.y * mat[1][1] + vec.z * mat[2][1] + mat[3][1];
        float z = vec.x * mat[0][2] + vec.y * mat[1][2] + vec.z * mat[2][2] + mat[3][2];
        float w = vec.x * mat[0][3] + vec.y * mat[1][3] + vec.z * mat[2][3] + mat[3][3];

        vec.x = (1.0 + x / w) * center_x;
        vec.y = (1.0 - y / w) * center_y;
        vec.z = z / w;

        return vec;
    }
};
