/*
* Quality.cpp, 8/10/2020 11:34 PM
*/

/* Quality component offsets */

static std::map<string, int> quality_component_offsets {
    {"quality", 0x18},
};

class Quality : public Component {
public:

    Quality(addrtype address) : Component(address, "Quality", &quality_component_offsets) {
    }

    int quality() {
        return read<byte>("quality");
    }

    void to_print() {
        Component::to_print();
        if (quality())
            printf("\t\t\t! %d%%", quality());
    }
};
