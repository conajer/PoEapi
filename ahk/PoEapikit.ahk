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

SetBatchLines, -1
SetWorkingDir, %A_ScriptDir%
CoordMode, Mouse, Client
CoordMode, Pixel, Client

#Include, %A_ScriptDir%\lib\PoEapi.ahk
#Include, %A_ScriptDir%\lib\ajax.ahk
#Include, %A_ScriptDir%\lib\curl.ahk
#Include, %A_ScriptDir%\lib\sqlite3.ahk
#Include, %A_ScriptDir%\lib\LocalDB.ahk
#Include, %A_ScriptDir%\Settings.ahk

global CloseAllUIKey, InventoryKey, DropFlareKey, DropDynamiteKey
global language := "en"

readIni("production_Config.ini")
DllCall("msvcrt\_putenv", "AStr", "LANG=" language)
DllCall("AddFontResource", "Str", A_ScriptDir "\fonts\Fontin-SmallCaps.ttf")
DllCall("poeapi\poeapi_get_version", "int*", major_version, "int*", minor_version, "int*", patchlevel)

global db := new LocalDB("local.db")
global ptask := new PoETask()

global version := "1.6.0"
global poeapiVersion := Format("{}.{}.{}", major_version, minor_version, patchlevel)
syslog("<b>PoEapikit v{} (" _("Powered by") " PoEapi v{})</b>", version, poeapiVersion)

Hotkey, IfWinActive, ahk_class POEWindowClass
Hotkey, ~%AttackSkillKey%, Attack
if (SecondaryAttackSkillKey)
    Hotkey, ~%SecondaryAttackSkillKey%, Attack
Hotkey, $%QuickDefenseKey%, QuickDefense
Hotkey, ~%AutoPickupKey%, AutoPickup
Hotkey, IfWinActive

#Include, %A_ScriptDir%\extras\vendor.ahk
#Include, %A_ScriptDir%\extras\Pricer.ahk
#Include, %A_ScriptDir%\extras\Trader.ahk
#Include, %A_ScriptDir%\extras\Updater.ahk

; Patreon only features
#Include, %A_ScriptDir%\patreon.ahk

OnExit("__Exit")

; end of auto-execute section
return

readIni(iniFile) {
    EnvGet, homepath, USERPROFILE
    FileRead, production_config, %HOMEPATH%\Documents\My Games\Path of Exile\%iniFile%
    production_config := SubStr(production_config, 1)
    FileAppend, %production_config%, %A_ScriptDir%\%iniFile%
    IniRead, close_panels, %A_ScriptDir%\%iniFile%, ACTION_KEYS, close_panels
    IniRead, open_inventory_panel, %A_ScriptDir%\%iniFile%, ACTION_KEYS, open_inventory_panel
    IniRead, use_temporary_skill1, %A_ScriptDir%\%iniFile%, ACTION_KEYS, use_temporary_skill1
    IniRead, use_temporary_skill2, %A_ScriptDir%\%iniFile%, ACTION_KEYS, use_temporary_skill1
    IniRead, language, %A_ScriptDir%\%iniFile%, LANGUAGE, language, en
    FileDelete, %A_ScriptDir%\%iniFile%

    CloseAllUIKey := Chr(close_panels)
    InventoryKey := Chr(open_inventory_panel)
    DropFlareKey := Chr(use_temporary_skill1)
    DropDynamiteKey := Chr(use_temporary_skill2)
}

_(str) {
    if (language == "en")
        return str
    if (not __translations.hasKey(str))
        __translations[str] := (pos := InStr(str, "/"))
            ? _(SubStr(str, 1, pos - 1)) "/" _(SubStr(str, pos + 1)) : str
    return __translations[str]
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
    global __libs := {}

    h := DllCall("LoadLibrary", "Str", filename, "Ptr")
    __libs[filename] := h

    return h
}

Attack:
    if (ptask.InMap)
        ptask.player.onAttack()
    sleep, 300
return

QuickDefense:
    if (ptask.getChat().isOpened())
        SendInput, {q}
    else
        SendInput, %QuickDefenseAction%
return

~s::
    ptask.levelupGems()
return

AutoPickup:
    ptask.beginPickup()
return

OnClipboardChange:
    if (ptask.isActive)
        return

    if (RegExMatch(Clipboard, "^@([^ ]+) (Hi, (I would|I'd) like to buy your .*)", matched)) {
        if (GetKeyState("Ctrl") && Not GetKeyState("c")) {
            SendInput, {Ctrl up}
            ptask.activate()
            ptask.sendKeys("^{v}") 
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

~RButton::
    if (ptask.InMap)
        return

    loop, 50 {
        if (item := ptask.getHoveredItem())
            break
        SLeep, 10
    }

    if (item.rarity == 0 || item.rarity == 3) {
        price := $(item)
        if (Not price)
            return

        loop, 10 {
            e := ptask.getIngameUI().getChild(141, 1)
            tag := e.getChild(1, 2, 2, 1)
            if (tag.isVisible() && not tag.getText()) {
                exalted := $("Exalted Orb")
                if (price >= 2 * exalted)
                    note := Format("~b/o {:.1f} exalted", price / exalted)
                else if (price >= 1)
                    note := Format("~b/o {} chaos", Round(price))
                else if (item.stackSize() > 1)
                    note := Format("~b/o {}/{} chaos", 10, Round(10 / price))
                else
                    note := Format("~b/o {:.1f} chaos", price)
                SendInput, %note%
                break
            }
            Sleep, 50
        }
    }
return

^WheelDown::SendInput {Right}
^WheelUp::SendInput {Left}

#d::
    WinMinimize, A
return

^m::
    ptask.toggleMaphack()
return

+-::_
-::NumpadSub
+::NumpadAdd

~*LAlt::
    stickyMode := false
    ptask.c.clear()
    if (betrayer.isOpened())
        betrayer.show()
    else if (not ptask.getHoveredItem())
        ptask.showPrices()

    loop, {
        if (item := ptask.getHoveredItem()) {
            if (item.name && item != hoveredItem) {
                ptask.c.clear()
                item.price := $(item)
                ptask.showPrices(hoveredItem := item)
            }
        }

        if (GetKeyState("Ctrl"))
            stickyMode := true
        Sleep, 100
        if (Not GetKeyState("Alt")) {
            if (Not stickyMode)
                ptask.c.clear()
            break
        }
    }
return

~^c::
~^+c::
    Sleep, 100
    if (item := ptask.getHoveredItem()) {
        if (GetKeyState("Shift")) {
            MsgBox, 0, Item Info, %Clipboard%
        } else {
            Clipboard := item.name
        }
    }
return

~^f::
    if (ptask.stash.isOpened()) {
        if (item := ptask.getHoveredItem()) {
            name := item.name
            SendInput, %name%{Enter}
        }
    }
return

^w::
    if (item := ptask.getHoveredItem()) {
        if (Not item.isIdentified || item.rarity ~= "1|2")
            name := item.baseName
        else
            name := item.name
        Run, % "https://pathofexile.fandom.com/wiki/" RegExReplace(name, " ", "_")
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
