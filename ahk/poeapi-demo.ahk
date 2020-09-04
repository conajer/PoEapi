#SingleInstance force
#NoEnv ; Recommended for performance and compatibility with future AutoHotkey releases.
#Persistent ; Stay open in background

SetWorkingDir %A_ScriptDir%

#include %A_ScriptDir%\Logger.ahk

global logger := new Logger("PoEapi-demo log")

; Load PoEapi library
if (FileExist("..\poeapi.dll")) {
    FileMove ..\poeapi.dll, poeapi.dll, true
}

global League, AreaName
global Life, Mana

; PoEapi related event handlers
OnMessage(0x9000, "OnLog")
OnMessage(0x9001, "OnPlayerChanged")
OnMessage(0x9002, "OnLeagueChanged")
OnMessage(0x9003, "OnLifeChanged")
OnMessage(0x9004, "OnManaChanged")
OnMessage(0x9005, "OnEnergyShieldChanged")
OnMessage(0x9006, "OnDied")
OnMessage(0x9007, "OnUseSkill")
OnMessage(0x9008, "OnMove")
;OnMessage(0x9009, "OnBuffAdded")
;OnMessage(0x900a, "OnBuffRemoved")
OnMessage(0x900b, "OnAreaChanged")
OnMessage(0x900c, "OnMonsterChanged")
OnMessage(0x900d, "OnMinionChanged")
;OnMessage(0x900e, "OnKilled")

apiLib := DllCall("LoadLibrary", "str", "poeapi.dll", "ptr")
if (Not apiLib) {
    Msgbox, % "No PoEapi library found!"
    ExitApp
}

DllCall("poeapi\poeapi_get_version", "int*", major_version, "int*", minor_version, "int*", patch)
debug("PoEapi-demo v0.1 (powered by PoEapi v{}.{}.{})", major_version, minor_version, patch)

global __Classes := { "RemoteMemoryObject" : RemoteMemoryObject
                    , "Component"          : Component
                    , "Entity"             : Entity
                    , "Actor"              : Actor
                    , "Life"               : Life
                    , "Player"             : Player
                    , "Positioned"         : Positioned
                    , "Render"             : Render
                    , "Targetable"         : Targetable }

global __Entities := {}

class RemoteMemoryObject {

    __new(address) {
    }

    read(address, size) {
        return DllCall("poeapi\ahkobj_read", "Ptr", address, "int", size, "Ptr")
    }

    getByte(address) {
        dataPtr := DllCall("poeapi\ahkobj_read", "Ptr", address, "Char", 1, "Ptr")
        return NumGet(dataPtr + 0, "Char")
    }

    getShort(address) {
        dataPtr := DllCall("poeapi\ahkobj_read", "Ptr", address, "Short", 2, "Ptr")
        return NumGet(dataPtr + 0, "Short")
    }

    getInt(address) {
        dataPtr := DllCall("poeapi\ahkobj_read", "Ptr", address, "Int", 4, "Ptr")
        return NumGet(dataPtr + 0, "Int")
    }

    getFloat(address) {
        dataPtr := DllCall("poeapi\ahkobj_read", "Ptr", address, "Int", 4, "Ptr")
        return NumGet(dataPtr + 0, "Float")
    }

    getPtr(address) {
        dataPtr := DllCall("poeapi\ahkobj_read", "Ptr", address, "Int", 8, "Ptr")
        return NumGet(dataPtr + 0, "Ptr")
    }

    getString(address, len) {
        dataPtr := this.read(address, (len + 1) * 2)
        return StrGet(dataPtr + 0)
    }

    getAString(address, len) {
        dataPtr := this.read(address, len + 1)
        return StrGet(dataPtr + 0, "utf-8")
    }

    readString(address, len = 0) {
        len := len > 0 ? len : this.getInt(address + 0x10)
        address := this.getPtr(address)
        dataPtr := this.read(address, (len + 1) * 2)
        return StrGet(dataPtr + 0)
    }
}

class Component extends RemoteMemoryObject {
}
class Entity extends RemoteMemoryObject {
}
class Actor extends Component {
}
class Life extends Component {
    Life {
        Get {
            return this.getInt(this.address + 0x25c)
        }
    }
}
class Player extends Component {
    Name {
        Get {
            this.Name := this.readString(this.address + 0x158)
            return this.Name
        }
    }
}
class Positioned extends Component {
}
class Render extends Component {
}
class Targetable extends Component {
}

