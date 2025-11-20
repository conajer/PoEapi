/*
* Stash.c, 9/17/2020 6:22 PM
*/

static std::map<string, int> stash_offsets {
    {"tabs",                 0x310},
        {"active_tab_index", 0x288},
};

class Stash : public Element {
public:

    Stash(addrtype address) : Element(address, &stash_offsets) {
        add_method(L"isOpened", (Element*)this, (MethodType)&Element::is_visible, AhkBool);
        add_method(L"activeTabIndex", this, (MethodType)&Stash::active_tab_index);
        add_method(L"hasTab", this, (MethodType)&Stash::has_tab, AhkBool, ParamList{AhkWString});
    }

    int active_tab_index() {
        return read<int>("tabs", "active_tab_index");
    }

    bool has_tab(const wchar_t* name) {
        shared_ptr<Element> tabs = get_child(std::vector<int>{0, 1, 0});
        for (auto& i : tabs->get_childs()) {
            shared_ptr<Element> e = i->get_child(std::vector<int>{0, 1});
            if (e) {
                wstring& tab_name = e->get_text();
                if (!wcsicmp(tab_name.c_str(), name))
                    return true;
            }
        }

        return false;
    }
};
