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
                addrtype addr = PoEMemory::read<addrtype>(elements[i]->address + 0x390);
                if (addr)
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
                addrtype addr = PoEMemory::read<addrtype>(elements[i]->address + 0x390);
                if (addr)
                    your_items.push_back(shared_ptr<Item>(new Item(addr)));
            }
        }

        return your_items;
    }
};
