/*
* Entity.cpp, 8/6/2020 1:48 PM
*/

#include <vector>
#include <unordered_map>
#include <math.h>

struct ComponentLookupTable {
    byte flags[8];
    struct {
        addrtype name_ptr;
        int index;
        int __padding_0;
    } components[8];
};

std::map<wstring, wstring> archnemesis_mods = {
    {L"Abberath",                L"Abberath-touched"},
    {L"Arakaali",                L"Arakaali-touched"},
    {L"Arcane",                  L"Arcane Buffer"},
    {L"Berserk",                 L"Berserker"},
    {L"Brine King",              L"Brine King-touched"},
    {L"Charged",                 L"Overcharged"},
    {L"Consecrating",            L"Consecrator"},
    {L"Crystalline",             L"Crystal-skinned"},
    {L"Deadly",                  L"Assassin"},
    {L"Elemental",               L"Prismatic"},
    {L"Empowering",              L"Union of Souls"},
    {L"Entangling",              L"Entangler"},
    {L"Flamewoven",              L"Flameweaver"},
    {L"Freezing",                L"Permafrost"},
    {L"Frostwoven",              L"Frostweaver"},
    {L"Gargantuan",              L"Gargantuan"},
    {L"Hexing",                  L"Hexer"},
    {L"Hungering",               L"Soul Eater"},
    {L"Igniting",                L"Incendiary"},
    {L"Imprisoning",             L"Ice Prison"},
    {L"Innocence",               L"Innocence-touched"},
    {L"Invulnerable",            L"Benevolent Guardian"},
    {L"Kitava",                  L"Kitava-touched"},
    {L"Lunaris",                 L"Lunaris-touched"},
    {L"Opulent",                 L"Opulent"},
    {L"Rejuvenating",            L"Rejuvenating"},
    {L"Sentinel",                L"Sentinel"},
    {L"Shakari",                 L"Shakari-touched"},
    {L"Shocking",                L"Electrocuting"},
    {L"Solaris",                 L"Solaris-touched"},
    {L"Soul Conduit",            L"Soul Conduit"},
    {L"Spectral",                L"Spirit Walkers"},
    {L"Stormwoven",              L"Stormweaver"},
    {L"Swift",                   L"Hasted"},
    {L"Temporal",                L"Temporal Bubble"},
    {L"Toxic",                   L"Toxic"},
    {L"Tukohama",                L"Tukohama-touched"},
    {L"Vampiric",                L"Vampiric"},
    {L"Woodland",                L"Treant Horde"},

    {L"of Accuracy",             L"Deadeye"},
    {L"of Beyond",               L"Voidspawn of Abaxoth"},
    {L"of Bloodletting",         L"Bloodletter"},
    {L"of Bombardment",          L"Bombardier"},
    {L"of Chaos",                L"Chaosweaver"},
    {L"of Clones",               L"Mirror Image"},
    {L"of Corruption",           L"Corrupter"},
    {L"of Detonation",           L"Corpse Detonator"},
    {L"of Drought",              L"Drought Bringer"},
    {L"of Dying Breath",         L"Final Gasp"},
    {L"of Echoes",               L"Echoist"},
    {L"of Effigies",             L"Effigy"},
    {L"of Elemental Attunement", L"Empowered Elements"},
    {L"of Endurance",            L"Juggernaut"},
    {L"of Execution",            L"Executioner"},
    {L"of Flame-striding",       L"Flame Strider"},
    {L"of Frenzy",               L"Frenzied"},
    {L"of Frost-striding",       L"Frost Strider"},
    {L"of Heralding",            L"Heralds of the Obelisk"},
    {L"of Magma",                L"Magma Barrier"},
    {L"of Malediction",          L"Malediction"},
    {L"of Necromancy",           L"Necromancer"},
    {L"of Siphoning",            L"Mana Siphoner"},
    {L"of Splitting",            L"Splinterer"},
    {L"of Steel",                L"Steel-infused"},
    {L"of Storm-striding",       L"Storm Strider"},
    {L"of Storms",               L"Storm Herald"},
    {L"of Stunning",             L"Bonebreaker"},
    {L"of Trickery",             L"Trickster"},
};

