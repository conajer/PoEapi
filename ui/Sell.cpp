/*
* Sell.cpp, 9/28/2020 6:58 PM
*/

class Sell : public Element {
private:

    AhkObjRef* __get_items() {
        AhkObj temp_items;
        for (auto& i : get_items())
            temp_items.__set(L"", (AhkObjRef*)*i, AhkObject, nullptr);
        __set(L"items", (AhkObjRef*)temp_items, AhkObject, nullptr);

        return temp_items;
    }

    AhkObjRef* __get_your_items() {
        AhkObj temp_items;
        for (auto& i : get_your_items())
            temp_items.__set(L"", (AhkObjRef*)*i, AhkObject, nullptr);
        __set(L"yourItems", (AhkObjRef*)temp_items, AhkObject, nullptr);

        return temp_items;
    }

public:

    shared_ptr<Element> sell_panel;
    std::vector<int> path;
    std::vector<shared_ptr<Item>> items, your_items;

    Sell(addrtype address) : Element(address) {
        path.push_back(3);
        add_method(L"isOpened", (Element*)this, (MethodType)&Sell::is_opened, AhkBool);
        add_method(L"getSellPanel", this, (MethodType)&Sell::get_sell_panel, AhkVoid);
        add_method(L"getItems", this, (MethodType)&Sell::__get_items, AhkObject);
        add_method(L"getYourItems", this, (MethodType)&Sell::__get_your_items, AhkObject);
        
    }

    bool is_opened() {
        get_sell_panel();
        return sell_panel ? is_visible() : false;
    }

    shared_ptr<Element>& get_sell_panel() {
        if (is_visible()) {
            if (!sell_panel || !sell_panel->is_valid()) {
                sell_panel = shared_ptr<Element>(get_child(path));
                if (sell_panel)
                    sell_panel->get_childs();
            }
        } else {
            sell_panel.reset();
        }
        __set(L"sellPanel", sell_panel ? (AhkObjRef*)*sell_panel : nullptr, AhkObject, nullptr);

        return sell_panel;
    }

    std::vector<shared_ptr<Item>>& get_items() {
        if (is_opened()) {
            auto elements = sell_panel->childs[1]->get_childs();

            std::vector<shared_ptr<Item>> new_items;
            for (int i = 1; i < elements.size(); ++i) {
                addrtype addr = PoEMemory::read<addrtype>(elements[i]->address + 0x388);
                shared_ptr<Item> item;
                for (auto& e : items) {
                    if (e->address == addr) {
                        item = e;
                        break;
                    }
                }

                if (!item)
                    item = shared_ptr<Item>(new Item(addr));
                new_items.push_back(item);
            }
            items = new_items;
        } else {
            items.clear();
        }

        return items;
    }

    std::vector<shared_ptr<Item>>& get_your_items() {
        if (is_opened()) {
            auto elements = sell_panel->childs[0]->get_childs();

            std::vector<shared_ptr<Item>> new_items;
            for (int i = 2; i < elements.size(); ++i) {
                addrtype addr = PoEMemory::read<addrtype>(elements[i]->address + 0x388);
                shared_ptr<Item> item;
                for (auto& e : your_items) {
                    if (e->address == addr) {
                        item = e;
                        break;
                    }
                }

                if (!item)
                    item = shared_ptr<Item>(new Item(addr));
                new_items.push_back(item);
            }
            your_items = new_items;
        } else {
            your_items.clear();
        }

        return your_items;
    }
};
