/*
* MinimapIcon.cpp, 8/11/2020 11:32 AM
*/

/* MinimapIcon component offsets */

static std::map<string, int> minimapicon_component_offsets {
    {"base",    0x28},
        {"name", 0x0},
};

class MinimapIcon : public Component {
protected:

    wstring base_name;

public:

    MinimapIcon(addrtype address) : Component(address, "MinimapIcon", &minimapicon_component_offsets) {
    }

    wstring& name() {
        if (base_name.empty())
            base_name = PoEMemory::read<wstring>(read<addrtype>("base"), 32);
        return base_name;
    }

    void to_print() {
        Component::to_print();
        wprintf(L"\t\t! %S", name().c_str());
    }
};
