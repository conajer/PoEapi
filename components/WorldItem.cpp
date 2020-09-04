/*
* WorldItem.cpp, 8/11/2020 11:27 AM
*/

/* WorldItem component offsets */

static std::map<string, int> worlditem_component_offsets {
    {"item", 0x28},
};

class WorldItem : public Component {
public:

    WorldItem(addrtype address) : Component(address, "WorldItem", &worlditem_component_offsets) {
    }

    addrtype item() {
        return read<addrtype>("item");
    }

    void to_print() {
        Component::to_print();
        wprintf(L"\t\t\t! %llx", item());
    }
};
