/*
* RemoteMemoryObject.cpp, 8/5/2020 2:32 PM
*/

#include <functional>
#include <map>
#include <unordered_map>

using FieldOffsets = std::map<string, int>;
template <typename T> using Factory = std::unordered_map<string, std::function<T* (addrtype)>>;

static FieldOffsets default_offsets = {
};

class RemoteMemoryObject : public PoEMemory { //, public AhkObjStub {
private:

    static Factory<RemoteMemoryObject> remote_object_factory;

protected:

    AhkObj* ahkobj = nullptr;
    
    virtual AhkObj* get_ahkobj(RemoteMemoryObject& obj) {
        if (!ahkobj) {
            ahkobj = new AhkObjStub(obj, address);
            ahkobj->set("Address", address, AHK_PTR, 0);
        }

        return ahkobj;
    }

public:

    addrtype address;
    FieldOffsets* offsets;
    int verbose = 1;

    RemoteMemoryObject(addrtype address, FieldOffsets* offsets = &default_offsets)
        : address(address), offsets(offsets)
    {
    }

    ~RemoteMemoryObject() {
        delete ahkobj;
        ahkobj = nullptr;
    }

    bool operator==(RemoteMemoryObject& obj) {
        return address == obj.address;
    }

    bool operator!=(RemoteMemoryObject& obj) {
        return address != obj.address;
    }

    template<typename T> T read(const string& field_name) {
        if (offsets->find(field_name) != offsets->end())
            return PoEMemory::read<T>(address + (*offsets)[field_name]);

        return T();
    }

    template<typename T> T read(const string& sub_name, const string& field_name) {
        if (offsets->find(sub_name) != offsets->end()) {
            addrtype addr = PoEMemory::read<addrtype>(address + (*offsets)[sub_name]);
            if (addr && offsets->find(field_name) != offsets->end())
                return PoEMemory::read<T>(addr + (*offsets)[field_name]);
        }

        return T();
    }

    template<typename T> T read(const std::vector<string> field_names) {
        addrtype addr = address;
        int last_index = field_names.size() - 1;

        for (int i = 0; i <= last_index; ++i) {
            if (!addr || offsets->find(field_names[i]) == offsets->end())
                break;

            if (i == last_index)
                return PoEMemory::read<T>(addr + (*offsets)[field_names[i]]);
            addr = PoEMemory::read<addrtype>(addr + (*offsets)[field_names[i]]);
        }

        return T();
    }

    template<typename T> std::vector<T> read_array(const string& field_name, int element_size) {
        if (offsets->find(field_name) != offsets->end())
            return PoEMemory::read_array<T>(address + (*offsets)[field_name], element_size);
        return std::vector<T>();
    }

    template<typename T> std::vector<T> read_array(const string& field_name, int offset, int element_size) {
        if (offsets->find(field_name) != offsets->end())
            return PoEMemory::read_array<T>(address + (*offsets)[field_name], offset, element_size);
        return std::vector<T>();
    }

    template <typename T> T* read_object(const string& name, addrtype address) {
        auto i = remote_object_factory.find(name);
        if (i != remote_object_factory.end())
            return dynamic_cast<T*>((i->second)(address));
        return new T(address);
    }

    virtual operator AhkObj*() {
        return get_ahkobj(*this);
    }

    virtual void to_print() {
        printf("%llx:", address);
    }
};

ostream& operator<<(ostream& os, RemoteMemoryObject& obj)
{
    obj.to_print();
    return os;
}

wostream& operator<<(wostream& os, RemoteMemoryObject& obj)
{
    obj.to_print();
    return os;
}

#include "Component.cpp"
#include "Entity.cpp"
#include "AreaTemplate.cpp"
#include "InGameUI.cpp"
#include "InGameData.cpp"
#include "ServerData.cpp"
#include "GameState.cpp"

#include "components/Actor.cpp"

#define NEW_ENTRY(T) {#T, [](addrtype address) { return new T(address);}}

Factory<RemoteMemoryObject> RemoteMemoryObject::remote_object_factory = {
    NEW_ENTRY(InGameState),
    NEW_ENTRY(Actor),
    NEW_ENTRY(Base),
    NEW_ENTRY(Charges),
    NEW_ENTRY(Chest),
    NEW_ENTRY(Flask),
    NEW_ENTRY(HarvestSeed),
    NEW_ENTRY(Life),
    NEW_ENTRY(Map),
    NEW_ENTRY(MinimapIcon),
    NEW_ENTRY(Mods),
    NEW_ENTRY(Monster),
    NEW_ENTRY(NPC),
    NEW_ENTRY(ObjectMagicProperties),
    NEW_ENTRY(Player),
    NEW_ENTRY(Positioned),
    NEW_ENTRY(Prophecy),
    NEW_ENTRY(Quality),
    NEW_ENTRY(Render),
    NEW_ENTRY(SkillGem),
    NEW_ENTRY(Sockets),
    NEW_ENTRY(Stack),
    NEW_ENTRY(Targetable),
    NEW_ENTRY(WorldItem),
};
