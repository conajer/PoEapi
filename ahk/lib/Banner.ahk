;
; Banner.ahk, 9/14/2020 11:43 AM
;

#Include, %A_ScriptDir%\extras\debug.ahk
#Include, %A_ScriptDir%\extras\vendoring.ahk

class AhkGui {

    __var(name) {
        return "__obj" &this "_" name
    }

    __Get(key) {
        if (Not this.HasKey(key)) {
            var := this.__var(key)
            return (%var%)
        }
    }

    __event_wrapper() {
        global
L1:
        RegExMatch(A_GuiControl, "__obj([0-9]+)_(.*)", matched)
        obj := Object(matched1)
        obj[matched2]()  ; Call object's method
        return
    }
}

class Banner extends AhkGui {

    __new(ownerHwnd) {
        global

        WinGetPos, x, y, w, h, ahk_id %ownerHwnd%
        y := (y < 0) ? -5 : 0
        Gui, __banner:New, -Caption +Owner%ownerHwnd% +HwndHwnd +LastFound
        Gui, Margin, 0, 0
        Gui, Color, White
        Gui, Font, cRed bold, Fontin SmallCaps
        Gui, Add, Text, % "y+5s w60 Hwnd" this.__var("Life"), Life: 100`%
        Gui, Font, cBlue bold
        Gui, Add, Text, % "ys w90 Hwnd" this.__var("Mana"), Mana: 100`%
        if (ShowEnergyShield) {
            Gui, Font, c1e90ff bold
            Gui, Add, Text, % "ys w150 Hwnd" this.__var("EnergyShield"), Energy Shield: 100`%
        }
        Gui, Font, cBlack bold
        Gui, Add, Text, % "ys w100 Hwnd" this.__var("Kills"), Kills: 0/0
        Gui, Font, cPurple bold
        Gui, Add, Text, % "ys x+10 w200 Hwnd" this.__var("Statusbar")

        Gui, Add, Button, % "x+5 y0 gL1 v" this.__var("hideout"), Hideout
        Gui, Add, Button, % "x+5 y0 gL1 v" this.__var("sellItems"), Sell
        Gui, Add, Button, % "x+1 y0 gL1 v" this.__var("stashItems"), Stash
        Gui, Add, Button, % "x+1 y0 gL1 v" this.__var("reload"), Reload

        ; Extras buttons
        Gui, Add, Text, x+10
        addVendorButton()
        addDebugButton()

        Gui, Show, % "x" x + 150 "y" y + 6

        OnMessage(WM_PLAYER_LIFE, ObjBindMethod(this, "lifeChanged"))
        OnMessage(WM_PLAYER_MANA, ObjBindMethod(this, "manaChanged"))
        OnMessage(WM_PLAYER_ES, ObjBindMethod(this, "energyShieldChanged"))
        OnMessage(WM_KILL_COUNTER, ObjBindMethod(this, "onKillCounter"))
    }

    destroy() {
        Gui, __banner:Destroy
    }

    display(text = "", duration = 15000) {
        if (Not text) {
            GuiControl,, % this.Display
            return
        }

        GuiControl,, % this.Statusbar, > %text%
        fn := ObjBindMethod(this, "display")
        SetTimer, %fn%, % -duration
    }

    hideout() {
        ptask.activate()
        ptask.sendKeys("/Hideout")
    }

    sellItems() {
        ptask.activate()
        ptask.sellItems()
    }

    stashItems() {
        ptask.activate()
        ptask.stashItems()
    }


    reload() {
        ptask.stop()
        Reload
    }

    onKillCounter(kills, total) {
        rdebug("#KILLED", "Killed: <b>{}</b>/{}", kills, total)
        GuiControl,, % this.Kills, Killed: %kills%/%total%
    }

    lifeChanged(life, lParam) {
        maximum := lParam & 0xffff
        reserved := lParam >> 16
        lifePercent := Round(life * 100 / maximum)
        rdebug("#LIFE", "<b style=""color:red"">Life: {}/{}</b>", life, maximum - reserved)
        GuiControl,, % this.Life, Life: %lifePercent%`%
    }

    manaChanged(mana, lParam) {
        maximum := lParam & 0xffff
        reserved := lParam >> 16
        rdebug("#MANA", "<b style=""color:blue"">Mana: {}/{}</b>", Mana, maximum - reserved)
        manaPercent := Round(mana * 100 / maximum)
        if (reserved > 0) {
            reservedPercent := 100 - manaPercent
            GuiControl,, % this.Mana, Mana: %manaPercent%`%`/%reservedPercent%`%
        }
        else {
            GuiControl,, % this.Mana, Mana: %manaPercent%`%`
        }
    }

    energyShieldChanged(ES, maximum) {
        esPercent := Round(ES * 100 / maximum)
        rdebug("#ES", "<b style=""color:dodgerblue"">Energy Shield: {}/{}</b>", ES, maximum)
        GuiControl,, % this.EnergyShield, Energy Shield: %esPercent%`%
    }
}
