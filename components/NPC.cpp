/*
* NPC.cpp, 8/11/2020 11:04 AM
*/

/* NPC component offsets */

static std::map<string, int> npc_component_offsets {
    {"internal",           0x18},
        {"base",           0x10},
            {"name",        0x8},
            {"short_name", 0x2c},
            {"act",        0x34},
};

class NPC : public Component {
protected:

    wstring npc_name;

public:

    NPC(addrtype address) : Component(address, "NPC", &npc_component_offsets) {
    }

    wstring& name() {
        if (npc_name.empty()) {
            addrtype base = read<addrtype>("internal", "base");
            npc_name = PoEMemory::read<wstring>(base + (*offsets)["short_name"], 16);
            if (npc_name.empty())
                npc_name = PoEMemory::read<wstring>(base + (*offsets)["name"], 16);
        }

        return npc_name;
    }

    int act() {
        return PoEMemory::read<int>(read<addrtype>("internal", "base") + (*offsets)["act"]);
    }

    void to_print() {
        Component::to_print();
        wprintf(L"\t\t\t! %S", name().c_str());
    }
};
