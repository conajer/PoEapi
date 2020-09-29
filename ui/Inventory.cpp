/*
* Inventory.cpp, 9/17/2020 6:21 PM
*/

class Inventory : public Element {
public:

    Inventory(addrtype address) : Element(address) {
        add_method(L"isOpened", (Element*)this, (MethodType)&Element::is_visible, AhkBool);
    }
};
