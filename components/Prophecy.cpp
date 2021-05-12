/*
* Prophecy.cpp, 8/11/2020 11:13 AM
*/

/* Prophecy component offsets */

static std::map<string, int> prophecy_component_offsets {
    {"base",               0x18},
        {"id_string",       0x0},
        {"prediction_text", 0x8},
        {"id",             0x10},
        {"name",           0x14},
        {"flavour_text",   0x1c},
};

class Prophecy : public Component {
private:

    const wchar_t* get_id_text() {
        return id_text().c_str();
    }

    const wchar_t* get_prediction_text() {
        return prediction_text().c_str();
    }

    const wchar_t* get_name() {
        return name().c_str();
    }

    const wchar_t* get_flavour_text() {
        return flavour_text().c_str();
    }

public:

    wstring id_string;
    wstring prediction;
    wstring prophecy_name;
    wstring flavour;

    Prophecy(addrtype address) : Component(address, "Prophecy", &prophecy_component_offsets) {
        add_method(L"idText", this, (MethodType)&Prophecy::get_id_text, AhkWString);
        add_method(L"id", this, (MethodType)&Prophecy::id, AhkInt);
        add_method(L"predictionText", this, (MethodType)&Prophecy::get_prediction_text, AhkWString);
        add_method(L"name", this, (MethodType)&Prophecy::get_name, AhkWString);
        add_method(L"flavourText", this, (MethodType)&Prophecy::get_flavour_text, AhkWString);
    }

    wstring& id_text() {
        if (id_string.empty()) {
            wchar_t buffer[256];
            addrtype addr = read<addrtype>("base", "id_string");
            PoEMemory::read<wchar_t>(addr, buffer, 256);
            id_string = buffer;
        }

        return id_string;
    }

    wstring& prediction_text() {
        if (prediction.empty()) {
            wchar_t buffer[256];
            addrtype addr = read<addrtype>("base", "prediction_text");
            PoEMemory::read<wchar_t>(addr, buffer, 256);
            prediction = buffer;
        }

        return prediction;
    }

    int id() {
        return read<int>("base", "id");
    }

    wstring& name() {
        if (prophecy_name.empty()) {
            wchar_t buffer[32];
            addrtype addr = read<addrtype>("base", "name");
            PoEMemory::read<wchar_t>(addr, buffer, 32);
            prophecy_name = buffer;
        }

        return prophecy_name;
    }

    wstring& flavour_text() {
        if (flavour.empty()) {
            wchar_t buffer[256];
            addrtype addr = read<addrtype>("base", "flavour_text");
            PoEMemory::read<wchar_t>(addr, buffer, 256);
            flavour = buffer;
        }

        return flavour;
    }

    void to_print() {
        Component::to_print();
        wprintf(L"\t\t\t! %S", id_text().c_str());
    }
};