FieldOffsets entity_offsets = {
    {"internal",              0x8},
        {"path",              0x8},
        {"component_lookup", 0x30},
    {"component_list",       0x10},
    {"id",                   0x60},
};

/* Forward declaration */
class Item;

class Entity : public PoEObject {
protected:

    std::vector<string> component_names;
    std::unordered_map<string, shared_ptr<Component>> components;

    Component* read_component(const string& name, addrtype address) {
        Component *component = read_object<Component>(name, address);
        component->type_name = name;

        return component;
    }

    void get_all_components(addrtype component_lookup) {
        std::vector<addrtype> component_list;

        component_list = read_array<addrtype>("component_list", 0x0, 0x8);
        if (component_list.empty() || component_list.size() > 32)
            return;

        addrtype entry_ptr = PoEMemory::read<addrtype>(component_lookup + 0x30);
        int have_more_components = PoEMemory::read<int>(component_lookup + 0x48);
        if (have_more_components > 32)
            return;

        while (have_more_components) {
            ComponentLookupTable lookup_table;
            char name[32];

            PoEMemory::read<ComponentLookupTable>(entry_ptr, &lookup_table, 1);
            for (int i = 0; i < 8; ++i) {
                if (lookup_table.flags[i] != 0xff) {
                    PoEMemory::read<char>(lookup_table.components[i].name_ptr, name, 32);
                    component_names.push_back(name);
                    components[name] = shared_ptr<Component>(
                        read_component(name, component_list[lookup_table.components[i].index]));
                    have_more_components--;
                }
            }
            entry_ptr += 0x8 + 0x80;
        }
    }

    AhkObjRef* __get_component(const char* name) {
        if (components.find(name) != components.end())
            return (AhkObjRef*)*components[name].get();
        return nullptr;
    }

    AhkObjRef* get_components() {
        AhkObj all_components;
        for (auto& i : components)
            all_components.__set(i.second->type_name.c_str(), (AhkObjRef*)*i.second, AhkObject, nullptr);
        __set(L"components", (AhkObjRef*)all_components, AhkObject, nullptr);

        return all_components;
    }

    AhkObjRef* __get_pos() {
        if (label) {
            Point p = label->get_pos();
            AhkTempObj pos;
            pos.__set(L"x", p.x, AhkInt, L"y", p.y, AhkInt, nullptr);

            return pos;
        }

        return nullptr;
    }
    
public:

    wstring type_name;
    wstring path;
    int id;
    Life *health = nullptr;
    Positioned* positioned = nullptr;
    Render* render = nullptr;
    shared_ptr<Element> label;
    shared_ptr<Item> item;
    Vector3 pos, bounds;
    Point grid_pos;
    int max_life = -1;
    int saved_life = -1;
    int saved_es = -1;
    int damage_taken = 0;

    bool is_player = false;
    bool is_npc = false;
    bool is_movable = true;

    /* Monster related fields */
    bool is_monster = false;
    bool is_beast = false;
    bool is_minion = false;
    bool is_neutral = false;
    int rarity = 0;
    wstring archnemesis_hint;

