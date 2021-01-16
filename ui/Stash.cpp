/*
* Stash.c, 9/17/2020 6:22 PM
*/

static std::map<string, int> stash_offsets {
    {"tabs",             0x230},
    {"active_tab_index", 0x9a0},
};

class Stash : public Element {
public:

    Stash(addrtype address) : Element(address, &stash_offsets) {
        this->address = read<addrtype>("tabs");
        add_method(L"isOpened", (Element*)this, (MethodType)&Element::is_visible, AhkBool);
        add_method(L"activeTabIndex", this, (MethodType)&Stash::active_tab_index);
    }

    int active_tab_index() {
        return read<int>("active_tab_index");
    }
};
