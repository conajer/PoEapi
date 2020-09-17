;
; poeapikit.ahk, 9/16/2020 7:41 PM
;

#SingleInstance force
#NoEnv ; Recommended for performance and compatibility with future AutoHotkey releases.
#Persistent ; Stay open in background
#IfWinActive, Path of Exile ahk_class POEWindowClass

SetWorkingDir %A_ScriptDir%

#Include, %A_ScriptDir%\lib\PoEapi.ahk
#Include, %A_ScriptDir%\Settings.ahk

DllCall("poeapi\poeapi_get_version", "int*", major_version, "int*", minor_version, "int*", patch)
debug("PoEapi-demo v0.1 (powered by PoEapi v{}.{}.{})", major_version, minor_version, patch)

global ptask := new PoETask()
ptask.activate()

Hotkey, ~%AttackSkillKey%, Attack
Hotkey, %QuickDefenseKey%, QuickDefense

; end of auto-execute section
return

dumpObj(obj, name = "", prefix = "") {
    if (Not IsObject(obj)) {
        debug("Not a object")
        return
    }

    if (name)
        debug(prefix "@" name)

    for k, v in obj {
        debug("{}   {}{}, {}", prefix, IsObject(v) ? "*" : " ", k, IsObject(v) ? v.Count() : v)
        if (IsObject(v))
            dumpObj(v,, prefix "    ")
    }
}

Attack:
    ptask.onAttack()
return

QuickDefense:
    SendInput, %QuickDefenseAction%
return

`::
    ptask.logout()
return

F1::
    SendInput, %AruasKey%
return

F5::
    ptask.sendKeys("/Hideout")
return

#d::
    WinMinimize, A
return

^m::
    ptask.toggleMaphack()
return

#IfWinActive

^r::
    Reload
return

^q::
    ExitApp
return

F10::
    ;vendor := new Vendor()
    ;vendor.selectNPC()
    ;obj := ptask.getNearestEntity("NPC")
    ptask.select("Stash")
    ;dumpObj(ptask.stashTabs)
    ;debug(&ptask.stashTabs)
    ;dumpObj(ptask)
    ptask.getInventories()
    ptask.getStashTabs()
    flasks := ptask.inventories[12]
    flasks.getItems()
    dumpObj(flasks)
    cFlask := flasks.Items[1].getComponent("Flask")
    cCharges := flasks.Items[1].getComponent("Charges")
    dumpObj(cCharges)
    dumpObj(cFlask)
    ;for i, f in flasks.Items
    ;    dumpObj(f, "Flask " i ":")
return
