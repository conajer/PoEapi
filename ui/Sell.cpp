/*
* Sell.cpp, 9/28/2020 6:58 PM
*/

class Sell : public Element {
private:

    AhkObjRef* __get_items() {
        AhkObj temp_items;

        items.clear();
        auto& elements = get_child(topIndex)->get_childs();
        for (int i = 1; i < elements.size(); ++i) {
            if (addrtype addr = elements[i]->read<addrtype>("item")) {
                Item* item = new Item(addr);
                Rect r = elements[i]->get_rect();
                item->__set(L"x", r.x, AhkInt,
                            L"y", r.y, AhkInt,
                            L"w", r.w, AhkInt,
                            L"h", r.h, AhkInt,
                            nullptr);
                temp_items.__set(L"", (AhkObjRef*)*item, AhkObject, nullptr);
                items.push_back(shared_ptr<Item>(item));
            }
        }
        __set(L"items", (AhkObjRef*)temp_items, AhkObject, nullptr);

        return temp_items;
    }

    AhkObjRef* __get_your_items() {
        AhkObj temp_items;

        your_items.clear();
        auto& elements = get_child(bottomIndex)->get_childs();
        for (int i = 2; i < elements.size(); ++i) {
            if (addrtype addr = elements[i]->read<addrtype>("item")) {
                Item* item = new Item(addr);
                Rect r = elements[i]->get_rect();
                item->__set(L"x", r.x, AhkInt,
                            L"y", r.y, AhkInt,
                            L"w", r.w, AhkInt,
                            L"h", r.h, AhkInt,
                            nullptr);
                temp_items.__set(L"", (AhkObjRef*)*item, AhkObject, nullptr);
                your_items.push_back(shared_ptr<Item>(item));
            }
        }
        __set(L"yourItems", (AhkObjRef*)temp_items, AhkObject, nullptr);

        return temp_items;
    }

public:

    std::vector<shared_ptr<Item>> items, your_items;
    int topIndex = 0, bottomIndex = 1;

    Sell(addrtype address, bool reverse = false) : Element(address) {
        if (reverse)
            topIndex = 1, bottomIndex = 0;

        add_method(L"isOpened", (Element*)this, (MethodType)&Element::is_visible, AhkBool);
        add_method(L"getItems", this, (MethodType)&Sell::__get_items, AhkObject);
        add_method(L"getYourItems", this, (MethodType)&Sell::__get_your_items, AhkObject);
    }

    std::vector<shared_ptr<Item>>& get_items() {
        items.clear();
        if (is_visible()) {
            auto& elements = get_child(topIndex)->get_childs();
            for (int i = 1; i < elements.size(); ++i) {
                if (addrtype addr = elements[i]->read<addrtype>("item"))
                    items.push_back(shared_ptr<Item>(new Item(addr)));
            }
        }

        return items;
    }

    std::vector<shared_ptr<Item>>& get_your_items() {
        your_items.clear();
        if (is_visible()) {
            auto& elements = get_child(bottomIndex)->get_childs();
            for (int i = 2; i < elements.size(); ++i) {
                if (addrtype addr = elements[i]->read<addrtype>("item"))
                    your_items.push_back(shared_ptr<Item>(new Item(addr)));
            }
        }

        return your_items;
    }
};