    Entity(addrtype address, const wchar_t* metadata = L"")
        : PoEObject(address, &entity_offsets), path(metadata)
    {
        addrtype hdata[4];
        PoEMemory::read<addrtype>(address, hdata, 4);
        if (!hdata[1] || !hdata[2])
            return;

        addrtype internal[8];
        PoEMemory::read<addrtype>(hdata[1], internal, 8);

        if (path.empty()) {
            int len = (internal[3] & 0xff) + 1;
            wchar_t path_string[len];
            PoEMemory::read<wchar_t>(internal[1], path_string, len);
            if (path_string[0] != L'M')
                return;

            path = path_string;
        }
        id = read<int>("id");
        get_all_components(internal[6]);
        health = get_component<Life>();
        positioned = get_component<Positioned>();
        render = get_component<Render>();
        get_position();

        if (has_component("Player"))
            is_player = true;
        else if (has_component("NPC"))
            is_npc = true;
        else if (has_component("Monster"))
            is_monster = true;
        else
            is_movable = false;

        if (is_monster) {
            is_minion = positioned->is_minion();
            is_neutral = positioned->is_neutral();
            ObjectMagicProperties* props = get_component<ObjectMagicProperties>();
            rarity = props ? props->rarity() : 0;

            if (rarity == 2) {
                for (auto& i : props->get_mods()) {
                    if (i.id.find(L"Archnemesis") != wstring::npos) {
                        if (!archnemesis_hint.empty())
                            archnemesis_hint += L", ";
                        archnemesis_hint += !archnemesis_mods[i.name].empty() ? archnemesis_mods[i.name] : i.name;
                    }
                }

                if (path.find(L"Bestiary") != wstring::npos && path.find(L"Minion") == wstring::npos)
                    is_beast = true;
            }

            if (health) {
                saved_es = health->energy_shield();
                saved_life = health->life(&max_life);
            }
        }
    }

    void __init() {
        PoEObject::__init();
        add_method(L"name", this, (MethodType)&Entity::name, AhkWStringPtr);
        add_method(L"getItem", this, (MethodType)&Entity::get_item, AhkObject);
        add_method(L"getComponent", this, (MethodType)&Entity::__get_component, AhkObject, ParamList{AhkString});
        add_method(L"getComponents", this, (MethodType)&Entity::get_components, AhkObject);
        add_method(L"__getPos", this, (MethodType)&Entity::__get_pos, AhkObject);
    }

    void __new() {
        PoEObject::__new();
        __set(L"id", id, AhkInt,
              L"path", path.c_str(), AhkWString,
              nullptr);
    }

    virtual wstring& name() {
        if (type_name.empty()) {
            if (has_component("Render"))
                type_name = get_component<Render>()->name();

            if (type_name.empty())
                type_name = path.substr(path.rfind(L'/') + 1);
        }

        return type_name;            
    }

    // implemented after Item class.
    AhkObjRef* get_item();

    int life() {
        if (health) {
            int es = health->energy_shield();
            int life = health->life();

            if (saved_es > es)
                damage_taken += saved_es - es;
            if (saved_life > life)
                damage_taken += saved_life - life;
            saved_es = es;
            saved_life = life;
        }

        return saved_life;
    }

    bool is_dead() {
        return life() == 0;
    }

    bool is_valid() {
        return id == read<int>("id");
    }

    void get_position() {
        if (render) {
            pos = render->position();
            bounds = render->bounds();
        }
        if (positioned)
            grid_pos = positioned->grid_position();
    }

    bool has_component(const string& name) {
        return components.find(name) != components.end();
    }

    int has_component(std::vector<string>& names) {
        for (int i = 0; i < names.size(); ++i)
            if (components.find(names[i]) != components.end())
                return i;
        return -1;
    }

    bool is(const string& type_name) {
        return components.find(type_name) != components.end();
    }

    template <typename T> T* get_component() {
        char* component_name;

        /* typeid's name has format 'N<typename>', N is length of the typename string. */
        strtol(typeid(T).name(), &component_name, 0);

        auto i = components.find(component_name);
        if (i != components.end())
            return dynamic_cast<T*>(i->second.get());

        return  nullptr;
    }

    Component* operator[](const string& component_name) {
        return components[component_name].get();
    }

    void list_components() {
        for (string& name : component_names) {
            std::cout << "\t";
            components[name]->to_print();
            std::cout << endl;
        }
    }

    void to_print() {
        wprintf(L"%llx: ", address);
        if (has_component("Render")) {
            wprintf(L"%S", get_component<Render>()->name().c_str());
        }

        if (verbose) {
           wprintf(L"\n    %4x, %S", id, path.c_str());
           if (verbose > 1)
               list_components();
        }
    }
};

