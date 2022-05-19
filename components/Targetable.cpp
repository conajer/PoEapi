/*
* Targetable.cpp, 8/11/2020 10:24 PM
*/

/* Targetable component offsets */

static std::map<string, int> targetable_component_offsets {
    {"is_targetable",       0x48},
    {"is_highlightable",    0x49},
    {"is_targeted",         0x4a},
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

    bool is_highlightable() {
        return read<byte>("is_highlightable");
    }

    bool is_targeted() {
        return read<byte>("is_targeted");
    }

    void to_print() {
        Component::to_print();
        printf("\t\t\t! %s", is_targetable() ? "True" : "False");
    }
};
