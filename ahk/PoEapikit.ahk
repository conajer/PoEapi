;
; PoEapikit.ahk, 9/16/2020 7:41 PM
;

#SingleInstance force
#NoEnv ; Recommended for performance and compatibility with future AutoHotkey releases.
#Persistent ; Stay open in background
#IfWinActive, Path of Exile ahk_class POEWindowClass

If (Not A_IsAdmin) {
    try {
        Run *RunAs "%A_AhkPath%" "%A_ScriptFullPath%"
        ExitApp
    }
}

SetWorkingDir %A_ScriptDir%
CoordMode, Mouse, Client
CoordMode, Pixel, Client

#Include, %A_ScriptDir%\lib\PoEapi.ahk
#Include, %A_ScriptDir%\lib\ajax.ahk
#Include, %A_ScriptDir%\Settings.ahk

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

loadLibrary("libintl-8.dll")
DllCall("msvcrt\_putenv", "AStr", "LANG=" language)
DllCall("libintl-8\bindtextdomain", "AStr", "PoEapikit", "AStr", "./locale")
DllCall("libintl-8\textdomain", "AStr", "PoEapikit")
DllCall("AddFontResource", "Str", A_ScriptDir "\fonts\Fontin-SmallCaps.ttf")
DllCall("poeapi\poeapi_get_version", "int*", major_version, "int*", minor_version, "int*", patchlevel)

global logger := new Logger("PoEapikit log")
global ptask := new PoETask()

global version := "1.3.1"
global poeapiVersion := Format("{}.{}.{}", major_version, minor_version, patchlevel)
syslog("<b>PoEapikit v{} (" _("Powered by") " PoEapi v{})</b>", version, poeapiVersion)

Hotkey, IfWinActive, ahk_class POEWindowClass
Hotkey, ~%AttackSkillKey%, Attack
if (SecondaryAttackSkillKey)
    Hotkey, ~%SecondaryAttackSkillKey%, Attack
Hotkey, $%QuickDefenseKey%, QuickDefense
Hotkey, ~%AutoPickupKey%, AutoPickup
Hotkey, IfWinActive

#Include, %A_ScriptDir%\extras\vendoring.ahk
#Include, %A_ScriptDir%\extras\Pricer.ahk
#Include, %A_ScriptDir%\extras\Trader.ahk
#Include, %A_ScriptDir%\extras\Updater.ahk

; Patreon only features
#Include, %A_ScriptDir%\patreon.ahk

OnExit("__Exit")

; end of auto-execute section
return

_(str) {
    return DllCall("libintl-8\gettext", "AStr", str, "AStr")
}

$(item) {
    return pricer.getPrice(item)
}

__Exit() {
    global __libs

    ptask.stop()
    DllCall("RemoveFontResource", "Str", A_ScriptDir "\fonts\Fontin-SmallCaps.ttf")
    for filename, h in __libs
        DllCall("FreeLibrary", "Ptr", h)
}

loadLibrary(filename) {
    global __libs

    if (Not __libs)
        __libs := {}

    h := DllCall("LoadLibrary", "Str", filename, "Ptr")
    __libs[filename] := h

    return h
}

objdump(obj, prefix = "", depth = 0) {
    if (Not IsObject(obj)) {
        debug("Not an object")
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

    if (Not prefix)
        debug("{}{:#x}{}:", prefix, &obj, baseClasses)
    for k, v in obj {
        try {
            debug("{}   {}{}, {}", prefix, IsObject(v) ? "*" : " ", k, IsObject(v) ? v.Count() : v)
            if (depth > 0 && IsObject(v))
                objdump(v, prefix "    ", depth - 1)
        } catch {}
    }
}

Attack:
    if (ptask.InMap)
        ptask.onAttack()
return

QuickDefense:
    SendInput, %QuickDefenseAction%
return

~s::
    ptask.levelupGems()
return

AutoPickup:
    ptask.beginPickup()
return

OnClipboardChange:
    if (RegExMatch(Clipboard, "^@([^ ]+) (Hi, (I would|I'd) like to buy your .*)", matched)) {
        if (GetKeyState("Ctrl") && Not GetKeyState("c")) {
            SendInput, {Ctrl up}
            ptask.activate()
            ptask.sendKeys(Clipboard) 
        }
    }
return

AutoClick() {
    global clickerEnabled

    MouseGetPos, x0, y0
    Loop {
        if (Not clickerEnabled && Not GetKeyState("LButton", "P"))
            break
        if (Not GetKeyState("Ctrl", "P") && Not GetKeyState("Shift", "P"))
            break

        MouseGetPos, x, y
        if (abs(x - x0) > 100 || abs(y - y0) > 100)
            break

        x0 := x
        y0 := y
        if (GetKeyState("Shift", "P")) {
            SendInput, +{Click}
            Sleep, 100
        } else {
            SendInput, ^{Click}
        }
        Sleep, 30
    }
    clickerEnabled := false
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

~+LButton::
    SetTimer, AutoClick, -200
return

^LButton::
    If (A_PriorHotKey = A_ThisHotKey and A_TimeSincePriorHotkey < 200)
        clickerEnabled := true
    SendInput, ^{Click}
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

^WheelDown::SendInput {Right}
^WheelUp::SendInput {Left}

~*LAlt::
    ptask.showPrices()
return

~^c::
~^+c::
    Sleep, 100
    if (Clipboard) {
        if (SubStr(Clipboard, 1, 11) == "Item Class:") {
            if (GetKeyState("Shift")) {
                MsgBox, 0, Item Info, %clipboard%
            } else {
                RegExMatch(Clipboard, "Rarity: (.+)\r\n(.+)\r\n", matched)
                Clipboard := matched2
            }
        }
    }
return

*^f::
    if (ptask.stash.isOpened()) {
        Clipboard := ""
        SendInput, ^{c}
        SendInput, ^{f}
        Sleep, 100
        if (Clipboard) {
            if (SubStr(Clipboard, 1, 11) == "Item Class:") {
                RegExMatch(Clipboard, "Rarity: (.+)\r\n(.+)\r\n", matched)
                SendInput, "%matched2%"{Enter}
            }
        }
    }
return

^w::
    Clipboard := ""
    SendInput, ^{c}
    Sleep, 100
    if (Clipboard) {
        if (SubStr(Clipboard, 1, 11) == "Item Class:") {
            RegExMatch(Clipboard, "Rarity: (.+)\r\n(.+)\r\n(.+)\r\n", matched)
            name := matched2
            if (matched1 ~= "Magic|Rare" && Not RegExMatch(Clipboard, "Unidentified"))
                name := matched3
            Run, % "https://pathofexile.fandom.com/wiki/" RegExReplace(name, " ", "_")
        }
    }
return

^t::
    MouseGetPos tempX, tempY
    PixelGetColor, bgr, tempX, tempY
    MsgBox, % "Width:" ptask.Width " Hieght:" ptask.Height "`nX=" tempX " Y=" tempY "`nColor=" bgr
return

#IfWinActive

^r::
Reload() {
    Reload
}

^q::
ExitApp() {
    ExitApp
}

F12::
    logger.show()
return
