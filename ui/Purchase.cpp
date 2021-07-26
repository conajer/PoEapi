/*
* Purchase.cpp, 4/1/2021 5:16 PM
*/

class Purchase : public Element {
private:

    AhkObjRef* __get_childs() {
        AhkObj temp_childs;

        if (is_visible()) {
            auto& elements = get_childs();
            for (int i = 1; i < elements.size(); ++i) {
                addrtype addr = PoEMemory::read<addrtype>(elements[i]->address + 0x390);
                if (addr) {
                    int l = elements[i]->PoEMemory::read<int>(elements[i]->address + 0x398);
                    int t = elements[i]->PoEMemory::read<int>(elements[i]->address + 0x39c);
                    int index = l * 12 + t + 1;
                    items[index] = shared_ptr<Item>(new Item(addr));
                    elements[i]->__set(L"item", (AhkObjRef*)*items[index], AhkObject, nullptr);
                }
                temp_childs.__set(L"", (AhkObjRef*)*elements[i], AhkObject, nullptr);
            }
        }
        __set(L"childs", (AhkObjRef*)temp_childs, AhkObject, nullptr);

        return temp_childs;
    }

    AhkObjRef* __get_items() {
        AhkObj temp_items;
        for (auto& i : get_items())
            temp_items.__set(std::to_wstring(i.first).c_str(), (AhkObjRef*)*i.second, AhkObject, nullptr);
        __set(L"items", (AhkObjRef*)temp_items, AhkObject, nullptr);

        return temp_items;
    }

public:

    std::map<int, shared_ptr<Item>> items;

    Purchase(addrtype address) : Element(address) {
        add_method(L"isOpened", (Element*)this, (MethodType)&Element::is_visible, AhkBool);
        add_method(L"getChilds", this, (MethodType)&Purchase::__get_childs, AhkObject);
        add_method(L"getItems", this, (MethodType)&Purchase::__get_items, AhkObject);
        
    }

    std::map<int, shared_ptr<Item>>& get_items() {
        items.clear();
        if (is_visible()) {
            auto& elements = get_childs();
            for (int i = 1; i < elements.size(); ++i) {
                addrtype addr = PoEMemory::read<addrtype>(elements[i]->address + 0x390);
                if (addr) {
                    int l = elements[i]->PoEMemory::read<int>(elements[i]->address + 0x398);
                    int t = elements[i]->PoEMemory::read<int>(elements[i]->address + 0x39c);
                    items[l * 11 + t + 1] = shared_ptr<Item>(new Item(addr));
                }
            }
        }

        return items;
    }
};
