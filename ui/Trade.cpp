/*
* Trade.cpp, 10/22/2020 11:55 AM
*/

class Trade : public Sell {
public:

    Trade(addrtype address) : Sell(address) {
    }

    shared_ptr<Element>& get_sell_panel() {
        if (is_visible()) {
            sell_panel = get_child(std::vector<int>{3, 1, 0, 0});
            sell_panel->get_childs();
            __set(L"sellPanel", (AhkObjRef*)*sell_panel, AhkObject, nullptr);
            __set(L"accept", (AhkObjRef*)*sell_panel->childs[5], AhkObject, nullptr);
            __set(L"cancel", (AhkObjRef*)*sell_panel->childs[6], AhkObject, nullptr);
        } else {
            sell_panel.reset();
        }

        return sell_panel;
    }
};
