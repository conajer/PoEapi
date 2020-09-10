/*
* TriggerableBlockage.cpp, 9/9/2020 4:24 PM
*/

/* TriggerableBlockage component offsets */

static std::map<string, int> triggerable_blockage_component_offsets {
    {"is_closed", 0x30},
};

class TriggerableBlockage : public Component {
public:

    TriggerableBlockage(addrtype address)
        : Component(address, "TriggerableBlockage", &triggerable_blockage_component_offsets)
    {
    }

    bool is_closed() {
        return read<byte>("is_closed");
    }

    void to_print() {
        Component::to_print();
        printf("\t\t\t! %s", is_closed() ? "Closed" : "Opened");
    }
};
