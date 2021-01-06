;
; PoEapikit.ahk, 9/16/2020 7:41 PM
;

#SingleInstance force
#NoEnv ; Recommended for performance and compatibility with future AutoHotkey releases.
#Persistent ; Stay open in background
#IfWinActive, Path of Exile ahk_class POEWindowClass

CoordMode, Mouse, Client
CoordMode, Pixel, Client
SetWorkingDir %A_ScriptDir%

#Include, %A_ScriptDir%\lib\PoEapi.ahk
#Include, %A_ScriptDir%\extras\Trader.ahk
#Include, %A_ScriptDir%\Settings.ahk

If (Not A_IsAdmin) {
    try {
        Run *RunAs "%A_AhkPath%" "%A_ScriptFullPath%"
        ExitApp
    }
}

EnvGet, homepath, USERPROFILE
FileRead, production_config, %HOMEPATH%\Documents\My Games\Path of Exile\production_Config.ini
production_config := SubStr(production_config, 1)
FileAppend, %production_config%, %A_ScriptDir%\production_config.ini
IniRead, close_panels, %A_ScriptDir%\production_config.ini, ACTION_KEYS, close_panels
IniRead, open_inventory_panel, %A_ScriptDir%\production_config.ini, ACTION_KEYS, open_inventory_panel
IniRead, use_temporary_skill1, %A_ScriptDir%\production_config.ini, ACTION_KEYS, use_temporary_skill1
IniRead, use_temporary_skill2, %A_ScriptDir%\production_config.ini, ACTION_KEYS, use_temporary_skill1
IniRead, language, %A_ScriptDir%\production_config.ini, LANGUAGE, language, en
FileDelete, %A_ScriptDir%\production_config.ini

global CloseAllUIKey := Chr(close_panels)
global InventoryKey := Chr(open_inventory_panel)
global DropFlareKey := Chr(use_temporary_skill1)
global DropDynamiteKey := Chr(use_temporary_skill2)

DllCall("LoadLibrary", "Str", "libintl-8.dll")
DllCall("msvcrt\_putenv", "AStr", "LANG=" language)
DllCall("libintl-8\bindtextdomain", "AStr", "PoEapikit", "AStr", "./locale")
DllCall("libintl-8\textdomain", "AStr", "PoEapikit")
DllCall("AddFontResource", "Str", A_ScriptDir "\fonts\Fontin-SmallCaps.ttf")
DllCall("poeapi\poeapi_get_version", "int*", major_version, "int*", minor_version, "int*", patchlevel)

global logger := new Logger("PoEapikit log",,, Not ShowLogger)
global ptask := new PoETask()
global trader := new Trader()

version := "0.6.0"
poeapiVersion := Format("{}.{}.{}", major_version, minor_version, patchlevel)
syslog("<b>PoEapikit v{} (" _("Powered by") " PoEapi v{})</b>", version, poeapiVersion)

Hotkey, IfWinActive, ahk_class POEWindowClass
Hotkey, ~%AttackSkillKey%, Attack
if (SecondaryAttackSkillKey)
    Hotkey, ~%SecondaryAttackSkillKey%, Attack
Hotkey, $%QuickDefenseKey%, QuickDefense
Hotkey, ~%AutoPickupKey%, AutoPickup
Hotkey, IfWinActive

; end of auto-execute section
return

_(str) {
    return DllCall("libintl-8\gettext", "AStr", str, "AStr")
}

objdump(obj, prefix = "", depth = 0) {
    if (Not IsObject(obj)) {
        debug("Not a object")
        return
    }

    baseClasses := ""
    base := obj.base
    loop {
        if (Not base)
            break
        baseClasses .= " -> " base.__Class
        base := base.base
    }

    if (obj.base)
        debug("{}{:#x}{}", prefix, &obj, baseClasses)

    for k, v in obj {
        debug("{}   {}{}, {}", prefix, IsObject(v) ? "*" : " ", k, IsObject(v) ? v.Count() : v)
        if (depth > 0 && IsObject(v))
            objdump(v, prefix "    ", depth - 1)
    }
}

Attack:
    ptask.onAttack()
return

QuickDefense:
    SendInput, %QuickDefenseAction%
return

AutoPickup:
    ptask.levelupGems()
    ptask.beginPickup()
return

AutoClick() {
    MouseGetPos, x0, y0
    Loop {
        if (Not GetKeyState("Ctrl", "P"))
            break

        MouseGetPos, x, y
        if (abs(x - x0) > 100 || abs(y - y0) > 100)
            break

        x0 := x
        y0 := y
        SendInput ^{Click}
        Sleep, 30
    }
}

~LButton::
    if (GetKeyState("LButton", "P"))
        ptask.stopPickup()
return

`::
    ptask.logout()
return

F1::
    SendInput, %AruasKey%
return

F2::
    ptask.inventory.openPortal()
return

F3::
    ptask.sellItems()
return

!F3::
    ptask.sellItems(true)
return

F4::
    ptask.stashItems()
return

F5::
    ptask.sendKeys("/hideout")
return

^F5::
    ptask.sendKeys("/delve")
return

!F5::
    ptask.sendKeys("/menagerie")
return

~^LButton::
    If (A_PriorHotKey = A_ThisHotKey and A_TimeSincePriorHotkey < 200)
        SetTimer, AutoClick, -200
return

#d::
    WinMinimize, A
return

^m::
    ptask.toggleMaphack()
return

+-::_
-::NumpadSub
+::NumpadAdd

^WheelDown::Right
^WheelUp::Left

#IfWinActive

^r::
Reload:
    DllCall("RemoveFontResource", "Str", A_ScriptDir "\fonts\Fontin-SmallCaps.ttf")
    Reload
return

^q::
    DllCall("RemoveFontResource", "Str", A_ScriptDir "\fonts\Fontin-SmallCaps.ttf")
    ExitApp
return

~^c::
    Sleep, 100
    if (Clipboard) {
        if (SubStr(Clipboard, 1, 7) == "Rarity:")
            MsgBox, 0, Item Info, %clipboard%
    }
return

^t::
    MouseGetPos tempX, tempY
    PixelGetColor, bgr, tempX, tempY
    MsgBox, % "Width:" ptask.Width " Hieght:" ptask.Height "`nX=" tempX " Y=" tempY "`nColor=" bgr
return

class PassiveSkill extends Element {

    __init() {
        address := this.getPtr(this.address + 0x1b0)
        this.u1 := this.getByte(this.address + 0x380)

        address := this.getPtr(address + 0x10)
        this.id := this.getInt(address + 0x30)
        address := this.getPtr(address + 0x34)
        this.name := this.getString(address, 64)
    }

    toString() {
        return Format("{:x}: {}{}, {:x}, {:x}, {:x}", this.address, this.u1 ? "*" : "", this.name, this.u1, this.u2, this.u3)
    }
}

F12::
    logger.show(!logger.isVisible())
return
