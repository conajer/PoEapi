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

std::map<string, int> game_state_controller_offsets {
    {"active_game_states", 0x20},
        {"current",         0x0},
    {"game_states",        0x48},
        {"root",            0x8},
};

class GameStateController : public PoEObject {
public:

    std::map<wstring, GameState> all_game_states;
    shared_ptr<GameState> active_game_state;

    GameStateController(addrtype address)
        : PoEObject(address, &game_state_controller_offsets),
          active_game_state(nullptr)
    {
    }

    GameState* get_active_game_state() {
        if (addrtype addr = read<addrtype>("active_game_states", "current")) {
            if (!active_game_state || active_game_state->address != addr) {
                string state_name = PoEMemory::read<string>(addr + 0x10);
                active_game_state.reset(read_object<GameState>(state_name, addr));
            }
        } else {
            active_game_state.reset();
        }

        return active_game_state.get();
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

std::map<string, int> in_game_state_offsets {
    {"name",          0x10},
    {"load_stage1",   0x40},
    {"in_game_ui",    0x80},
    {"in_game_data", 0x500},
    {"server_data",  0x508},
    {"ui_root",      0x630},
    {"hovered",      0x698},
    {"hovered_item", 0x6a8},
    {"time_in_game", 0x6dc},
    {"load_stage2",  0x6f0},
    {"camera",      0x1178},
    {"width",       0x1180},
    {"height",      0x1184},
    {"matrix",      0x1238},
};

class InGameState : public GameState {
public:

    unique_ptr<InGameUI> igu;
    unique_ptr<InGameData> igd;
    unique_ptr<ServerData> sd;
    int load_stage = 0;

    float width, height;
    float center_x, center_y;

    InGameState(addrtype address) : GameState(address, &in_game_state_offsets)
    {
        width = read<int>("width");
        height = read<int>("height");
        center_x = width / 2;
        center_y = height / 2;
    }

    bool is_loading() {
        int stage1 = read<short>("load_stage1");
        if (load_stage || stage1) {
            addrtype stage2 = read<addrtype>("load_stage2");
            switch (load_stage) {
                case 1:
                    if (stage2 == 0 || !stage1) {
                        Sleep(500);
                        load_stage = 2;
                    }
                    break;
                case 2:
                    if (stage2 != 0)
                        load_stage = 0;
                    break;

                default:
                    load_stage = 1;
            }
        }

        return load_stage;
    }

    InGameUI* in_game_ui() {
        addrtype addr = read<addrtype>("in_game_ui");
        if (!igu || igu->address != addr)
            igu.reset(new InGameUI(addr));

        return igu.get();
    }

    InGameData* in_game_data() {
        addrtype addr = read<addrtype>("in_game_data");
        if (!igd || igd->address != addr)
            igd.reset(new InGameData(addr));

        return igd.get();
    }

    ServerData* server_data() {
        addrtype addr = read<addrtype>("server_data");
        if (!sd || sd->address != addr)
            sd.reset(new ServerData(addr));

        return sd.get();
    }

    Element* get_hovered_element() {
        addrtype addr = read<addrtype>("hovered");
        return addr ? new Element(addr) : nullptr;
    }

    Item* get_hovered_item() {
        addrtype addr = read<addrtype>("hovered_item", 0x388);
        return addr ? new Item(addr) : nullptr;
    }

    unsigned int time_in_game() {
        return read<unsigned int>("time_in_game");
    }

    void reset() {
        igu.reset();
        igd.reset();
        sd.reset();
    }

    Vector3& transform(Vector3& vec) {
        Point size = read<Point>("width");
        Matrix4x4 mat = read<Matrix4x4>("matrix");
        float x = vec.x * mat[0][0] + vec.y * mat[1][0] + vec.z * mat[2][0] + mat[3][0];
        float y = vec.x * mat[0][1] + vec.y * mat[1][1] + vec.z * mat[2][1] + mat[3][1];
        float z = vec.x * mat[0][2] + vec.y * mat[1][2] + vec.z * mat[2][2] + mat[3][2];
        float w = vec.x * mat[0][3] + vec.y * mat[1][3] + vec.z * mat[2][3] + mat[3][3];

        vec.x = (1.0 + x / w) * size.x / 2;
        vec.y = (1.0 - y / w) * size.y / 2;
        vec.z = z / w;

        return vec;
    }
};
