/*
* Base.cpp, 8/11/2020 11:01 AM
*/

/* Base component offsets */

static std::map<string, int> base_component_offsets {
    {"internal",       0x10},
        {"x_cells",    0x10},
        {"y_cells",    0x11},
        {"name",       0x18},
    {"influence_type", 0xd6},
    {"is_corrupted",   0xd7},
};

class Base : public Component {
protected:

    wstring base_name;

public:

    Base(addrtype address) : Component(address, "Base", &base_component_offsets) {
    }

    wstring& name() {
        if (base_name.empty())
            base_name = read<wstring>("internal", "name");
        return base_name;
    }

    int influence_type() {
        return read<byte>("influence_type");
    }

    bool is_corrupted() {
        return read<byte>("is_corrupted") & 0x01;
    }

    int width() {
        return read<byte>("internal", "x_cells");
    }

    int height() {
        return read<byte>("internal", "y_cells");
    }

    int size() {
        int w = read<byte>("internal", "x_cells");
        int h = read<byte>("internal", "y_cells");

        return w * h;
    }

    void to_print() {
        const char* influence_names[] = {"Shaper", "Elder", "Crusader", "Redeemer", "Hunter", "Warlord"};

        Component::to_print();
        printf("\t\t\t! %S", name().c_str());
        if (is_corrupted())
            printf(", Corrupted");

        int type = influence_type();
        if (type > 0) {
            printf(",");
            for (int i = 0; i < 5; i++)
                if (type & (0x1 << i))
                    printf(" |%s|", influence_names[i]);
        }
    }
};
