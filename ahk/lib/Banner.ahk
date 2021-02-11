;
; Banner.ahk, 9/14/2020 11:43 AM
;

#Include, %A_ScriptDir%\extras\debug.ahk
#Include, %A_ScriptDir%\extras\vendoring.ahk

class AhkGui {

    show(visible = true, noActivate = false) {
        DllCall("ShowWindow", "Int", this.Hwnd, "Int", visible + (noActivate << 2))
    }

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

class KillStats {

    cols := [ "Time", "Area Name", "Level", "Kills", "Gained Exp(%)"
            , "Normal", "Magic", "Rare", "Unique", "Used Time" ]

    __new() {
        global

        styleSheet =
        (
        <style>
            * { font-family: Fontin SmallCaps, Courier New; font-size: 18px; }
            body { background: #1A1411; margin: 0;}
            div { position: absolute; left: 0px; top: 0px; right: 0px; bottom: 0px; }
            table { text-align: right; margin: 0 auto; }
            tr { background: #120E0A; color: #875516; padding: 5px; transition: .1s ease-in; } 
            th { text-align: left; padding: .2rem; }
            td { padding: .5rem; }
            tr:first-child { background: #1A1411; color: #D9AA6F; font-size: 1rem; }
            tr:hover:not(#firstrow) { background: #D9AA6F; color: #120E0A; }
        </style>
        )

        Gui, __stats:New, +ToolWindow -Caption HwndHwnd, Kill statistics
        Gui, Margin, 0, 0
        Gui, Color, 1a1411
        Gui, Add, ActiveX, Border r15 w900 v__mshtml, Shell.Explorer
        Gui, Show, Hide

        this.hwnd := Hwnd
        __mshtml.Navigate("about:<meta http-equiv=""X-UA-Compatible"" content=""IE=edge""/>")
        this.doc := __mshtml.Document
        this.doc.write(styleSheet)
        this.doc.write("<div id=""#KillStats""></div>")
        this.statsTable := this.doc.getElementById("#KillStats")
        ComObjConnect(this.statsTable, this)
    }

    show(x, y) {
        innerHtml := "<div id=""#KillStats""><table>"
        innerHtml .= "<tr id=""firstrow"">"
        for i, col in this.cols
            innerHtml .=  "<th>" col "</th>"
        innerHtml .= "</tr>"

        p := ptask.getPlugin("KillCounter")
        for i, stat in p.getStats() {
            innerHtml .= "<tr>"
            innerHtml .= "<td>"  stat.timestamp "</td>"
            innerHtml .= "<td style=""text-align:left"">"  stat.areaName "</td>"
            innerHtml .= "<td>"  stat.areaLevel "</td>"
            innerHtml .= "<td>"  stat.totalKills "/" stat.totalMonsters "</td>"
            innerHtml .= "<td>"  Format("{}({:.2f})", stat.gainedExp, stat.gainedExp * 100 / levelExp[ptask.player.level]) "</td>"
            innerHtml .= "<td style=""color:#FFFAFA"">"  stat.normalKills "</td>"
            innerHtml .= "<td style=""color:#8787FE"">"  stat.magicKills "</td>"
            innerHtml .= "<td style=""color:#FEFE76"">"  stat.rareKills "</td>"
            innerHtml .= "<td style=""color:#AF5F1C"">"  stat.uniqueKills "</td>"
            innerHtml .= "<td>"  Format("{}m {:02d}s", stat.usedTime // 60, Mod(stat.usedTime, 60)) "</td>"
            innerHtml .= "</tr>"
        }
        this.statsTable.innerHtml := innerHtml "</table></div>"

        Gui, __stats:Show, % "x" x "y" y + 10
        WinSet, Transparent, 250, % "ahk_id " this.hwnd
    }

    onmouseleave() {
        Gui, __stats:Hide
        ptask.activate()
    }
}

class Banner extends AhkGui {

    __new(ownerHwnd) {
        global

        WinGetPos, x, y, w, h, ahk_id %ownerHwnd%
        y := (y < 0) ? -5 : 0
        Gui, __banner:New, -Caption +AlwaysOnTop +Toolwindow +HwndHwnd +LastFound
        Gui, Margin, 0, 0
        Gui, Color, White
        Gui, Font, cRed bold, Fontin SmallCaps
        Gui, Add, Text, % "y+5s w60 Hwnd" this.__var("Life"), % _("Life") ": 100`%"
        Gui, Font, cBlue bold
        Gui, Add, Text, % "ys w90 Hwnd" this.__var("Mana"), % _("Mana") ": 100`%"
        if (ShowEnergyShield) {
            Gui, Font, c1e90ff bold
            Gui, Add, Text, % "ys w150 Hwnd" this.__var("EnergyShield"), % _("Energy Shield") ": 100`%"
        }
        Gui, Font, cBlack bold
        Gui, Add, Text, % "ys w90 Hwnd" this.__var("kills") " gL1 v" this.__var("showKillStats"), % _("Kills") ": 0/0"
        Gui, Font, cRed bold
        Gui, Add, Text, % "ys x+10 w200 Hwnd" this.__var("Statusbar")

        Gui, Add, Button, % "x+5 y0 gL1 v" this.__var("hideout"), % _("Hideout")
        Gui, Add, Button, % "x+5 y0 gL1 v" this.__var("sellItems"), % _("Sell")
        Gui, Add, Button, % "x+1 y0 gL1 v" this.__var("stashItems"), % _("Stash")
        Gui, Add, Button, % "x+1 y0 gReload", % _("Reload")

        ; Extras buttons
        Gui, Add, Text, x+10
        addVendorButton()
        addDebugButton()

        this.Hwnd := hwnd
        OnMessage(WM_PLAYER_LIFE, ObjBindMethod(this, "lifeChanged"))
        OnMessage(WM_PLAYER_MANA, ObjBindMethod(this, "manaChanged"))
        OnMessage(WM_PLAYER_ES, ObjBindMethod(this, "energyShieldChanged"))
        OnMessage(WM_KILL_COUNTER, ObjBindMethod(this, "onKillCounter"))
    }

    destroy() {
        Gui, __banner:Destroy
    }

    show(flag = true) {
        if (Not flag) {
            Gui, __banner:Show, Hide
            return
        }

        r := ptask.getRect()
        Gui, __banner:Show, % "x"r.l + 150 " y" r.t + 6 "  NoActivate"
    }

    setStatusText(text = "", duration = 15000) {
        if (Not text) {
            GuiControl,, % this.Statusbar
            return
        }

        GuiControl,, % this.Statusbar, %text%
        fn := ObjBindMethod(this, "setStatusText")
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

    onKillCounter(kills, total) {
        GuiControl,, % this.Kills, Kills: %kills%/%total%
    }

    showKillStats() {
        if (Not this.stats)
            this.stats := new KillStats()
        WinGetPos, x, y, w, h, % "ahk_id " this.hwnd
        this.stats.show(x, y + h)
    }

    lifeChanged(life, lParam) {
        maximum := lParam & 0xffff
        reserved := lParam >> 16
        lifePercent := Round(life * 100 / maximum)
        rdebug("#LIFE", "<b style=""color:red"">" _("Life") ": {}/{}</b>", life, maximum - reserved)
        GuiControl,, % this.Life, % _("Life") ": " lifePercent "%"
    }

    manaChanged(mana, lParam) {
        maximum := lParam & 0xffff
        reserved := lParam >> 16
        rdebug("#MANA", "<b style=""color:blue"">" _("Mana") ": {}/{}</b>", Mana, maximum - reserved)
        manaPercent := Round(mana * 100 / maximum)
        if (reserved > 0) {
            reservedPercent := 100 - manaPercent
            GuiControl,, % this.Mana, % _("Mana") ": " manaPercent "%/" reservedPercent "%"
        }
        else {
            GuiControl,, % this.Mana, % _("Mana") ": " manaPercent "%"
        }
    }

    energyShieldChanged(ES, maximum) {
        esPercent := Round(ES * 100 / maximum)
        rdebug("#ES", "<b style=""color:dodgerblue"">" _("Energy Shield") ": {}/{}</b>", ES, maximum)
        GuiControl,, % this.EnergyShield, % _("Energy Shield") ": " esPercent "%"
    }
}