class LocalPlayer : public Entity {
public:

    wstring player_name;
    wstring player_class;
    Player* player;
    Actor* actor;

    LocalPlayer(addrtype address) : Entity(address) {
        player = get_component<Player>();
        player_name = player->name();
        actor = get_component<Actor>();
        player_class = get_component<PlayerClass>()->name();

        add_method(L"getExp", this, (MethodType)&LocalPlayer::get_exp, AhkUInt);
        add_method(L"isMoving", this, (MethodType)&LocalPlayer::is_moving, AhkBool);
        add_method(L"level", this, (MethodType)&LocalPlayer::level, AhkInt);
    }

    void __new() {
        Entity::__new();
        __set(L"name", player_name.c_str(), AhkWString,
              L"className", player_class.c_str(), AhkWString,
              nullptr);
    }

    wstring& name() {
        return player_name;
    }

    bool is_moving() {
        return actor->is_moving();
    }

    int dist(Entity& entity) {
        if (!entity.has_component("Positioned"))
            return -1;

        int dx = grid_pos.x - entity.grid_pos.x;
        int dy = grid_pos.y - entity.grid_pos.y;

        return sqrt(dx * dx + dy * dy);
    }

    int level() {
        return player->level();
    }

    unsigned int get_exp() {
        return player->exp();
    }
};

class Item : public Entity {
protected:

    Base* base;
    Mods* mods;
    
public:

    wstring item_name;
    bool identified = false;
    int size;
    int width = 1, height = 1;

    Item(addrtype address) : Entity(address) {
        base = get_component<Base>();
        mods = get_component<Mods>();
    }

    void __init() {
        Entity::__init();
        identified = is_identified();
        size = get_size();

        add_method(L"isCorrupted", this, (MethodType)&Item::is_corrupted, AhkBool);
        add_method(L"isBlighted", this, (MethodType)&Item::is_blighted, AhkBool);
        add_method(L"isBlightRavaged", this, (MethodType)&Item::is_blight_ravaged, AhkBool);
        add_method(L"isCrafted", this, (MethodType)&Item::is_crafted, AhkBool);
        add_method(L"isEnchanted", this, (MethodType)&Item::is_enchanted, AhkBool);
        add_method(L"isFractured", this, (MethodType)&Item::is_fractured, AhkBool);
        add_method(L"isMirrored", this, (MethodType)&Item::is_mirrored, AhkBool);
        add_method(L"isSynthesised", this, (MethodType)&Item::is_synthesised, AhkBool);
        add_method(L"isVeiled", this, (MethodType)&Item::is_veiled, AhkBool);
        add_method(L"isRGB", this, (MethodType)&Item::is_rgb, AhkBool);
        add_method(L"ilvl", this, (MethodType)&Item::get_item_level);
        add_method(L"itemLevel", this, (MethodType)&Item::get_item_level);
        add_method(L"quality", this, (MethodType)&Item::get_quality);
        add_method(L"qualityType", this, (MethodType)&Item::get_quality_type);
        add_method(L"sockets", this, (MethodType)&Item::get_sockets);
        add_method(L"links", this, (MethodType)&Item::get_links);
        add_method(L"tier", this, (MethodType)&Item::get_tier);
        add_method(L"level", this, (MethodType)&Item::get_level);
        add_method(L"stackCount", this, (MethodType)&Item::get_stack_count);
        add_method(L"stackSize", this, (MethodType)&Item::get_stack_size);
        add_method(L"charges", this, (MethodType)&Item::get_charges);
        add_method(L"job", this, (MethodType)&Item::get_job, AhkWString);
        add_method(L"size", this, (MethodType)&Item::get_size);
        add_method(L"getInfluences", this, (MethodType)&Item::get_influences, AhkWString);
        add_method(L"getInfluenceType", this, (MethodType)&Item::get_influence_type, AhkInt);
        add_method(L"getExplicitMods", this, (MethodType)&Item::get_explicit_mods, AhkObject);
        add_method(L"getMods", this, (MethodType)&Item::get_mods, AhkObject);
        add_method(L"getExplicitStats", this, (MethodType)&Item::get_explicit_stats, AhkObject);
        add_method(L"getStats", this, (MethodType)&Item::get_stats, AhkObject);
    }

