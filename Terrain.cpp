/*
* Terrain.cpp, 1/14/2021 1:29 AM
*/

#include <cmath>
#include <algorithm>

static std::map<string, int> terrain_offsets {
    {"cols",           0x18},
    {"rows",           0x20},
    {"melee_layer",    0xb0},
    {"ranged_layer",   0xc8},
    {"bytes_per_row",  0xe0},
};

class MapData : public AhkObj {
public:

    int width, height;
    shared_ptr<byte> data;

    MapData(int w, int h) : width(w), height(h) {
        size_t size = width * height;
        byte* buffer = new byte[size];
        std::fill(buffer, buffer + size - 1, 0);
        data = shared_ptr<byte>(buffer);
    }

    void __new() {
        __set(L"width", width, AhkInt,
              L"height", height, AhkInt,
              L"data", data.get(), AhkPointer,
              nullptr);
    }
};

class Terrain : public PoEObject {
public:

    int rows, cols;
    int bytes_per_row;
    shared_ptr<byte> melee_layer_data;
    shared_ptr<byte> ranged_layer_data;
    shared_ptr<MapData> map_data;

    Terrain(addrtype address) : PoEObject(address, &terrain_offsets) {
        cols = read<int>("cols");
        rows = read<int>("rows");
        bytes_per_row = read<int>("bytes_per_row");

        add_method(L"getMeleeLayerData", this, (MethodType)&Terrain::get_melee_layer_data, AhkPointer);
        add_method(L"getRangedLayerData", this, (MethodType)&Terrain::get_ranged_layer_data, AhkPointer);
        add_method(L"getMapData", this, (MethodType)&Terrain::get_map_data, AhkObject, ParamList{AhkInt, AhkInt});
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

    AhkObjRef* get_map_data(int w, int h) {
        byte* layer_data = get_melee_layer_data();
        int r, c;
        int x1 = (rows - 1) * 23, y1 = (cols / 2 - 1) * 23;
        int x2 = 0, y2 = 0;

        for (r = 0; r < (rows - 1) * 23; ++r)
            for (c = 0; c < (cols - 1) * 23 / 2; ++c)
                if (layer_data[r * bytes_per_row + c]) {
                    if (r < y1) y1 = r;
                    if (r > y2) y2 = r;
                    break;
                }

        for (c = 0; c < (cols - 1) * 23 / 2; ++c)
            for (r = 0; r < (rows - 1) * 23; ++r)
                if (layer_data[r * bytes_per_row + c]) {
                    if (c < x1) x1 = c;
                    if (c > x2) x2 = c;
                    break;
                }

        float scale = std::min(w * 1.0f / ((x2 - x1) * 2), h * 1.0f / (y2 - y1));
        scale = std::min(scale, 1.0f);
        w = round((x2 - x1) * 2 * scale);
        h = round((y2 - y1) * scale);
        map_data = shared_ptr<MapData>(new MapData(w, h));
        byte* data = map_data->data.get();
        for (r = y1; r < y2; ++r)
            for (c = x1; c < x2; ++c) {
                int x = round((c - x1) * 2 * scale);
                int y = round((r - y1) * scale);
                int d = layer_data[r * bytes_per_row + c];
                if ((d & 0xf) < 3)
                    data[y * w + x] |= d;
                if ((d >> 4) < 3)
                    data[y * w + x + 1] |= d;
            }

        return *map_data;
    }
};
