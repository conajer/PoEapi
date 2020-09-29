/*
* Sell.cpp, 9/28/2020 6:58 PM
*/

class Sell : public Element {
public:

    Sell(addrtype address) : Element(address) {
        add_method(L"isOpened", (Element*)this, (MethodType)&Element::is_visible, AhkBool);
    }
};
