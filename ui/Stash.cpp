/*
* Stash.c, 9/17/2020 6:22 PM
*/

class Stash : public Element {
public:

    Stash(addrtype address) : Element(address) {
    }

    Stash(Element& element) : Element(element.address) {
    }
};
