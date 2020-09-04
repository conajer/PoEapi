/*
* Targetable.cpp, 8/11/2020 10:24 PM
*/

/* Targetable component offsets */

static std::map<string, int> targetable_component_offsets {
    {"is_targetable ", 0x48},
};

class Targetable : public Component {
public:

    Targetable(addrtype address)
        : Component(address, "Targetable", &targetable_component_offsets)
    {
    }

    bool is_targetable() {
        return read<byte>("is_targetable");
    }

    void to_print() {
        Component::to_print();
        printf("\t\t\t! %s", is_targetable() ? "True" : "False");
    }
};
