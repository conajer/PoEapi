/*
* Entity.cpp, 8/6/2020 1:48 PM
*/

#include <vector>
#include <unordered_map>
#include <math.h>

FieldOffsets entity_offsets = {
    {"internal",        0x8},
        {"path",        0x8},
    {"component_list", 0x10},
    {"id",             0x60},
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

    void get_all_components() {
        addrtype component_list = read<addrtype>("component_list");
        addrtype component_lookup = PoEMemory::read<addrtype>(address + 0x8, {0x30, 0x30});
        addrtype component_entry = component_lookup;
        string component_name;

        while (1) {
            component_entry = PoEMemory::read<addrtype>(component_entry);
            if (!component_entry || component_entry == component_lookup)
                break;

            // Component name
            component_name = PoEMemory::read<string>(component_entry + 0x10, 32);

            // Component address
            int offset = PoEMemory::read<int>(component_entry + 0x18);
            addrtype addr = PoEMemory::read<addrtype>(component_list + offset * 8);
            addrtype owner_address = PoEMemory::read<addrtype>(addr + 0x8);

            // Invalid component
            if (owner_address != address)
                break;
            
            component_names.insert(component_names.end(), component_name);
            std::shared_ptr<Component> component_ptr(read_component(component_name, addr));
            components.insert(std::make_pair(component_name, component_ptr));
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
    bool is_valid = true;
    shared_ptr<Element> label;
    shared_ptr<Item> item;
    Point pos;

    bool is_player = false;
    bool is_npc = false;

    /* Monster related fields */
    bool is_monster = false;
    bool is_neutral = false;
    int rarity = 0;

    Entity(addrtype address) : PoEObject(address, &entity_offsets) {
        path = read<wstring>("internal", "path");
        if (path[0] != L'M') {
            this->is_valid = false;
            return;
        }
        id = read<int>("id");

        get_all_components();
        is_player = has_component("Player");
        is_npc = has_component("NPC");
        is_monster = has_component("Monster");

        if (is_monster) {
            Positioned *positioned = get_component<Positioned>();
            is_neutral = positioned ? positioned->is_neutral() : false;
            ObjectMagicProperties* props = get_component<ObjectMagicProperties>();
            rarity = props ? props->rarity() : 0;
        }

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

    AhkObjRef* get_item();

    int life() {
        Life* life = get_component<Life>();
        return life ? life->life() : -1;
    }

    bool is_dead() {
        Life* life = get_component<Life>();
        return life && life->life() == 0;
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
    Life* life;
    Player* player;
    Positioned* positioned;
    Actor* actor;

    LocalPlayer(addrtype address) : Entity(address) {
        player = get_component<Player>();
        life = get_component<Life>();
        positioned = get_component<Positioned>();;
        actor = get_component<Actor>();;

        player_name = player->name();
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
        if (!positioned || !entity.has_component("Positioned"))
            return -1;

        Point pos1 = positioned->grid_position();
        Point pos2 = entity.get_component<Positioned>()->grid_position();
        int dx = pos1.x - pos2.x;
        int dy = pos1.y - pos2.y;

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

    Item(addrtype address) : Entity(address) {
        base = get_component<Base>();
        mods = get_component<Mods>();

        add_method(L"isCorrupted", this, (MethodType)&Item::is_corrupted, AhkBool);
        add_method(L"isBlighted", this, (MethodType)&Item::is_blighted, AhkBool);
        add_method(L"isCrafted", this, (MethodType)&Item::is_crafted, AhkBool);
        add_method(L"isEnchanted", this, (MethodType)&Item::is_enchanted, AhkBool);
        add_method(L"isFractured", this, (MethodType)&Item::is_fractured, AhkBool);
        add_method(L"isMirrored", this, (MethodType)&Item::is_mirrored, AhkBool);
        add_method(L"isSynthesised", this, (MethodType)&Item::is_synthesised, AhkBool);
        add_method(L"isVeiled", this, (MethodType)&Item::is_veiled, AhkBool);
        add_method(L"isRGB", this, (MethodType)&Item::is_rgb, AhkBool);
        add_method(L"itemLevel", this, (MethodType)&Item::get_item_level);
        add_method(L"quality", this, (MethodType)&Item::get_quality);
        add_method(L"sockets", this, (MethodType)&Item::get_sockets);
        add_method(L"links", this, (MethodType)&Item::get_links);
        add_method(L"tier", this, (MethodType)&Item::get_tier);
        add_method(L"level", this, (MethodType)&Item::get_level);
        add_method(L"stackCount", this, (MethodType)&Item::get_stack_count);
        add_method(L"stackSize", this, (MethodType)&Item::get_stack_size);
        add_method(L"charges", this, (MethodType)&Item::get_charges);
        add_method(L"size", this, (MethodType)&Item::get_size);
        add_method(L"getInfluenceType", this, (MethodType)&Item::get_influence_type, AhkInt);
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
              nullptr);
    }

    wstring& name() {
        if (mods) {
            if (mods->rarity > 0 && mods->is_identified())   /* magic/rare/unique items */
                return mods->name(base_name());
        } else if (has_component("Prophecy")) {
            Prophecy* prophecy = get_component<Prophecy>();
            if (prophecy)
                return prophecy->name();
        } else if (has_component("SkillGem")) {
            SkillGem* skill_gem = get_component<SkillGem>();
            if (skill_gem)
                return skill_gem->name(base_name());
        }

        return base_name();                                 /* normal or unidentified items */
    }

    wstring& base_name() {
         if (has_component("CapturedMonster"))
            return get_component<CapturedMonster>()->name();

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

    int get_size() {
        return base ? base->size() : 1;
    }

    int get_influence_type() {
        return base ? base->influence_type() : 0;
    }

    AhkObjRef* get_mods() {
        AhkTempObj temp_mods;
        if (mods) {
            mods->get_mods();
            for (auto& i : mods->enchant_mods)
                temp_mods.__set(L"", i.id.c_str(), AhkWString, nullptr);
            for (auto& i : mods->implicit_mods)
                temp_mods.__set(L"", i.id.c_str(), AhkWString, nullptr);
            for (auto& i : mods->explicit_mods)
                temp_mods.__set(L"", i.id.c_str(), AhkWString, nullptr);
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
