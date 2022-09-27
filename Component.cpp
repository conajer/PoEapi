/*
* Component.cpp, 8/6/2020 9:02 PM
*/

#include <functional>

static FieldOffsets component_offsets {
    {"owner", 0x8},
};

class Component : public PoEObject {
private:

    static std::map<string, std::function<Component* (addrtype)>> component_table;

public:
  
    string type_name;

    Component(addrtype address, const string& name = "", FieldOffsets* offsets = &component_offsets)
        : PoEObject(address, offsets)
    {
    }

    addrtype owner_address() {
        return PoEMemory::read<addrtype>(address + 0x8);
    }

    void __new() {
        PoEObject::__new();
        __set(L"typeName", type_name.c_str(), AhkString, 0);
    }

    void to_print() {
        printf("%llx: %s", address, type_name.c_str());
    }
};

#include "components/Actor.cpp"
#include "components/Animated.cpp"
#include "components/Base.cpp"
#include "components/Buffs.cpp"
#include "components/CapturedMonster.cpp"
#include "components/Charges.cpp"
#include "components/Chest.cpp"
#include "components/Flask.cpp"
#include "components/HarvestSeed.cpp"
#include "components/HeistBlueprint.cpp"
#include "components/HeistContract.cpp"
#include "components/HeistRewardDisplay.cpp"
#include "components/Life.cpp"
#include "components/Map.cpp"
#include "components/MinimapIcon.cpp"
#include "components/Mods.cpp"
#include "components/Monster.cpp"
#include "components/NPC.cpp"
#include "components/ObjectMagicProperties.cpp"
#include "components/Player.cpp"
#include "components/PlayerClass.cpp"
#include "components/Positioned.cpp"
#include "components/Prophecy.cpp"
#include "components/Quality.cpp"
#include "components/Render.cpp"
#include "components/SkillGem.cpp"
#include "components/Sockets.cpp"
#include "components/Stack.cpp"
#include "components/Stats.cpp"
#include "components/Targetable.cpp"
#include "components/TriggerableBlockage.cpp"
#include "components/WorldItem.cpp"
