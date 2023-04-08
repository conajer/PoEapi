/*
* Chest.cpp, 8/10/2020 11:14 PM
*/

/* Chest component offsets */

static std::map<string, int> chest_component_offsets {
    {"is_opened",    0x168},
    {"is_locked",    0x169},
    {"is_strongbox", 0x1a0},
};

class Chest : public Component {
public:

    Chest(addrtype address) : Component(address, "Chest", &chest_component_offsets) {
    }

    int is_opened() {
        return read<byte>("is_opened");
    }

    bool is_locked() {
        return read<byte>("is_locked");
    }

    bool is_strongbox() {
        return read<byte>("is_strongbox");
    }

    void to_print() {
        Component::to_print();
        printf("%s%s", is_opened() ? "\t\t\t! Opened"
                                   : (is_locked() ? "\t\t\t! Locked" : ""),
               is_strongbox() ? ", Strongbox" : "");
    }
};
