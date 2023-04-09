;
; PoEapikit.ahk, 9/16/2020 7:41 PM
;

#SingleInstance force
#NoEnv ; Recommended for performance and compatibility with future AutoHotkey releases.
#Persistent ; Stay open in background
#IfWinActive, Path of Exile ahk_class POEWindowClass

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
loadHotkeys()
global ptask := new PoETask()

global version := "1.11.1a"
global poeapiVersion := Format("{}.{}.{}", major_version, minor_version, patchlevel)
syslog("<b>PoEapikit v{} (" _("Powered by") " PoEapi v{})</b>", version, poeapiVersion)

#Include, %A_ScriptDir%\extras\vendor.ahk
#Include, %A_ScriptDir%\extras\Pricer.ahk
#Include, %A_ScriptDir%\extras\Trader.ahk
#Include, %A_ScriptDir%\extras\Updater.ahk

; Start PoE task
ptask.start()

; Patreon only features
Sleep, 500
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
    IniRead, use_temporary_skill2, %A_ScriptDir%\%iniFile%, ACTION_KEYS, use_temporary_skill2
    IniRead, language, %A_ScriptDir%\%iniFile%, LANGUAGE, language, en
    FileDelete, %A_ScriptDir%\%iniFile%

    CloseAllUIKey := Chr(close_panels)
    InventoryKey := Chr(open_inventory_panel)
    DropFlareKey := Chr(use_temporary_skill1)
    DropDynamiteKey := Chr(use_temporary_skill2)
}

_(str) {
    if (Not ObjHasKey(__translations, str)) {
        if (Not pos := InStr(str, "/"))
            return str
        __translations[str] := _(SubStr(str, 1, pos - 1)) "/" _(SubStr(str, pos + 1))
    }

    return __translations[str]
}

$(item) {
    return pricer.getPrice(item)
}

$$(price, rate = 1, symbol = "") {
    return pricer.format(price, rate, symbol)
}

__Exit() {
    global __libs

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
    if (ptask.InMap)
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
    MouseGetPos, x0, y0
    keys := GetKeyState("Ctrl") ? "^" : ""
    keys .= GetKeyState("Shift") ? "+" : ""
    if (GetKeyState("LButton", "P")) {
        button := "LButton"
        keys .= "{Click}"
    } else {
        button := "RButton"
        keys .= "{Click, Right}"
    }

    loop, {
        if (Not GetKeyState(button, "P"))
            break
        if (Not GetKeyState("Ctrl") && Not GetKeyState("Shift"))
            break

        SendInput, %keys%
        Sleep, 50
    }
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
    if (ptask.InHideout || ptask.getEntities("/Stash").count() > 0)
        ptask.sellItems(GetKeyState("Alt"))
    else
        SendInput, % "{" RegExReplace(A_ThisHotkey, "\*") "}"
return

StashItems:
    if (ptask.InHideout || ptask.getEntities("/Stash").count() > 0)
        ptask.stashItems()
    else
        SendInput, % "{" A_ThisHotkey "}"
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
    if (GetKeyState("Shift", P))
        SendInput, ^+{Click}
    else if (GetKeyState("Alt", P))
        SendInput, ^!{Click}
    else
        SendInput, ^{Click}
    SetTimer, AutoClick, -200
return

AutoShiftClick:
    SetTimer, AutoClick, -200
return

AutoRButtonClick:
    SetTimer, AutoClick, -200
return

AutoFillPrice:
    loop, 50 {
        if (item := ptask.getHoveredItem())
            break
        SLeep, 10
    }

    SendInput, {RButton}
    if (Not price := $(item))
        return

    loop, 5 {
        Sleep, 100
        e := ptask.getIngameUI().getChild(142, 1)
        if (e.isVisible()) {
            tag := e.getChild(1, 2, 2, 1)
            if (tag.isVisible()) {
                if (price > $divine)
                    note := Format("~b/o {:.1f} divine", price / $divine)
                else
                    note := Format("~b/o {:.f} chaos", price)
                SendInput, %note%
            } else if (RegExMatch(tag.getText(), "~(b/o|price) ([0-9./]+) (.+)", matched)) {
                tag := e.getChild(1, 2, 3, 1)
                tag.getPos(x, y)
                MouseMove, x,  y, 0
                SendInput, {Click}{Click}
                if (matched3 == "divine") {
                    price := Round(price / $divine, 1)
                } else if (matched3 == "exalted") {
                    if (price < .3 * exalted)
                        price := Format("1/{}", Round($exalted / price))
                    else
                        price := Round(price / $exalted, 1)
                } else if (matched3 == "chaos") {
                    if (price >= 1)
                        price := Round(price)
                    else if (item.stackSize() > 1)
                        price := Format("{}/{}", 10, Round(10 / price))
                    else
                        price := Round(price, 1)
                }

                SendInput, %price%
            }
            break
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
    stickyMode := false
    ptask.c.clear()
    loop, {
        if (Betrayer.isOpened())
            Betrayer.show()
        else if (Temple.isOpened())
            Temple.show()
        else {
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

        if (GetKeyState("Ctrl"))
            stickyMode := true
        if (Not GetKeyState("Alt")) {
            if (Not stickyMode) {
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
    if (item := ptask.getHoveredItem()) {
        name := (item.rarity == 1 || item.rarity == 2) ? item.baseName : item.name
        if (ptask.stash.isOpened())
            SendInput, %name%{Enter}

        ptask.c.clear()
        if (ptask.inventory.isOpened()) {
            for i, item in ptask.inventory.findItems(name)
                ptask.inventory.highlight(item)
        }
    }
return

OpenWiki:
    if (item := ptask.getHoveredItem()) {
        if (Not item.isIdentified || item.rarity ~= "1|2")
            name := RegExReplace(item.baseName, "Synthesised ", "")
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
