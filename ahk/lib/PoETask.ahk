;
; PoETask.ahk, 9/16/2020 10:36 PM
;

#Include, %A_ScriptDir%\lib\PoEOffsets.ahk
#Include, %A_ScriptDir%\lib\Banner.ahk
#Include, %A_ScriptDir%\lib\Canvas.ahk
#include, %A_ScriptDir%\lib\Logger.ahk
#Include, %A_ScriptDir%\lib\Character.ahk

global levelXP := [525,1235,2021,3403,5002,7138,10053,13804,18512,24297,31516,39878,50352,62261,76465,92806,112027,133876,158538,187025,218895,255366,295852,341805,392470,449555,512121,583857,662181,747411,844146,949053,1064952,1192712,1333241,1487491,1656447,1841143,2046202,2265837,2508528,2776124,3061734,3379914,3723676,4099570,4504444,4951099,5430907,5957868,6528910,7153414,7827968,8555414,9353933,10212541,11142646,12157041,13252160,14441758,15731508,17127265,18635053,20271765,22044909,23950783,26019833,28261412,30672515,33287878,36118904,39163425,42460810,46024718,49853964,54008554,58473753,63314495,68516464,74132190,80182477,86725730,93748717,101352108,109524907,118335069,127813148,138033822,149032822,160890604,173648795,187372170,202153736,218041909,235163399,253547862,273358532,294631836,317515914]

class Rect {

