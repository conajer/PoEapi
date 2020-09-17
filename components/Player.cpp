/*
* Player.cpp, 8/8/2020 11:10 AM
*/

/* Player component offsets */

static std::map<string, int> player_component_offsets {
    {"name",  0x158},
    {"exp",    0x17c},
    {"level", 0x1a8},
};

class Player : public Component {
public:

    wstring player_name;

    Player(addrtype address) : Component(address, "Player", &player_component_offsets) {
    }

    wstring name() {
        if (player_name.empty())
            player_name = read<wstring>("name");
        return player_name;
    }

    unsigned long xp() {
        return read<unsigned long>("xp");
    }

    int level() {
        return read<byte>("level");
    }

    void to_print() {
        Component::to_print();
        wprintf(L"\t\t\t! %S, %d", name().c_str(), level());
    }
};
