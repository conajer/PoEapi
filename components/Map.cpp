/*
* Map.cpp, 8/10/2020 11:32 PM
*/

/* Map component offsets */

static std::map<string, int> map_component_offsets {
    {"tier", 0x18},
};

class Map : public Component {
public:

    Map(addrtype address) : Component(address, "Map", &map_component_offsets) {
    }

    int tier() {
        return read<byte>("tier");
    }

    void to_print() {
        Component::to_print();
        printf("\t\t\t! T%d", tier());
    }
};
