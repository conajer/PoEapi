;
; PoETask.ahk, 9/16/2020 10:36 PM
;

#Include, %A_ScriptDir%\lib\Banner.ahk
#Include, %A_ScriptDir%\lib\Canvas.ahk
#include, %A_ScriptDir%\lib\Logger.ahk
#Include, %A_ScriptDir%\lib\Character.ahk

global levelXP := [525,1235,2021,3403,5002,7138,10053,13804,18512,24297,31516,39878,50352,62261,76465,92806,112027,133876,158538,187025,218895,255366,295852,341805,392470,449555,512121,583857,662181,747411,844146,949053,1064952,1192712,1333241,1487491,1656447,1841143,2046202,2265837,2508528,2776124,3061734,3379914,3723676,4099570,4504444,4951099,5430907,5957868,6528910,7153414,7827968,8555414,9353933,10212541,11142646,12157041,13252160,14441758,15731508,17127265,18635053,20271765,22044909,23950783,26019833,28261412,30672515,33287878,36118904,39163425,42460810,46024718,49853964,54008554,58473753,63314495,68516464,74132190,80182477,86725730,93748717,101352108,109524907,118335069,127813148,138033822,149032822,160890604,173648795,187372170,202153736,218041909,235163399,253547862,273358532,294631836,317515914]

class Rect {

    __new(l, t, w = 0, h = 0, r = 0, b = 0) {
        this.l := l
        this.t := t

        if (w > 0) {
            this.w := w
            this.r := l + w
        } else {
            this.r := r
            this.w := r - l
        }

        if (h > 0) {
            this.h := h
            this.b := t + h
        } else {
            this.b := b
            this.h := b - t
        }
    }
}

pointInRect(r, x, y) {
    return (x > r.l && y > r.t && x < r.r && y < r.b)
}

clipToRect(r, ByRef x, ByRef y) {
    oldX := x
    oldY := y

    x := (x < r.l) ? r.l : ((x > r.r) ? r.r : x)
    y := (y < r.t) ? r.t : ((y > r.b) ? r.b : y)

    return (oldX != x) || (oldY != y)
}

MouseClick(x, y, Button = "Left") {
    MouseMove, x, y, 0
    Sleep, 20
    MouseClick, %Button%, x, y
}


class PoETask extends AhkObj {

    __new() {
        base.__new()
        OnMessage(WM_POEAPI_LOG, ObjBindMethod(this, "onLog"))
        OnMessage(WM_PLAYER_CHANGED, ObjBindMethod(this, "playerChanged"))
        OnMessage(WM_AREA_CHANGED, ObjBindMethod(this, "areaChanged"))
        OnMessage(WM_DELVE_CHEST, ObjBindMethod(this, "onDelveChest"))
        OnMessage(WM_PICKUP, ObjBindMethod(this, "onPickup"))
        OnMessage(WM_PTASK_ATTACHED, ObjBindMethod(this, "attach"))

        this.setGenericItemFilter(genericItemFilter)
        this.setRareItemFilter(rareItemFilter)
        this.useSkillHandler := ObjBindMethod(this, "onUseSkill")

        ; Start PoE task
        this.start()
    }

    attach(hwnd) {
        if (Not hwnd) {
            ; PoE window was closed.
            this.c.destory()
            this.banner.destroy()
            return
        }

        this.activate()
        WinGetPos, x, y, w, h, ahk_id %hwnd%
        if (Not this.isMaximized()) {
            r := this.getWindowRect(DllCall("GetDesktopWindow"))
            x := (r.w > w) ? r.w - w + 8 : x
            y := 0
            h := (h > r.h - 50) ? h : r.h - 42

            WinMove, ahk_class POEWindowClass,, x, y, w, h
            WinGetPos, x, y, w, h, ahk_class POEWindowClass
        }

        this.Hwnd := hwnd
        this.width := w
        this.height := h
        this.actionArea := new Rect(225, 125, w - 450, h - 250)

        if (EnableCanvas)
            this.c := new Canvas(hwnd)
        if (EnableBanner)
            this.banner := new Banner(hwnd)
        this.activate()
    }

    __delete() {
        this.stop()
        DllCall("FreeLibrary", "Ptr", poeapi)
        debug("Exiting...")
    }

    activate() {
        if (WinActive("ahk_class POEWindowClass"))
            return

        if (InIdle) {
            ; Turn monitor on by moving mouse if it's off
            MouseMove, 5, 5, 0, R
            if (this.isMinimized())
                Sleep, 1000
        }

        WinActivate, ahk_class POEWindowClass
    }

    getWindowRect(hwnd) {
        VarSetCapacity(r, 16)
        DllCall("GetWindowRect", "UInt", hwnd, "UInt", &r)

        left := NumGet(r, 0, "Int")
        top := NumGet(r, 4, "Int")
        right := NumGet(r, 8, "Int")
        bottom := NumGet(r, 12, "Int")

        return new Rect(left, top,,, right, bottom)
    }

