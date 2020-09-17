/*
* Charges.cpp, 8/11/2020 2:07 PM
*/

/* Charges component offsets */

static std::map<string, int> charges_component_offsets {
    {"base",                0x10},
        {"max_charges",     0x10},
        {"charges_per_use", 0x14},
    {"charges",             0x18},
};

class Charges : public Component {
public:

    Charges(addrtype address) : Component(address, "Charges", &charges_component_offsets) {
        add_method(L"Charges", this, (MethodType)&Charges::charges);
        add_method(L"ChargesPerUse", this, (MethodType)&Charges::charges_per_use);
        add_method(L"MaxCharges", this, (MethodType)&Charges::max_charges);
    }

    int charges() {
        return read<int>("charges");
    }

    int max_charges() {
        return read<int>("base", "max_charges");
    }

    int charges_per_use() {
        return read<int>("base", "charges_per_use");
    }

    void to_print() {
        Component::to_print();
        printf("\t\t\t! %d/%d", charges_per_use(), max_charges());
    }
};
