/*
* Actor.cpp, 8/8/2020 11:12 AM
*/

/* Actor component offsets */

static FieldOffsets actor_skill_offsets {
    {"id",                      0x10},
    {"GrantedEffectsPerLevel",  0x20},
        {"active_skill",         0x8},
            {"name",             0x0},
        {"level",               0x10},
        {"required_level",      0x74},
        {"mana_multiplier",     0x78},
    {"can_be_used_with_weapon", 0x50},
    {"can_not_be_used",         0x51},
    {"mana_cost",               0x54},
    {"total_uses",              0x58},
    {"cooldown",                0x60},
    {"souls_per_use",           0x70},
    {"total_vaal_uses",         0x74},
};

class ActorSkill : public RemoteMemoryObject {
public:

    int id;
    wstring name;
    bool is_vaal_skill;
    
    ActorSkill(addrtype address) : RemoteMemoryObject(address, &actor_skill_offsets) {
        id = read<short>("id") & 0xffff;
        addrtype addr = read<addrtype>("GrantedEffectsPerLevel", "active_skill");
        name = PoEMemory::read<wstring>(addr, 32);
        if (name.empty()) {
            if (id == 0x266)
                name = L"Interactive";
            else if (id == 0x2909)
                name = L"Move";
        }
        is_vaal_skill = souls_per_use() > 0;
    }

    int level() {
        return read<int>("GrantedEffectsPerLevel", "level");
    }

    int mana_cost() {
        return read<int>("mana_cost");
    }

    int cooldown() {
        return read<int>("cooldown");
    }

    int souls_per_use() {
        return read<int>("souls_per_use");
    }

    void to_print() {
        printf("    %llx: %04x, %2d, %S\n", address, id, level(), name.c_str());
    }
};

enum ActionFlags {
    ACTION_USING_SKILL = 0x2,
    ACTION_DEAD        = 0x40,
    ACTION_MOVING      = 0x80,
    ACTION_HAS_MINES   = 0x800,
};

static std::map<string, int> actor_component_offsets {
    {"action",       0x1a8},
        {"skill",    0x150},
        {"target",   0x168},
    {"action_id",    0x208},
    {"action_count", 0x20a},
    {"vaal_skills",  0x540},
    {"skills",       0x650},
};

class Actor : public Component {
protected:
    
    void get_skills() {
        for (auto addr : read_array<addrtype>("skills", 0x8, 16)) {
            ActorSkill* skill = new ActorSkill(addr);
            skills.insert(std::make_pair(addr, shared_ptr<ActorSkill>(skill)));
        }
    }

public:
    
    std::unordered_map<addrtype, shared_ptr<ActorSkill>> skills;
    addrtype target_address;
    shared_ptr<ActorSkill> skill;

    Actor(addrtype address) : Component(address, "Actor", &actor_component_offsets) {
    }

    int action_id() {
        int tmp_action_id = read<short>("action_id");
        if (tmp_action_id) {
            addrtype addr = read<addrtype>("action", "skill");
            if (!skill || addr != skill->address) {
                auto i = skills.find(addr);
                if (i == skills.end()) {
                    /* update actor skills */
                    get_skills();
                    i = skills.find(addr);
                }

                if (i != skills.end()) {
                    skill = i->second;
                    target_address = read<addrtype>("action", "target");
                }
            }
        } else {
            skill.reset();
            target_address = 0;
        }

        return tmp_action_id;
    }

    int action_count() {
        return read<short>("action_count") & 0xffff;
    }

    bool is_using_skill() {
        return read<short>("action_id") & 0x2;
    }

    bool is_dead() {
        return read<short>("action_id") & 0x40;
    }

    bool is_moving() {
        return read<short>("action_id") & 0x80;
    }

    bool has_mines() {
        return read<short>("action_id") & 0x800;
    }

    void list_skills() {
        cout << "Actor skills: " << endl;
        for (auto s : read_array<ActorSkill>("skills", 0x8, 16))
            s.to_print();
    }

    void to_print() {
        Component::to_print();

        printf("\t\t\t! %x", action_id());
    }
};
