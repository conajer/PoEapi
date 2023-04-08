/*
* PlayerClass.cpp, 1/4/2021 10:23 PM
*/

/* PlayerClass component offsets */

static std::map<string, int> player_class_component_offsets {
    {"class", 0x158},
};

static const wchar_t* player_classes[] = {
    L"Scion", L"Ascendant", L"", L"",
    L"Marauder", L"Juggernaut", L"Berserker", L"Chieftain",
    L"Ranger", L"Raider", L"Deadeye", L"Pathfinder",
    L"Witch", L"Occultist", L"Elementalist", L"Necromancer",
    L"Duelist", L"Slayer", L"Gladiator", L"Champion",
    L"Templar", L"Inquisitor", L"Hierophant", L"Guardian",
    L"Shadow", L"Assassin", L"Trickster", L"Saboteur",
};

class PlayerClass : public Component {
public:

    PlayerClass(addrtype address) : Component(address, "PlayerClass", &player_class_component_offsets) {
    }

    const wchar_t* name() {
        int c = read<byte>("class");
        return player_classes[((c & 0xf) << 2) + (c >> 6)];
    }
};
