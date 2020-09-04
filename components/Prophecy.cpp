/*
* Prophecy.cpp, 8/11/2020 11:13 AM
*/

/* Prophecy component offsets */

static std::map<string, int> prophecy_component_offsets {
    {"base",               0x20},
        {"id_string",       0x0},
        {"prediction_text", 0x8},
        {"id",             0x10},
        {"name",           0x14},
        {"flavour_text",   0x1c},
};

class Prophecy : public Component {
public:

    Prophecy(addrtype address) : Component(address, "Prophecy", &prophecy_component_offsets) {
    }

    wstring id_string() {
        return read<wstring>("base", "id_string");
    }

    wstring prediction_text() {
        return read<wstring>("base", "prediction_text");
    }

    int id() {
        return read<int>("base", "id");
    }

    wstring name() {
        return read<wstring>("base", "name");
    }

    wstring flavour_text() {
        return read<wstring>("base", "flavour_text");
    }

    void to_print() {
        Component::to_print();
        wprintf(L"\t\t\t! %S", id_string().c_str());
    }
};
