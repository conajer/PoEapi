/*
* Component.cpp, 8/6/2020 9:02 PM
*/

#include <functional>

static FieldOffsets component_offsets {
    {"owner", 0x8},
};

class Component : public RemoteMemoryObject {
private:

    static std::map<string, std::function<Component* (addrtype)>> component_table;

protected:

    AhkObj* get_ahkobj(RemoteMemoryObject& obj) {
        if (!ahkobj) {
            ahkobj = RemoteMemoryObject::get_ahkobj(obj);
            ahkobj->set("TypeName", type_name.c_str(), AHK_ASTR, 0);
        }

        return ahkobj;
    }

public:
  
    string type_name;

    Component(addrtype address, const string& name = "", FieldOffsets* offsets = &component_offsets)
        : RemoteMemoryObject(address, offsets)
    {
    }

    addrtype owner_address() {
        return PoEMemory::read<addrtype>(address + 0x8);
    }

    void to_print() {
        printf("%llx: %s", address, type_name.c_str());
    }
};

#include "components/Base.cpp"
#include "components/Charges.cpp"
#include "components/Chest.cpp"
#include "components/Flask.cpp"
#include "components/HarvestSeed.cpp"
#include "components/Life.cpp"
#include "components/Map.cpp"
#include "components/MinimapIcon.cpp"
#include "components/Mods.cpp"
#include "components/Monster.cpp"
#include "components/NPC.cpp"
#include "components/ObjectMagicProperties.cpp"
#include "components/Player.cpp"
#include "components/Positioned.cpp"
#include "components/Prophecy.cpp"
#include "components/Quality.cpp"
#include "components/Render.cpp"
#include "components/SkillGem.cpp"
#include "components/Sockets.cpp"
#include "components/Stack.cpp"
#include "components/Targetable.cpp"
#include "components/WorldItem.cpp"
