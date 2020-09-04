/*
* Entity.cpp, 8/6/2020 1:48 PM
*/

#include <vector>
#include <unordered_map>
#include <math.h>

class AHKEntity;

static FieldOffsets entity_offsets = {
    {"internal",        0x8},
        {"path",        0x8},
    {"component_list", 0x10},
    {"id",             0x50},
};

class Entity : public RemoteMemoryObject {
protected:

    std::vector<string> component_names;
    std::unordered_map<string, shared_ptr<Component>> components;

    Component* read_component(const string& name, addrtype address) {
        Component *c = read_object<Component>(name, address);
        c->type_name = name;

        return c;
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
    
    AhkObj* get_ahkobj(RemoteMemoryObject& obj) {
        if (!ahkobj) {
            ahkobj = RemoteMemoryObject::get_ahkobj(obj);

            if (0)
            ahkobj->set("Name", name().c_str(), AHK_WSTR,
                        "Id", id, AHK_INT,
                        "Path", path.c_str(), AHK_WSTR,
                        "Components", 0, AHK_OBJECT,
                        0);
            else {
            ahkobj->set("Name", name().c_str(), AHK_WSTR, 0);
            ahkobj->set("Id", id, AHK_INT, 0);
            ahkobj->set("Path", path.c_str(), AHK_WSTR, 0);
            ahkobj->set("Components", 0, AHK_OBJECT, 0);
            }

            void* objref = ahkobj->get("Components");
            if (objref) {
                AhkObj ahkobj_components((AhkObjRef*)objref);
                for (auto i : components) {
                    AhkObj* ahkobj = *i.second;
                    ahkobj_components.set(i.first.c_str(), ahkobj->ahkobj_ref, AHK_OBJECT, 0);
                }
            }
        }

        return ahkobj;
    }

public:

    wstring type_name;
    wstring path;
    int id;
    bool is_valid = true;

    /* Monster related fields */
    bool is_monster, is_neutral;
    int rarity;

    Entity(addrtype address) : RemoteMemoryObject(address, &entity_offsets) {
        path = read<wstring>("internal", "path");
        if (path[0] != L'M') {
            this->is_valid = false;
            return;
        }
        id = read<int>("id");

        get_all_components();
        is_monster = has_component("Monster");
        if (is_monster) {
            rarity = get_component<ObjectMagicProperties>()->rarity();
            is_neutral = get_component<Positioned>()->is_neutral();
        }

        // generate AhkObj automatically
        ahkobj = get_ahkobj(*this);
    }

    wstring& name() {
        if (has_component("Render"))
            return get_component<Render>()->name();

        if (type_name.empty())
            type_name = path.substr(path.rfind(L'/') + 1);

        return type_name;            
    }

    bool has_component(const string& name) {
        return components.find(name) != components.end();
    }

    bool is(const string& type_name) {
        return components.find(type_name) != components.end();
    }

    template <typename T> T* get_component() {
        char* component_name;

        /* typeid's name has format 'N<typename>', N is length of the typename string. */
        strtol(typeid(T).name(), &component_name, 0);

        auto i = components.find(component_name);
        if (i != components.end()) {
            return dynamic_cast<T*>(i->second.get());
        }

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
    int level;
    Life* life;
    Positioned* positioned;

    LocalPlayer(addrtype address) : Entity(address) {
        Player* player = get_component<Player>();
        life = get_component<Life>();
        positioned = get_component<Positioned>();;

        player_name = player->name();
        level = player->level();
    }

    wstring& name() {
        return player_name;
    }

    int dist(Entity& entity) {
        if (!entity.has_component("Positioned"))
            return -1;

        Point pos1 = positioned->get_position();
        Point pos2 = entity.get_component<Positioned>()->get_position();
        int dx = pos1.x - pos2.x;
        int dy = pos1.y - pos2.y;

        return sqrt(dx * dx + dy * dy);
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
    }

    wstring& name() {
        if (!mods || !mods->is_identified() || mods->rarity == 0)   /* normal or unidentified items */
            return base_name();
        return mods->name(base_name());                             /* magic/rare/unique items */
    }

    wstring& base_name() {
        return base->name();
    }
};