    void __new() {
        Entity::__new();
        __set(L"name", name().c_str(), AhkWString,
              L"baseName", base_name().c_str(), AhkWString,
              L"isIdentified", is_identified(), AhkBool,
              L"ilvl", get_item_level(), AhkInt,
              L"rarity", get_rarity(), AhkInt,
              L"width", width, AhkInt,
              L"height", height, AhkInt,
              nullptr);
    }

    wstring& name() {
        if (item_name.empty()) {
            if (mods && mods->rarity > 0 && mods->is_identified())
                item_name = mods->get_name(base->name());   /* magic/rare/unique items */
            if (item_name.empty())
                item_name = base_name();                    /* normal or unidentified items */
        }

        return item_name;                                 
    }

    wstring& base_name() {
        if (has_component("CapturedMonster")) {
            if (path.find(L"Metamorphosis") == wstring::npos)
                return get_component<CapturedMonster>()->name();
        }

        return base ? base->name() : type_name;
    }

    bool is_identified() {
        return mods ? mods->is_identified() : true;
    }

    bool is_blighted() {
        if (mods) {
            mods->get_mods();
            if (!mods->implicit_mods.empty())
                return mods->implicit_mods.front().id == L"InfectedMap";
        }

        return false;
    }

    bool is_blight_ravaged() {
        if (mods) {
            mods->get_mods();
            if (!mods->implicit_mods.empty())
                return mods->implicit_mods.front().id == L"UberInfectedMap__";
        }

        return false;
    }

    bool is_corrupted() {
        return base ? base->is_corrupted() : false;
    }

    bool is_crafted() {
        if (mods) {
            mods->get_mods();
            if (!mods->explicit_mods.empty())
                return mods->explicit_mods.back().domain == 9;
        }

        return false;
    }

    bool is_enchanted() {
        if (mods) {
            mods->get_mods();
            return !mods->enchant_mods.empty();
        }

        return false;
    }

    bool is_fractured() {
        if (mods) {
            mods->get_stats();
            return !mods->fractured_stats.empty();
        }

        return false;
    }

    bool is_mirrored() {
        return mods ? mods->is_mirrored() : false;
    }

    bool is_synthesised() {
        return mods ? mods->is_synthesised() : false;
    }

    bool is_veiled() {
        if (mods) {
            mods->get_mods();
            for (auto& i : mods->explicit_mods) {
                if (i.id.find(L"Veiled") != wstring::npos)
                    return true;
            }
        }

        return false;
    }

    int get_item_level() {
        return mods ? mods->item_level : 0;
    }

    int get_rarity() {
        return mods ? mods->rarity : 0;
    }

    int get_sockets() {
        Sockets* sockets = get_component<Sockets>();
        return sockets ? sockets->sockets() : 0;
    }

    int get_links() {
        Sockets* sockets = get_component<Sockets>();
        return sockets ? sockets->links() : 0;
    }

    bool is_rgb() {
        Sockets* sockets = get_component<Sockets>();
        return sockets ? sockets->is_rgb() : false;
    }

    int get_quality() {
        Quality* quality = get_component<Quality>();
        return quality ? quality->quality() : 0;
    }

    int get_quality_type() {
        SkillGem* gem = get_component<SkillGem>();
        return gem ? gem->quality_type() : 0;
    }

    int get_tier() {
        Map* map = get_component<Map>();
        return map ? map->tier() : 0;
    }

    int get_level() {
        SkillGem* gem = get_component<SkillGem>();
        return gem ? gem->level() : 0;
    }

    int get_stack_count() {
        Stack* stack = get_component<Stack>();
        return stack ? stack->stack_count() : 0;
    }

