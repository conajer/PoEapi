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

; Hotkeys (^ for Ctrl, ! for Alt, + for Shift, # for window key)
;
;                           Enabled Prefix Key Name     Label              Description
;                           ------- ------ -----------  -----------------  --------------------------------------
global defaultHotkeys := [ [ true,  "",    "``",        "ExitGame",        "Exit to character selection"]
                         , [ true,  "~",   "a",         "AutoPickup",      "Pickup nearby items"]
                         , [ true,  "$",   "q",         "QuickDefense",    "Quick defence actions"]
                         , [ true,  "~",   "s",         "LevelupGems",     "Level up skill gems"]
                         , [ true,  "~",   "w",         "Attack",          "Main attack skill"]
                         , [ false, "~",   "RButton",   "Attack",          "Secondary attack skill"]
                         , [ true,  "",    "#d",        "MinimizeWindow",  "Minimize PoE window"]
                         , [ true,  "",    "F1",        "AutoAuras",       "Auto aruas (ALT+F1 for performance stats)"]
                         , [ true,  "",    "F2",        "OpenPortal",      "Open portal"]
                         , [ true,  "*",   "F3",        "SellItems",       "Auto identify and sell items"]
                         , [ true,  "",    "F4",        "StashItems",      "Stash items"]
                         , [ true,  "",    "F5",        "Hideout",         "Enter hideout"]
                         , [ true,  "",    "^F5",       "Delve",           "Enter mine encampment"]
                         , [ true,  "",    "!F5",       "Menagerie",       "Enter menagerie"]
                         , [ true,  "",    "F12",       "ShowLog",         "Show log window"]
                         , [ true,  "~*",  "LAlt",      "ShowPrices",      "Show price of the item(s) in stash tab, inventory, etc."]
                         , [ true,  "",    "^LButton",  "AutoCtrlClick",   "Hold to activate auto CTRL clicker"]
                         , [ true,  "~",   "+LButton",  "AutoShiftClick",  "Hold to activate auto SHIFT clicker"]
                         , [ true,  "~",   "^RButton",  "AutoFillPrice",   "Auto fill the price tag of the selected item"]
                         , [ true,  "~*",  "^c",        "CopyItemName",    "Copy the selected item's name"]
                         , [ true,  "~",   "^f",        "HighlightItems",  "Highlight items in stash tab"]
                         , [ true,  "",    "^m",        "ToggleMaphack",   "Toggle maphack"]
                         , [ true,  "",    "^w",        "OpenWiki",        "Open wiki"]
                         , [ true,  "",    "^r",        "Reload",          "Reload script"]
                         , [ true,  "",    "^q",        "ExitApp",         "Quit PoEapikit"] ]

global CloseAllUIKey, InventoryKey, DropFlareKey, DropDynamiteKey
global language := "en"

readIni("production_Config.ini")
DllCall("msvcrt\_putenv", "AStr", "LANG=" language)
DllCall("AddFontResource", "Str", A_ScriptDir "\fonts\Fontin-SmallCaps.ttf")
DllCall("poeapi\poeapi_get_version", "int*", major_version, "int*", minor_version, "int*", patchlevel)

global db := new LocalDB("local.db")
loadHotkeys()
global ptask := new PoETask()

global version := "1.7.3"
global poeapiVersion := Format("{}.{}.{}", major_version, minor_version, patchlevel)
syslog("<b>PoEapikit v{} (" _("Powered by") " PoEapi v{})</b>", version, poeapiVersion)

#Include, %A_ScriptDir%\extras\Archnemsis.ahk
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

loadHotkeys() {
    try {
        hotkeyOptions := db.exec("SELECT * FROM hotkeys;")
    } catch {
    } finally  {
        if (Not hotkeyOptions) {
            db.exec("
            (
                DROP TABLE IF EXISTS hotkeys;
                CREATE TABLE hotkeys (
                    id INTEGER PRIMARY KEY,
                    enabled INTEGER,
                    prefix TEXT,
                    name TEXT,
                    label TEXT,
                    description TEXT);
            )")

            for i, hotkey in defaultHotkeys
                db.exec("INSERT INTO hotkeys VALUES ({}, {}, '{}', '{}', '{}', ""{}"");", i, hotkey*)
            hotkeyOptions := db.exec("SELECT * FROM hotkeys;")
        }
    }

    Hotkey, IfWinActive, ahk_class POEWindowClass
    for i, hotkey in hotkeyOptions {
        try {
            if (hotkey.enabled)
                Hotkey, % hotkey.prefix . hotkey.name, % hotkey.label, On
        } catch e {
            hotkey.enabled := false
            db.exec("INSERT OR REPLACE INTO hotkeys VALUES ({}, {}, '{}', '{}', '{}', ""{}"");"
                    , i, hotkey.enabled, hotkey.prefix, hotkey.name, hotkey.label, hotkey.description)
            MsgBox, % "Hotkey error: " e.message
            return
        }
    }

    return hotkeyOptions
}

saveHotkeys(hotkeyOptions) {
    oldHotkeys := db.exec("SELECT * FROM hotkeys;")
    Hotkey, IfWinActive, ahk_class POEWindowClass
    for i, hotkey in hotkeyOptions {
        if (oldHotkeys[i].enabled)
            Hotkey, % oldHotkeys[i].prefix . oldHotkeys[i].name, Off
        db.exec("INSERT OR REPLACE INTO hotkeys VALUES ({}, {}, '{}', '{}', '{}', ""{}"");"
                , i, hotkey.enabled, hotkey.prefix, hotkey.name, hotkey.label, hotkey.description)
    }

    ; reload the hotkeys
    return loadHotkeys()
}

Attack:
    if (ptask.InMap)
        ptask.player.onAttack()
return

QuickDefense:
    if (ptask.InMap && Not ptask.getChat().isOpened())
        SendInput, %QuickDefenseAction%
    else
        SendInput, % SubStr(A_ThisHotkey, 0)
return

LevelupGems:
    ptask.levelupGems()
return

AutoPickup:
    ptask.beginPickup()
return

OnClipboardChange:
    if (ptask.isActive)
        return

    if (GetKeyState("Ctrl") && Not GetKeyState("c")) {
        if (__trader.checkMessage(Clipboard)) {
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

ExitGame:
    ptask.logout()
return

AutoAuras:
    SendInput, %AruasKey%
return

OpenPortal:
    ptask.inventory.openPortal()
return

SellItems:
    ptask.sellItems(GetKeyState("Alt"))
return

StashItems:
    ptask.stashItems()
return

Hideout:
    ptask.sendKeys("/hideout")
return

Delve:
    ptask.sendKeys("/delve")
return

Menagerie:
    ptask.sendKeys("/menagerie")
return

AutoCtrlClick:
    If (A_PriorHotKey = A_ThisHotKey and A_TimeSincePriorHotkey < 200)
        clickerEnabled := true
    SendInput, ^{Click}
    SetTimer, AutoClick, -200
return

AutoShiftClick:
    SetTimer, AutoClick, -200
return

AutoFillPrice:
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
            e := ptask.getIngameUI().getChild(143, 1)
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

+-::_
-::NumpadSub
+::NumpadAdd

MinimizeWindow:
    WinMinimize, A
return

ToggleMaphack:
    ptask.toggleMaphack()
return

ShowPrices:
    ptask.c.clear()
    loop, {
        if (Betrayer.isOpened())
            Betrayer.show()
        else if (Temple.isOpened())
            Temple.show()
        else if (archnemesis.isOpened()) {
            archnemesis.show()
        } else {
            item := ptask.getHoveredItem()
            if (item.name) {
                if (item != hoveredItem) {
                    ptask.c.clear()
                    item.price := $(item)
                    hoveredItem := item
                }
            } else if (hoveredItem) {
                ptask.c.clear()
                hoveredItem := ""
            }
            ptask.showPrices(hoveredItem)
        }

        if (Not GetKeyState("Alt")) {
            if (Not GetKeyState("Ctrl")) {
                ptask.c.clear()
                archnemesis.hide()
            }
            break
        }
        Sleep, 200
    }
return

CopyItemName:
    Sleep, 100
    if (item := ptask.getHoveredItem()) {
        if (GetKeyState("Shift")) {
            MsgBox, 0, Item Info, %Clipboard%
        } else {
            Clipboard := item.name
        }
    }
return

HighlightItems:
    name := (item := ptask.getHoveredItem()) ? item.name : ""
    if (name && ptask.stash.isOpened())
        SendInput, %name%{Enter}

    ptask.c.clear()
    if (name && ptask.inventory.isOpened()) {
        for i, item in ptask.inventory.findItems(name)
            ptask.inventory.highlight(item)
    }
return

OpenWiki:
    if (item := ptask.getHoveredItem()) {
        if (Not item.isIdentified || item.rarity ~= "1|2")
            name := item.baseName
        else
            name := item.name
        Run, % "https://www.poewiki.net/wiki/" RegExReplace(name, " ", "_")
    }
return

ShowLog:
    logger.show()
return

Reload() {
    Reload
}

ExitApp() {
    ExitApp
}
