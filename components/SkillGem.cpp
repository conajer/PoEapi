/*
* SkillGem.cpp, 8/10/2020 11:33 PM
*/

/* SkillGem component offsets */

static std::map<string, int> skillgem_component_offsets {
    {"level", 0x2c},
};

class SkillGem : public Component {
public:

    SkillGem(addrtype address) : Component(address, "SkillGem", &skillgem_component_offsets) {
    }

    int level() {
        return read<byte>("level");
    }

    void to_print() {
        Component::to_print();
        printf("\t\t\t! %d", level());
    }
};
