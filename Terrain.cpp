/*
* Terrain.cpp, 1/14/2021 1:29 AM
*/

static std::map<string, int> terrain_offsets {
    {"cols",           0x18},
    {"rows",           0x20},
    {"melee_layer",    0xb0},
    {"ranged_layer",   0xc8},
    {"bytes_per_row",  0xe0},
};

class Terrain : public PoEObject {
public:

    int rows, cols;
    int bytes_per_row;
    shared_ptr<byte> melee_layer_data;
    shared_ptr<byte> ranged_layer_data;

    Terrain(addrtype address) : PoEObject(address, &terrain_offsets) {
        cols = read<int>("cols");
        rows = read<int>("rows");
        bytes_per_row = read<int>("bytes_per_row");

        add_method(L"getMeleeLayerData", this, (MethodType)&Terrain::get_melee_layer_data, AhkPointer);
        add_method(L"getRangedLayerData", this, (MethodType)&Terrain::get_ranged_layer_data, AhkPointer);
    }

    void __new() {
        PoEObject::__new();
        __set(L"rows", (rows - 1) * 23, AhkInt,
              L"cols", (cols - 1) * 23, AhkInt,
              L"bytesPerRow", bytes_per_row, AhkInt,
              nullptr);
    }

    byte* get_melee_layer_data() {
        if (!melee_layer_data) {
            int size = (rows - 1) * 23 * bytes_per_row;
            byte* buffer = new byte[size];
            if (read<byte>("melee_layer", buffer, size))
                melee_layer_data = shared_ptr<byte>(buffer);
        }

        return melee_layer_data.get();
    }

    byte* get_ranged_layer_data() {
        if (!ranged_layer_data) {
            int size = (rows - 1) * 23 * bytes_per_row;
            byte* buffer = new byte[size];
            if (read<byte>("ranged_layer", buffer, size))
                ranged_layer_data = shared_ptr<byte>(buffer);
        }

        return ranged_layer_data.get();
    }
};