    int get_stack_size() {
        Stack* stack = get_component<Stack>();
        return stack ? stack->stack_size() : 0;
    }

    int get_charges() {
        Charges* charges = get_component<Charges>();
        return charges ? charges->charges(): 0;
    }

    const wchar_t* get_job() {
        HeistContract* contract = get_component<HeistContract>();
        return contract ? contract->get_jobs()[0]->skill_name.c_str(): nullptr;
    }

    int get_size() {
        if (base) {
            width = base->width();
            height = base->height();
        }

        return width * height;
    }

    int get_influence_type() {
        return base ? base->influence_type() : 0;
    }

    const wchar_t* get_influences() {
        static wstring all_influences[] = {L"Shaper", L"Elder", L"Crusader", L"Redeemer", L"Hunter", L"Warlord"};
        static wstring influences;
        int itype = base->influence_type();

        influences.clear();
        for (int i = 0; i < 6; ++i) {
            if (itype & (1 << i)) {
                if (!influences.empty()) {
                    influences += L"/" + all_influences[i];
                    break;
                }
                influences = all_influences[i];
            }
        }

        return influences.c_str();
    }

    AhkObjRef* get_explicit_mods() {
        AhkTempObj temp_mods;
        if (mods) {
            mods->get_mods();
            for (auto& i : mods->explicit_mods)
                temp_mods.__set(L"", (AhkObjRef*)i, AhkObject, nullptr);
        }
        
        return temp_mods;
    }

    AhkObjRef* get_mods() {
        AhkTempObj temp_mods;
        if (mods) {
            mods->get_mods();
            for (auto& i : mods->enchant_mods)
                temp_mods.__set(L"", (AhkObjRef*)i, AhkObject, nullptr);
            for (auto& i : mods->implicit_mods)
                temp_mods.__set(L"", (AhkObjRef*)i, AhkObject, nullptr);
            for (auto& i : mods->explicit_mods)
                temp_mods.__set(L"", (AhkObjRef*)i, AhkObject, nullptr);
        }
        
        return temp_mods;
    }

    AhkObjRef* get_explicit_stats() {
        AhkTempObj explicit_stats;
        if (mods) {
            mods->get_stats();
            for (auto& i : mods->fractured_stats)
                explicit_stats.__set(L"", (i + L" (fractured)").c_str(), AhkWString, nullptr);
            for (auto& i : mods->explicit_stats)
                explicit_stats.__set(L"", i.c_str(), AhkWString, nullptr);
            for (auto& i : mods->crafted_stats)
                explicit_stats.__set(L"", (i + L" (crafted)").c_str(), AhkWString, nullptr);
        }
        
        return explicit_stats;
    }

    AhkObjRef* get_stats() {
        AhkTempObj all_stats;
        if (mods) {
            mods->get_stats();
            for (auto& i : mods->enchant_stats)
                all_stats.__set(L"", (i + L" (enchant)").c_str(), AhkWString, nullptr);
            for (auto& i : mods->implicit_stats)
                all_stats.__set(L"", (i + L" (implicit)").c_str(), AhkWString, nullptr);
            for (auto& i : mods->fractured_stats)
                all_stats.__set(L"", (i + L" (fractured)").c_str(), AhkWString, nullptr);
            for (auto& i : mods->explicit_stats)
                all_stats.__set(L"", i.c_str(), AhkWString, nullptr);
            for (auto& i : mods->crafted_stats)
                all_stats.__set(L"", (i + L" (crafted)").c_str(), AhkWString, nullptr);
        }

        return all_stats;
    }
};

AhkObjRef* Entity::get_item() {
    if (has_component("WorldItem")) {
        item = shared_ptr<Item>(new Item(get_component<WorldItem>()->item()));
        return *item;
    } else if (has_component("HeistRewardDisplay")) {
        item = shared_ptr<Item>(new Item(get_component<HeistRewardDisplay>()->item()));
        return *item;
    }

    return nullptr;
}
