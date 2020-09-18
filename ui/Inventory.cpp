/*
* Inventory.c, 9/17/2020 6:21 PM
*/

class Inventory : public Element {
public:

    Inventory(addrtype address) : Element(address) {
    }

    Inventory(Element& element) : Element(element.address) {
    }
};