    __new(l, t, w = 0, h = 0, r = "", b = "") {
        this.l := l
        this.t := t

        if (w > 0 || r == "") {
            this.w := w
            this.r := l + w
        } else {
            this.r := r
            this.w := r - l
        }

        if (h > 0 || b == "") {
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

MouseClick(x, y, button = "Left") {
    MouseMove, x, y, 0
    Sleep, 20
    MouseClick, %button%, x, y
}

class Rules {

    __check(rule, item) {
        if ((rule.baseType && RegExMatch(item.baseType, rule.baseType))
            || (rule.baseName && RegExMatch(item.baseName, rule.baseName)))
        {
            for key, val in rule.constraints {

                if (IsObject(val)) {
                    if (item[key] < val[1] || item[key] > val[2])
                        return false
                } else if (Not (item[key] ~= val)) {
                    return false
                }
            }

            return true
        }
    }

    check(item) {
        for i, rule in this {
            if (this.__check(rule, item)) {
                return rule
            }
        }
    }
}

class PoETask extends AhkObj {

    __new() {
        base.__new()
        OnMessage(WM_POEAPI_LOG, ObjBindMethod(this, "onLog"))
        OnMessage(WM_PLAYER_CHANGED, ObjBindMethod(this, "playerChanged"))
        OnMessage(WM_AREA_CHANGED, ObjBindMethod(this, "areaChanged"))
        OnMessage(WM_HEIST_CHEST, ObjBindMethod(this, "onHeistChest"))
        OnMessage(WM_PICKUP, ObjBindMethod(this, "onPickup"))
        OnMessage(WM_PTASK_ATTACHED, ObjBindMethod(this, "onAttached"))

        this.useSkillHandler := ObjBindMethod(this, "onUseSkill")

        ; Start PoE task
        this.start()

        ; 'Create' Rules objects manually
        IdentifyExceptions.base := Rules
        VendorRules.base := Rules
        VendorExceptions.base := Rules
        StashRules.base := Rules
    }

    onAttached(hwnd) {
        if (Not hwnd) {
            ; PoE window was closed.
            this.c.destory()
            this.hud.destroy()
            this.setHud(-1)
            this.banner.destroy()
            return
        }

        this.Hwnd := hwnd
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

        this.x := x
        this.y := y
        this.width := w
        this.height := h
        this.actionArea := new Rect(210, 90, w - 450, h - 260)

        ; Update offsets
        for catalog, offsets in PoEOffsets.offsets {
            for key, value in offsets
                this.setOffset(catalog, key, value)
        }

        ; Configure plugins
        plugins := this.getPlugins()
        for name, options in PluginOptions {
            for key, value in options
                plugins[name][key] := value
        }

        if (EnableCanvas)
            this.c := new Canvas(hwnd)

        if (EnableHud) {
            this.hud := new Canvas(hwnd)
            this.setHudWindow(this.hud.hwnd)
        }

        if (EnableBanner)
            this.banner := new Banner(hwnd)
        this.activate()
        OnMessage(WM_PTASK_ACTIVE, ObjBindMethod(this, "onActive"))
    }

    onActive(hwnd) {
        if (hwnd == this.Hwnd && WinActive("ahk_id " this.Hwnd)) {
            Sleep, 300
            this.banner.show(true, false)
            this.hud.show(true, false)
        } else if (hwnd != this.banner.hwnd) {
            this.banner.show(false, false)
            this.c.clear()
            this.hud.show(false, false)
        }
    }

    activate() {
        if (WinActive("ahk_id " this.Hwnd))
            return

        if (InIdle) {
            ; Turn monitor on by moving mouse if it's off
            MouseMove, 5, 5, 0, R
            if (this.isMinimized())
                Sleep, 1000
        }

        WinActivate, % "ahk_id " this.Hwnd
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
        this.sendKeys("/exit")
    }

    maximize() {
        WinMaximize, ahk_class POEWindowClass
    }

    minimize() {
        WinMinimize, ahk_class POEWindowClass
    }

    getPlugin(name) {
        return this.getPlugins()[name]
    }

    sendKeys(keys, NoSend = false) {
        if (ptask.isMinimized())
            return

        ptask.activate()
        if (Not this.chat.isOpened())
            SendInput, {Enter}

        if (NoSend)
            SendInput, %keys%
        else
            SendInput, %keys%{Enter}
    }

    select(name) {
        if (Not WinActive("ahk_class POEWindowClass"))
            return

        this.selected := false
        this.entity := this.getNearestEntity(name)
        if (Not this.entity)
            return false

        OnMessage(WM_USE_SKILL, this.useSkillHandler)
        loop, 5 {
            if (this.selected)
                return true

            this.entity.getPos(x, y)
            clipToRect(this.actionArea, x, y)
            MouseClick(x, y)
            Sleep, 500
        }

        return false
    }

    sellItems(identifyAll = false) {
        vendor := this.getVendor()
        if (Not vendor.sell())
            return

        for i, item in this.inventory.getItems() {
            if (Not item.isIdentified() && (identifyAll || Not IdentifyExceptions.check(item))) {
                if (Not shift) {
                    SendInput {Shift down}
                    if (Not this.inventory.identify("")) {
                        SendInput {Shift up}
                        return
                    }
                    shift := true
                }

                this.inventory.identify(item, shift)
            }
        }

        if (shift) {
            Sleep, 200
            SendInput {Shift up}
        }

        for i, item in this.inventory.getItems() {
            if (item.rarity == 0 && item.baseName ~= "(Divine|Eternal) Life") {
                trans := this.inventory.findItem("Transmutation")
                this.inventory.use(trans, item)
            }

            if (Not VendorExceptions.check(item) && VendorRules.check(item))
                this.inventory.move(item)
        }
        ptask.getSell().accept()
    }

    stashItems() {
        if (Not this.stash.open())
            return

        Sleep, 100
        for i, item in this.inventory.getItems() {
            rule := StashRules.check(item)
            if (rule) {
                this.stash.switchTab(rule.tabName)
                this.inventory.move(item)
            }
        }
    }

    levelupGems() {
        l := this.width - 150
        t := 200
        r := this.width - 50
        b := this.height - 300

        MouseGetPos, oldX, oldY
        loop {
            ImageSearch, x, y, l, t, r, b, *30 *TransBlack %A_ScriptDir%\images\level_up_gem.bmp
            if (ErrorLevel != 0)
                break
            MouseClick(x + 40, y + 8)
            n += 1
            Sleep, 100
        }

        if (n > 0)
            MouseMove, oldX, oldY, 0
    }

    areaChanged(areaName, lParam) {
        Critical
        areaName := StrGet(areaName)
        level := lParam & 0xff
        isTown := (lParam & 0x100) || (areaName ~= _("Azurite Mine") "|" _("The Rogue Harbour"))
        isHideout := RegExMatch(areaName, _("Hideout")) && (areaName != _("Syndicate Hideout"))

        debug(_("You have entered") " <b style=""color:maroon"">{}, {}</b>", areaName, level)
        this.InMap := Not isTown && Not isHideout
        this.InHideout := isHideout

        this.player := this.getPlayer()
        playerName := this.player.name
        this.player.base := IsObject(%playerName%) ? %playerName% : Character
        this.player.__new()

        ; Calculate gained experience
        if (this.InMap) {
            if (Not this.savedXP)
                this.savedXP := this.player.getXP()
        } else if (this.savedXP) {
            currentXP := this.player.getXP()
            if (currentXP != this.savedXP) {
                gainedXP := currentXP - this.savedXP
                this.savedXP := 0
                lvl := this.player.level

                syslog(_("{:.2f}% experience gained"), gainedXP * 100 / levelXP[lvl])
            }
        }

        this.getInventorySlots()
        this.getStashTabs()
        this.getStash()
        this.getInventory()
        this.flasks := this.inventories[12]
        this.chat := this.getChat()
    }

    playerChanged(name) {
        syslog(this.player.whois())
    }

    onHeistChest(chestName, lParam) {
        chestName := StrGet(chestName)
        if (chestName) {
            chest := {}
            chest.name := chestName
            chest.x := lParam << 32 >> 48
            chest.y := lParam << 48 >> 48
            this.heistChests.Push(chest)
            return
        }

        this.c.beginPaint()
        this.c.clear()
        for i, chest in this.heistChests {
            x := chest.x
            y := chest.y
            if (RegExMatch(chest.name, HeistChestNameRegex, matched))
                this.c.drawText(x - 100, y + 50, 200, 30, matched2, 0xff00ff)
        }
        this.c.endPaint()

        this.heistChests := {}
    }

    onPickup(x, y) {
        MouseClick, Left, x, y
    }

    onUseSkill(skill, target) {
        skill := StrGet(skill)
        if (skill == "Interactive") {
            if (this.entity && this.entity.address == target) {
                this.selected := true
                OnMessage(WM_USE_SKILL, this.useSkillHandler, 0)
            }
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