__New(className) {
    className := StrGet(className, "utf-8")
    if (!__Classes[className]) {
        obj := {}
    } else {
        obj := new __Classes[className]
        obj.__new()
    }

    if (className == "Entity")
        __Entities[&obj] := obj

    return Object(obj)
}

__Delete(obj) {
    ;__Entities[&obj] = ""
    ObjRelease(obj)
}

__Get(obj, key) {
    value := Object(obj)[StrGet(key, "utf-8")]
    return IsObject(value) ? Object(value) : value
}

__Set(obj, key, params*) {
    params := NumGet(params + 0, "Ptr")
    offset := 0
    ;while (key) {
        key := StrGet(key, "utf-8")
        switch NumGet(params + offset + 8, "Int") {
        case 0: ; Int
            Object(obj)[key] := NumGet(params + offset, "Int")
        case 1: ; String
            Object(obj)[key] := StrGet(NumGet(params + offset, "Ptr"), "utf-8")
        case 2: ; Unicode  String
            Object(obj)[key] := StrGet(NumGet(params + offset, "Ptr"))
        case 3: ; Float
            Object(obj)[key] := NumGet(params + offset, "Double")
        case 4: ; Object
            objPtr := NumGet(params + offset, "Ptr")
            Object(obj)[key] := objPtr ? Object(objPtr) : {}
        case 5: ; Ptr
            Object(obj)[key] := NumGet(params + offset, "Ptr")
        }
        key := NumGet(params + offset + 16, "Ptr")
        offset += 24
    ;}
}

__Call(obj, method, args) {
    Object(obj).__Call(StrGet(method, "utf-8"), args)
}

DllCall("poeapi\ahkobj_set_callbacks"
    , "Ptr", RegisterCallback("__New")
    , "Ptr", RegisterCallback("__Delete", "F")
    , "Ptr", RegisterCallback("__Get", "F")
    , "Ptr", RegisterCallback("__Set", "F")
    , "Ptr", RegisterCallback("__Call", "F"))

; end of auto-execute section
return


OnLog(message) {
    rsyslog("#PoEapi", "<b>{}</b>", StrGet(message))
}

OnPlayerChanged(playerName, level) {
    debug("{} is level {} in the {} league", StrGet(playerName), level, League)
}

OnLeagueChanged(leagueName) {
    League := StrGet(leagueName)
}

OnLifeChanged(current, lParam) {
    Life := current
    maximum := lParam & 0xffff
    reserved := lParam >> 16
    rdebug("#LIFE", "<b style=""color:red"">Life: {}/{}</b>", Life, maximum - reserved)
}

OnManaChanged(current, lParam) {
    Mana := current
    maximum := lParam & 0xffff
    reserved := lParam >> 16
    rdebug("#MANA", "<b style=""color:blue"">Mana: {}/{}</b>", Mana, maximum - reserved)
}

OnEnergyShieldChanged(current, maximum) {
    ES := current
    rdebug("#ES", "<b style=""color:dodgerblue"">Energy Shield: {}/{}</b>", ES, maximum)
}

OnDied() {
    debug("died")
}

dumpObj(prefix, obj) {
    for k, v in obj {
        debug(prefix k ", " v)
        if (IsObject(v))
            dumpObj(prefix "    ", v)
    }
}

OnUseSkill(skillName, targetPtr) {
    entity := Object(targetPtr)
    debug("using {}, {}", StrGet(skillName), entity.Name)
    dumpObj("", entity)
}

OnMove() {
}

OnBuffAdded() {
}

OnBuffRemoved() {
}

OnAreaChanged(name, level) {
    AreaName := StrGet(name)
    debug("Entered <b style=""color:maroon"">{}, {}</b>", AreaName, level)
    __Objects := {}
}

OnMinionChanged(numOfMinions) {
    rdebug("#MINIONS", "<b>{}</b> minions", numOfMinions)
}

OnMonsterChanged(numOfMonsters, charges) {
    rdebug("#MONSTERS", "<b>{}</b> monsters, <b>{}</b> charges", numOfMonsters, charges)
}

OnKilled(killed, total) {
    debug("{}/{}", killed, total)
}

^r::
    DllCall("FreeLibrary", "ptr", apiLib)
    reload
return

F10::
    for a, obj in __Entities
        debug("{:02x}: {}", a, obj.Name)
    debug("Total {} objects", __Entities.Count())
return