    isMinimized() {
        WinGet, state, MinMax, ahk_class POEWindowClass
        return (state == -1)
    }

    isMaximized() {
        WinGet, state, MinMax, ahk_class POEWindowClass
        return (state == 1)
    }

    logout() {
        WinGetPos, x, y, w, h, % "ahk_id " this.Hwnd
        SendInput, {Esc}
        MouseClick, Left, w / 2, 440
    }

    maximize() {
        WinMaximize, ahk_class POEWindowClass
    }

    minimize() {
        WinMinimize, ahk_class POEWindowClass
    }

    sendKeys(keys) {
        this.activate()
        SendInput {Enter}%keys%{Enter}
    }

    select(name) {
        if (Not WinActive("ahk_class POEWindowClass"))
            return

        this.selected := false
        entity := this.getNearestEntity(name)
        if (Not entity)
            return false

        OnMessage(WM_USE_SKILL, this.useSkillHandler)
        loop, 10 {
            if (this.selected)
                return true

            pos := entity.getPos()
            x := NumGet(pos + 0x0, "Int")
            y := NumGet(pos + 0x4, "Int")
            clipToRect(this.actionArea, x, y)
            MouseClick(x, y)
            Sleep, 500
        }

        return false
    }

    sellItems() {
        this.select("NPC")
    }

    areaChanged(areaName, lParam) {
        Critical
        areaName := StrGet(areaName)
        level := lParam & 0xff
        isTown := lParam & 0x100
        isHideout := RegExMatch(areaName, "Hideout$") && (areaName != "Syndicate Hideout")

        debug("You have entered <b style=""color:maroon"">{}, {}</b>", areaName, level)
        this.InMap := Not isTown && Not isHideout

        ; Calculate gained experience
        if (this.InMap) {
            if (Not this.savedXP)
                this.savedXP := this.getXP()
        } else if (this.savedXP) {
            currentXP := this.getXP()
            if (currentXP != this.savedXP) {
                gainedXP := currentXP - this.savedXP
                this.savedXP := 0
                lvl := this.player.level

                syslog(Format("{:.2f}% experience gained.", gainedXP * 100 / levelXP[lvl]))
            }
        }

        this.getInventorySlots()
        this.getStashTabs()
        this.getStash()
        this.getInventory()
        this.flasks := this.inventories[12]
    }

    playerChanged(playerName, level) {
        this.player := IsObject(%playerName%) ? new %playerName%() : new Character()
        this.player.Level := level
        syslog("{} is level {} in the {} league", StrGet(playerName), level, this.League)
    }

    onDelveChest(chestName, lParam) {
        chestName := StrGet(chestName)
        if (RegExMatch(chestName, IgnoredChests))
            return

        if (chestName) {
            chest := {}
            chest.name := chestName
            chest.x := lParam << 32 >> 48
            chest.y := lParam << 48 >> 48
            this.delveChests.Push(chest)
            return
        }

        this.c.clear()
        for i, chest in this.delveChests {
            x := chest.x
            y := chest.y
            if (RegExMatch(chest.name, "AzuriteVein"))
                cIndicator := 0xff0000
            else if (RegExMatch(chest.name, "Resonator"))
                cIndicator := 0xff7f
            else if (RegExMatch(chest.name, "Fossil"))
                cIndicator := 0xffff
            else if (RegExMatch(chest.name, "Currency|Map"))
                cIndicator := 0xffffff
            else if (RegExMatch(chest.name, "SuppliesDynamite"))
                cIndicator := 0x7f
            else if (RegExMatch(chest.name, "SuppliesFlares"))
                cIndicator := 0xff
            else if (RegExMatch(chest.name, "Unique"))
                cIndicator := 0xffff00
            else if (RegExMatch(chest.name, "DelveWall"))
                cIndicator := 0xff00ff
            else 
                cIndicator := 0x7f7f7f

            clipToRect(this.actionArea, x, y)
            this.c.drawRect(x - 15, y - 50, 30, 30, cIndicator, 10)
            this.banner.display(path)
        }

        this.delveChests := {}
    }

    onPickup(x, y) {
        MouseClick, Left, x, y
    }

    onUseSkill(skill, target) {
        skill := StrGet(skill)
        if (skill == "Interactive") {
            this.selected := true
            OnMessage(WM_USE_SKILL, this.useSkillHandler, 0)
        }
    }

    onAttack() {
        this.stopPickup()
        this.player.onAttack()
    }

    onLog(text) {
        if (ShowLogMessage)
            rsyslog("#PoEapi", "<b>{}</b>", StrGet(text))
    }
}
