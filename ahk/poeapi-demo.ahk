#SingleInstance force
#NoEnv ; Recommended for performance and compatibility with future AutoHotkey releases.
#Persistent ; Stay open in background

SetWorkingDir %A_ScriptDir%
#Include, %A_ScriptDir%\ahkpp.ahk
#include, %A_ScriptDir%\logger.ahk

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
OnMessage(0x900e, "OnKilled")

apiLib := DllCall("LoadLibrary", "str", "poeapi.dll", "ptr")
if (Not apiLib) {
    Msgbox, % "PoEapi library not loaded!"
    ExitApp
}

DllCall("poeapi\poeapi_get_version", "int*", major_version, "int*", minor_version, "int*", patch)
debug("PoEapi-demo v0.1 (powered by PoEapi v{}.{}.{})", major_version, minor_version, patch)

global __Classes := { "RemoteMemoryObject" : RemoteMemoryObject
                    , "Component"          : Component
                    , "Entity"             : Entity }

global __Entities := {}

class RemoteMemoryObject extends AhkObj {
    
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

OnUseSkill(skillName, targetPtr) {
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
    rdebug("#KILLED", "Killed: <b>{}</b>/{}", killed, total)
}

dumpObj(prefix, obj) {
    for k, v in obj {
        debug(prefix k ", " v)
        if (IsObject(v))
            dumpObj(prefix "    ", v)
    }
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
