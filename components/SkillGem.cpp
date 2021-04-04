/*
* SkillGem.cpp, 8/10/2020 11:33 PM
*/

/* SkillGem component offsets */

static std::map<string, int> skillgem_component_offsets {
    {"level",        0x2c},
    {"quality_type", 0x38},
};

static wstring gem_types[4] = {L"", L"Anomalous", L"Divergent", L"Phantasmal"};

class SkillGem : public Component {
public:

    wstring gem_name;

    SkillGem(addrtype address) : Component(address, "SkillGem", &skillgem_component_offsets) {
    }

    wstring& name(wstring& base_name) {
        int type = quality_type();
        if (type > 0) {
            if (gem_name.empty())
                gem_name = gem_types[type] + L" " + base_name;
            return gem_name;
        }

        return base_name;
    }

    int level() {
        return read<byte>("level");
    }

    int quality_type() {
        return read<byte>("quality_type");
    }

    void to_print() {
        Component::to_print();
        printf("\t\t\t! %d", level());
    }
};
