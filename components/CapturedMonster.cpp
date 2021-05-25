/*
* CapturedMonster.cpp, 5/24/2021 8:05 PM
*/

/* CapturedMonster component offsets */

static std::map<string, int> captured_monster_component_offsets {
    {"internal",     0x18},
        {"name",    0x104},
};

class CapturedMonster : public Component {
public:

    wstring beast_name;

    CapturedMonster(addrtype address) : Component(address, "CapturedMonster", &captured_monster_component_offsets) {
    }

    wstring& name() {
        if (beast_name.empty()) {
            addrtype internal = read<addrtype>("internal");
            beast_name = PoEMemory::read<wstring>(internal + (*offsets)["name"], 32);
        }

        return beast_name;
    }
};
