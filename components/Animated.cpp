/*
* Animated.cpp, 9/26/2022 6:14 PM
*/

/* Animated component offsets */

static FieldOffsets animated_component_offsets {
    {"entity",  0x1e8},
};

class Animated : public Component {
public:

    wstring path;

    Animated(addrtype address) : Component(address, "Animated", &animated_component_offsets) {
    }

    wstring& get_path() {
        if (path.empty()) {
            addrtype entity = read<addrtype>("entity");
            path = PoEMemory::read<wstring>(entity, {0x8, 0x8});
        }

        return path;
    }
};
