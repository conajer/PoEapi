/*
* Favours.cpp, 9/17/2020 6:21 PM
*/

class Favours : public Element {
private:

    AhkObjRef* __get_childs() {
        AhkObj temp_childs;

        items.clear();
        for (auto& i : get_childs()) {
            if (!i)
                continue;

            addrtype addr = i->PoEMemory::read<addrtype>(i->address + 0x390);
            if (addr) {
                int l = i->PoEMemory::read<int>(i->address + 0x398);
                int t = i->PoEMemory::read<int>(i->address + 0x39c);
                int index = l * 12 + t + 1;
                items[index] = shared_ptr<Item>(new Item(addr));
                i->__set(L"item", (AhkObjRef*)*items[index], AhkObject, nullptr);
            }
            temp_childs.__set(L"", (AhkObjRef*)*i, AhkObject, nullptr);
        }
        __set(L"childs", (AhkObjRef*)temp_childs, AhkObject, nullptr);

        return temp_childs;
    }

    AhkObjRef* __get_items() {
        AhkTempObj items;
        for (auto& i : get_items())
            items.__set(std::to_wstring(i.first).c_str(), (AhkObjRef*)*i.second, AhkObject, nullptr);
        __set(L"items", (AhkObjRef*)items, AhkObject, nullptr);

        return items;
    }

public:

    std::map<int, shared_ptr<Item>> items;

    Favours(addrtype address) : Element(address) {
        add_method(L"isOpened", (Element*)this, (MethodType)&Element::is_visible, AhkBool);
        add_method(L"getChilds", this, (MethodType)&Favours::__get_childs, AhkObject);
        add_method(L"getItems", this, (MethodType)&Favours::__get_items, AhkObject);
    }

    std::map<int, shared_ptr<Item>> get_items() {
        items.clear();
        for (auto e : get_childs()) {
            addrtype addr = e->PoEMemory::read<addrtype>(e->address + 0x390);
            if (addr) {
                int l = e->PoEMemory::read<int>(e->address + 0x398);
                int t = e->PoEMemory::read<int>(e->address + 0x39c);
                items[l * 12 + t + 1] = shared_ptr<Item>(new Item(addr));
            }
        }

        return items;
    }
};
