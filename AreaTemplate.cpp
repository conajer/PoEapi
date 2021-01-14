/*
* AreaTemplate.cpp, 8/17/2020 6:07 PM
*/

static std::map<string, int> area_template_offsets {
    {"template_id",   0x0},
    {"name",          0x8},
    {"act",          0x10},
    {"is_town",      0x14},
    {"has_waypoint", 0x15},
    {"level",        0x26},
    {"area_id",      0x2a},
};

class AreaTemplate : public RemoteMemoryObject {
public:

    wstring area_name;

    AreaTemplate(addrtype address) : RemoteMemoryObject(address, &area_template_offsets) {
    }

    wstring& name() {
        if (area_name.empty())
            area_name = PoEMemory::read<wstring>(address + 0x8, 32);

        return area_name;
    }

    int act() {
        return read<byte>("act");
    }

    bool is_hideout() {
        return (name().find(L"Hideout") != wstring::npos
                && name().find(L"Syndicate Hideout") == wstring::npos);
    }

    bool is_town() {
        return read<byte>("is_town");
    }

    bool has_waypoint() {
        return read<byte>("has_waypoint");
    }

    int level() {
        return read<byte>("level");
    }

    int area_id() {
        return read<byte>("area_id");
    }

    void to_print() {
        wprintf(L"%S (%d)", name().c_str(), level());
    }
};
