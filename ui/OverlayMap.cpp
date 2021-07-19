/*
* OverlayMap.cpp, 9/28/2020 6:58 PM
*/

#include <math.h>

static std::map<string, int> overlay_map_offsets {
    {"shift_x",    0x290},
    {"shift_y",    0x294},
    {"zoom",       0x2d4},
};

class OverlayMap : public Element {
public:

    float shift_modifier = 0.0;

    OverlayMap(addrtype address) : Element(address, &overlay_map_offsets) {
        add_method(L"shiftX", this, (MethodType)&OverlayMap::shift_x, AhkFloat);
        add_method(L"shiftY", this, (MethodType)&OverlayMap::shift_y, AhkFloat);
        add_method(L"zoom", this, (MethodType)&OverlayMap::zoom, AhkFloat);
    }

    float shift_x() {
        return read<float>("shift_x");
    }

    float shift_y() {
        return read<float>("shift_y") + shift_modifier;
    }

    float zoom() {
        return read<float>("zoom");
    }
};
