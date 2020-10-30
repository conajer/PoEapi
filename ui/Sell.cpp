/*
* Sell.cpp, 9/28/2020 6:58 PM
*/

class Sell : public Element {
public:

    shared_ptr<Element> sell_panel;
    std::vector<int> path;
    std::vector<shared_ptr<Item>> items, your_items;

    Sell(addrtype address) : Element(address) {
        path.push_back(3);
        add_method(L"isOpened", (Element*)this, (MethodType)&Element::is_visible, AhkBool);
        add_method(L"getSellPanel", this, (MethodType)&Sell::get_sell_panel, AhkVoid);
        add_method(L"__getItems", this, (MethodType)&Sell::get_items);
        add_method(L"__getYourItems", this, (MethodType)&Sell::get_your_items);
        
    }

    shared_ptr<Element>& get_sell_panel() {
        if (is_visible()) {
            if (!sell_panel || !sell_panel->is_valid()) {
                sell_panel = shared_ptr<Element>(get_child(path));
                __set(L"sellPanel", (AhkObjRef*)*sell_panel, AhkObject, nullptr);
                sell_panel->get_childs();
            }
        } else {
            sell_panel.reset();
            __set(L"sellPanel", nullptr, AhkObject, nullptr);
        }

        return sell_panel;
    }

    std::vector<shared_ptr<Item>>& get_items() {
        if (is_visible()) {
            get_sell_panel();
            auto elements = sell_panel->childs[1]->get_childs();

            AhkObj temp_items;
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
                temp_items.__set(L"", (AhkObjRef*)*item, AhkObject, nullptr);
            }

            items = new_items;
            __set(L"items", (AhkObjRef*)temp_items, AhkObject, nullptr);
        } else {
            items.clear();
            __set(L"items", nullptr, AhkObject, nullptr);
        }

        return items;
    }

    std::vector<shared_ptr<Item>>& get_your_items() {
        if (is_visible()) {
            get_sell_panel();
            auto elements = sell_panel->childs[0]->get_childs();

            AhkObj temp_items;
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
                temp_items.__set(L"", (AhkObjRef*)*item, AhkObject, nullptr);
            }

            your_items = new_items;
            __set(L"yourItems", (AhkObjRef*)temp_items, AhkObject, nullptr);
        } else {
            your_items.clear();
            __set(L"yourItems", nullptr, AhkObject, nullptr);
        }

        return your_items;
    }
};
