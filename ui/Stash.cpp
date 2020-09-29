/*
* Stash.c, 9/17/2020 6:22 PM
*/

static std::map<string, int> stash_offsets {
    {"tabs",             0x438},
    {"active_tab_index", 0x490},
};

class Stash : public Element {
public:

    std::vector<shared_ptr<Element>> tabs;

    Stash(addrtype address) : Element(address, &stash_offsets) {
        Element all_tabs(read<addrtype>("tabs"));
        tabs = all_tabs.get_childs();

        add_method(L"activeTabIndex", this, (MethodType)&Stash::active_tab_index);
    }

    void __new() {
        AhkObjRef* ahkobj_ref;

        Element::__new();
        __set(L"tabs", nullptr, AhkObject, nullptr);
        __get(L"tabs", &ahkobj_ref, AhkObject);
        AhkObj __tabs(ahkobj_ref);
        for (auto& i : tabs)
            __tabs.__set(L"", (AhkObjRef*)*i, AhkObject, nullptr); 
    }

    int active_tab_index() {
        return read<int>("active_tab_index");
    }
};
