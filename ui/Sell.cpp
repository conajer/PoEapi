/*
* Sell.cpp, 9/28/2020 6:58 PM
*/

class Sell : public Element {
public:

    shared_ptr<Element> sell_panel;
    std::vector<shared_ptr<Item>> items, your_items;

    Sell(addrtype address) : Element(address) {
        add_method(L"isOpened", (Element*)this, (MethodType)&Element::is_visible, AhkBool);
        add_method(L"__getItems", this, (MethodType)&Sell::get_items);
        add_method(L"__getYourItems", this, (MethodType)&Sell::get_your_items);
    }

    shared_ptr<Element>& get_sell_panel() {
        if (is_visible()) {
            sell_panel = shared_ptr<Element>(get_child(3));
            __set(L"sellPanel", (AhkObjRef*)*sell_panel, AhkObject, nullptr);
            sell_panel->get_childs();
        } else {
            sell_panel.reset();
        }

        return sell_panel;
    }

    std::vector<shared_ptr<Item>>& get_items() {
        items.clear();
        if (is_visible()) {
            auto elements = sell_panel->childs[1]->get_childs();

            AhkObj items;
            for (int i = 1; i < elements.size(); ++i) {
                addrtype addr = PoEMemory::read<addrtype>(elements[i]->address + 0x388);
                shared_ptr<Item> item = shared_ptr<Item>(new Item(addr));
                this->items.push_back(item);
                items.__set(L"", (AhkObjRef*)*item, AhkObject, nullptr);
            }
            __set(L"items", (AhkObjRef*)items, AhkObject, nullptr);
        } else {
            __set(L"items", nullptr, AhkObject, nullptr);
        }

        return items;
    }

    std::vector<shared_ptr<Item>>& get_your_items() {
        your_items.clear();
        if (is_visible()) {
            auto elements = sell_panel->childs[0]->get_childs();

            AhkObj items;
            for (int i = 2; i < elements.size(); ++i) {
                addrtype addr = PoEMemory::read<addrtype>(elements[i]->address + 0x388);
                shared_ptr<Item> item = shared_ptr<Item>(new Item(addr));
                your_items.push_back(item);
                items.__set(L"", (AhkObjRef*)*item, AhkObject, nullptr);
            }
            __set(L"yourItems", (AhkObjRef*)items, AhkObject, nullptr);
        } else {
            __set(L"yourItems", nullptr, AhkObject, nullptr);
        }

        return your_items;
    }
};
