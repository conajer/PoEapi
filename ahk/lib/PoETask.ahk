;
; PoETask.ahk, 9/16/2020 10:36 PM
;

#Include, %A_ScriptDir%\lib\PoEOffsets.ahk
#Include, %A_ScriptDir%\lib\WebGui.ahk
#Include, %A_ScriptDir%\lib\Navi.ahk
#Include, %A_ScriptDir%\lib\Hud.ahk
#include, %A_ScriptDir%\lib\Logger.ahk
#Include, %A_ScriptDir%\lib\Character.ahk

global levelExp := [525,1235,2021,3403,5002,7138,10053,13804,18512,24297,31516,39878,50352,62261,76465,92806,112027,133876,158538,187025,218895,255366,295852,341805,392470,449555,512121,583857,662181,747411,844146,949053,1064952,1192712,1333241,1487491,1656447,1841143,2046202,2265837,2508528,2776124,3061734,3379914,3723676,4099570,4504444,4951099,5430907,5957868,6528910,7153414,7827968,8555414,9353933,10212541,11142646,12157041,13252160,14441758,15731508,17127265,18635053,20271765,22044909,23950783,26019833,28261412,30672515,33287878,36118904,39163425,42460810,46024718,49853964,54008554,58473753,63314495,68516464,74132190,80182477,86725730,93748717,101352108,109524907,118335069,127813148,138033822,149032822,160890604,173648795,187372170,202153736,218041909,235163399,253547862,273358532,294631836,317515914]

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
                    if (item[key] < val[1] || item[key] >= val[2])
                        return false
                } else {
                    if val is number
                        if (item[key] != val)
                            return false

                    if val is not number
                        if (Not (item[key] ~= val))
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
        OnMessage(WM_PTASK_LOADED, ObjBindMethod(this, "onLoaded"))
        OnMessage(WM_PTASK_ATTACHED, ObjBindMethod(this, "onAttached"))
        OnMessage(WM_PTASK_ACTIVE, ObjBindMethod(this, "onActive"))
        OnMessage(WM_AREA_CHANGED, ObjBindMethod(this, "onAreaChanged"))
        OnMessage(WM_PLAYER_CHANGED, ObjBindMethod(this, "onPlayerChanged"))
        OnMessage(WM_PICKUP, ObjBindMethod(this, "onPickup"))
        OnMessage(WM_USE_SKILL, ObjBindMethod(this, "onUseSkill"))
        OnMessage(WM_PTASK_EXIT, ObjBindMethod(this, "onExit"))

        this.player := new Character()

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

        ; Start PoE task
        this.start()

        ; 'Create' Rules objects manually
        IdentifyExceptions.base := Rules
        VendorRules.base := Rules
        VendorExceptions.base := Rules
        StashRules.base := Rules
    }

    onLoaded() {
        this.reset()
    }

    onExit() {
        this.league := ""
        this.InHideout := false
        this.InMap := false
    }

    onAttached(hwnd) {
        if (Not hwnd) {
            ; PoE window closed.
            this.nav.close()
            this.hud.close()
            return
        }

        this.Hwnd := hwnd
        if (EnableAutoSize) {
            this.activate()
            WinGetPos, x, y, w, h, ahk_id %hwnd%
            if (EnableAutoSize && Not this.isMaximized()) {
                r := this.getWindowRect(DllCall("GetDesktopWindow"))
                x := (r.w > w) ? r.w - w + 8 : x
                y := 0
                h := (h > r.h - 50) ? h : r.h - 42

                WinMove, % "ahk_id " hwnd,, x, y, w, h
            }
        }

        this.nav := new Navi()
        this.c := this.nav.getCanvas()
        this.hud := new Hud()
        this.reset()
    }

    onActive(hwnd) {
        if (hwnd == this.Hwnd) {
            Sleep, 300
            WinGetPos, x, y, w, h, ahk_id %hwnd%
            this.x := x
            this.y := y
            this.width := w
            this.height := h
            this.actionArea := new Rect(210, 90, w - 450, h - 260)
            this.isActive := true

            this.nav.show()
            this.hud.show()
        } else {
            if (Not WinActive("ahk_class AutoHotkeyGUI")) {
                if (Not WinActive("ahk_id " this.Hwnd)) {
                    this.nav.hide()
                    this.hud.hide()
                }
            }
            this.hud.clear()
            this.isActive := false
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

    getClientRect(hwnd = "") {
        hwnd := hwnd ? hwnd : this.Hwnd
        VarSetCapacity(r, 16)
        DllCall("GetClientRect", "UInt", hwnd, "Ptr", &r)
        DllCall("ClientToScreen", "UInt", hwnd, "Ptr", &r)

        left := NumGet(r, 0, "Int")
        top := NumGet(r, 4, "Int")
        width := NumGet(r, 8, "Int")
        height := NumGet(r, 12, "Int")

        return new Rect(left, top, width, height)
    }

    getWindowRect(hwnd = "") {
        hwnd := hwnd ? hwnd : this.Hwnd
        VarSetCapacity(r, 16)
        DllCall("GetWindowRect", "UInt", hwnd, "Ptr", &r)

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

    sendKeys(keys, NoSend = false) {
        if (this.isMinimized())
            return

        this.activate()
        if (Not this.getChat().isOpened())
            keys := "{Enter}" keys

        if (NoSend)
            SendInput, %keys%
        else
            SendInput, %keys%{Enter}
    }

    beginPickup() {
        ap := this.getPlugin("AutoPickup")
        ap.stopPickup()
        ap.beginPickup()
    }

    stopPickup() {
        this.getPlugin("AutoPickup").stopPickup()
    }

    select(name) {
        if (Not WinActive("ahk_class POEWindowClass"))
            return

        this.selected := false
        this.target := this.getNearestEntity(name)
        if (Not this.target)
            return false

        vendor := this.getVendor()
        loop, 5 {
            if (this.selected || (this.target.path ~= "NPC" && vendor.isSelected())) {
                this.target := ""
                return true
            }

            this.target.getPos(x, y)
            clipToRect(this.actionArea, x, y)
            MouseClick(x, y)
            Sleep, 500
        }

        return false
    }

    sellItems(identifyAll = false) {
        this.activate()
        vendor := this.getVendor()
        if (Not vendor.sell())
            return

        for i, aItem in this.inventory.getItems() {
            if (Not aItem.isIdentified && (identifyAll || Not IdentifyExceptions.check(aItem))) {
                if (Not shift) {
                    SendInput {Shift down}
                    if (Not this.inventory.identify(aItem)) {
                        SendInput {Shift up}
                        return
                    }
                    shift := true
                } else {
                    this.inventory.identify(aItem, shift)
                }
            }
        }

        if (shift) {
            Sleep, 200
            SendInput {Shift up}
        }

        for i, aItem in this.inventory.getItems() {
            if (aItem.rarity == 0) {
                if (aItem.baseName ~= "(Divine|Eternal) Life") {
                    trans := this.inventory.findItem("Orb of Transmutation")
                    aItem := this.inventory.use(trans, aItem)
                } else if (aItem.baseName ~= "Two-Toned|Stygain|Convoking|Bone") {
                    alchemy := this.inventory.findItem("Orb of Alchemy")
                    aItem := this.inventory.use(alchemy, aItem)
                }
            }

            if (Not VendorExceptions.check(aItem) && VendorRules.check(aItem))
                this.inventory.move(aItem)
        }
        this.getSell().accept()
    }

    stashItems() {
        this.activate()
        if (Not this.stash.open())
            return

        Sleep, 100
        for i, aItem in this.inventory.getItems() {
            rule := StashRules.check(aItem)
            if (rule) {
                this.stash.switchTab(rule.tabName)
                this.inventory.move(aItem)
            }
        }
    }

    levelupGems() {
        MouseGetPos, oldX, oldY
        gems := this.getIngameUI().getChild(5, 2, 1)
        n := gems.getChilds().Count()
        loop, %n% {
            for i, e in gems.getChilds() {
                if (e.getChild(4).getText() == "Click to level up") {
                    e.getChild(2).getPos(x, y)
                    MouseClick(x, y)
                    m += 1
                    Sleep, 75
                    break
                }
            }

            if (m < A_Index)
                break
        }

        if (m > 0)
            MouseMove, oldX, oldY, 0
    }

    displayItemPrice(e, sum = false) {
        price := e.item.price
        if (sum && e.item.stackCount > 0)
            price := e.item.stackCount * price

        if (e.item && price >= 0.05) {
            if (price > 1000)
                p := Format("{:.f}k", price / 1000)
            else if (price < 1)
                p := Format("{:.g}", price)
            else
                p := Format("{:.f}", price)

            r := e.getRect()
            if (p >= 10)
                this.c.drawText(p, r.r, r.b, "white", "red", 2)
            else
                this.c.drawText(p, r.r, r.b, "#00007f", "gold", 2)
        }
    }

    showPrices() {
        stickyMode := false
        shift := GetKeyState("Shift")
        this.c.clear()
        if (this.stash.isOpened()) {
            for i, e in this.stash.Tab.getChilds() {
                if (e.isVisible())
                    this.displayItemPrice(e, shift)
            }
        }

        if (this.inventory.isOpened()) {
            for i, e in this.inventory.getChilds()
                this.displayItemPrice(e, shift)
        }

        favours := this.getFavours()
        if (favours.isOpened()) {
            for i, e in favours.getChilds() {
                e.item.price := $(e.item)
                this.displayItemPrice(e, true)
            }
        }

        purchase := ptask.getPurchase()
        if (purchase.isOpened()) {
            for i, e in purchase.getChilds() {
                e.item.price := $(e.item)
                this.displayItemPrice(e)
            }
        }

        if (prophesier.isOpened()) {
            for i, p in prophesier.getProphecies() {
                p.item := {"price": $(p.name)}
                this.displayItemPrice(p)
            }
        }

        loop, {
            if (GetKeyState("Ctrl"))
                stickyMode := true
            Sleep, 100
            if (Not GetKeyState("Alt")) {
                if (Not stickyMode)
                    this.c.clear()
                break
            }
        }
    }

    onAreaChanged(areaName, lParam) {
        Critical
        areaName := StrGet(areaName)
        level := lParam & 0xff
        isTown := (lParam & 0x100) || (areaName == _("The Rogue Harbour"))
        isHideout := RegExMatch(areaName, _("Hideout")) && (areaName != _("Syndicate Hideout"))

        rdebug("#area", _("You have entered") " <b style=""color:maroon"">{}, {}</b>", areaName, level)
        this.InMap := Not isTown && Not isHideout
        this.InHideout := isHideout
        Character.base := this.getPlayer()
    }

    onPlayerChanged(name) {
        syslog(this.player.whois())
    }

    onUseSkill(skill, target) {
        skill := StrGet(skill)
        if (skill == "Interactive") {
            if (this.target) {
                if (this.target.address == target)
                    this.selected := true
            }
        }
    }

    onAttack() {
        this.stopPickup()
        this.player.onAttack()
    }
}
