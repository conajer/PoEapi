;
; Trader.ahk, 10/24/2020 7:49 PM
;

global trader := new Trader()

class TradeItem {

    static __alias := { "alchemy"      : "Orb of Alchemy"
                      , "alch"         : "Orb of Alchemy"
                      , "alteration"   : "Orb of Alteration"
                      , "augmentation" : "Orb of Augmentation"
                      , "blessed"      : "Blessed Orb"
                      , "chance"       : "Orb of Chance"
                      , "chaos"        : "Chaos Orb"
                      , "chisel"       : "Cartographer's Chisel"
                      , "chromatic"    : "Chromatic Orb"
                      , "chrome"       : "Chromatic Orb"
                      , "chrom"        : "Chromatic Orb"
                      , "coin"         : "Perandus Coin"
                      , "divine"       : "Divine Orb"
                      , "ex"           : "Exalted Orb"
                      , "exa"          : "Exalted Orb"
                      , "exalted"      : "Exalted Orb"
                      , "fusing"       : "Orb of Fusing"
                      , "gcp"          : "Gemcutter's Prism"
                      , "jeweller"     : "Jeweller's Orb"
                      , "portal"       : "Portal Scroll"
                      , "regal"        : "Regal Orb"
                      , "regret"       : "Orb of Regret"
                      , "scouring"     : "Orb of Scouring"
                      , "scroll"       : "Scroll of Wisdom"
                      , "silver"       : "Silver Coin"
                      , "trans"        : "Orb of Transmutation"
                      , "vaal"         : "Vaal Orb" }

    __new(fullName) {
        if (Not fullName) {
            this.fullName := "???"
        } else {
            this.fullName := fullName
            this.count := 1
        }

        if (RegExMatch(fullName, "level ([0-9]+) ([0-9]+)% (.+)", matched)) {
            this.isGem := true
            this.level := matched1
            this.quality := matched2
            this.name := matched3
        } else if (RegExMatch(fullName, "(.+) \(T([0-9]+)\)", matched)) {
            this.isMap := true
            this.name := matched1
            this.tier := matched2
        } else if (RegExMatch(fullName, "([0-9.,]*)([ ]*)([^-]+)", matched)) {
            this.name := this.__alias[matched3] ? this.__alias[matched3] : matched3
            if (matched1) {
                RegExMatch(matched1, "([0-9])(,([0-9]+))*", matched)
                this.count := matched1 matched3
            }
        }
    }
}

class TradeSession extends AhkGui {

    __new(player, item1, item2, position = "") {
        this.player := player
        this.item1 := new TradeItem(item1)
        this.item2 := new TradeItem(item2)

        if (RegExMatch(position, "stash tab (.*); position: left ([0-9]+), top ([0-9]+)", matched)) {
            this.left := matched2
            this.top := matched3

            RegExMatch(matched1, "\\""(.*)\\""|""(.*)""", matched)
            this.tabName := matched1 ? matched1 : matched2
        }

        this.__guiId := Format("__ts_{:x}", &this)
        tmpId := this.__guiId
        Gui, %tmpId%:New, -Caption +Toolwindow +AlwaysOnTop +Hwndhwnd +LastFound
        Gui, Color, c111318
        Gui, Margin, 5, 2
        WinSet, Transparent, 225

        this.hwnd := hwnd
        this.messages := []
        this.__updateTimer()
    }

    show() {
        tmpId := this.__guiId
        Gui, %tmpId%:Show, NoActivate
    }

    close() {
        if (Not this.isClosed) {
            this.isClosed := true
            tmpId := this.__guiId
            Gui, %tmpId%:Destroy
            adebug("#" this.__guiId, "    {} (<b>{}</b>)", _("Trade seesion closed"), this.player)
        }
    }

    checkItem() {
        return true
    }

    trade() {
        ptask.sendKeys("/tradewith " this.player)
    }

    1sec() {
        this.whisper(TraderMessages["1sec"])
        this.priority += 1
    }

    ask() {
        this.whisper(Format(TraderMessages["ask"], this.item2.fullName, this.item1.fullName))
    }

    hideout() {
        ptask.sendKeys("/hideout " this.player)
        Sleep, 3000
        this.needReturn := true
    }

    invite() {
        ptask.activate()
        if (Not this.isInvited && Not this.isJoined) {
            ptask.sendKeys("/invite " this.player)
            this.isInvited := true
            adebug("#" this.__guiId, "    {} {}...", _("Inviting"), this.player)
        }
    }

    kick() {
        if (this.isJoined) {
            poe.activate()
            if (ptask.getPartyStatus() == 0) {
                ; As the party's leader
                ptask.sendKeys("/kick " this.player)
            }
        }
    }

    leave() {
        if (ptask.getPartyStatus() < 3) {
            poe.activate()
            ptask.sendKeys("/kick " ptask.player.name)
            if (this.needReturn)
                ptask.sendKeys("/hideout")
        }
    }

    resend() {
        this.whisper(this.message)
    }

    sold() {
        this.whisper(TraderMessages["sold"])
        this.close()
    }

    thanks() {
        this.whisper(TraderMessages["thanks"])
    }

    whisper(message = "") {
        ptask.sendKeys("@" this.player " " message, Not message)
    }

    whois() {
        ptask.sendKeys("/whois " this.player)
    }

    __updateTimer() {
        if (this.isClosed) {
            SetTimer,, Delete
            return
        }

        if (this.elapsedTime == "") {
            this.elapsedTime := 0
            timer := ObjBindMethod(this, "__updateTimer")
            SetTimer, %timer%, 1000
            return
        }

        this.elapsedTime += 1
        if (this.elapsedTime < 120)
            elapsedTimeStr := this.elapsedTime "s"
        else if (this.elapsedTime < 3600)
            elapsedTimeStr := (this.elapsedTime // 60) "m"
        else
            elapsedTimeStr := (this.elapsedTime // 3600) "h"

        GuiControl,, % this.elapsedTimeHwnd, %elapsedTimeStr%
    }
}

class Trader {

    tsActive := {}      ; Active trade sessions
    tsActiveList := []  ; Ordered active trade session list
    tsQueue := []       ; Trade session queue
    tsHistory := []     ; Trade history
    tsCount := 0        ; Total trade session count

    __new() {
        OnMessage(0x02, ObjBindMethod(this, "onClose"))
        OnMessage(WM_NEW_MESSAGE, ObjBindMethod(this, "onMessage"))

        if (EnableAutoTrade)
            ptask.sendKeys("/afkoff")
    }

    beginTrade(aTrade) {
        if (EnabledAutoTrade && ptask.InHideout) {
            this.tsCount += 1
            debug(Format("{:03d} {} <b>{}</b>", this.tsCount, _("Trade with"), aTrade.player))

            timer := ObjBindMethod(aTrade, "trade")
            SetTimer, %timer%, -500
        }
    }

    queue(aTrade) {
        ; Ignore duplicated trade request.
        if (this.tsActive[aTrade.player])
            return

        for i, ts in this.tsQueue {
            if (aTrade.player == ts.player)
                return
        }

        this.tsQueue.Push(aTrade)
        rdebug("#" aTrade.__guiId, "Queued trade session with <b style=""color:blue"">{}</b> ({}/{})"
               , aTrade.player, aTrade.item1.fullName, aTrade.item2.fullName)
        this.nextTrade()
    }

    nextTrade() {
        Critical

        if (this.tsActive.Count() == TraderMaxSessions) {
            for player, ts in this.tsActive {
                if (ts.timestamp + TraderTimeout * 1000 < A_Tickcount)
                    ts.close()
            }
        }

        if (this.tsActive.Count() == TraderMaxSessions)
            return

        if (this.tsQueue.Length() > 0) {
            loop, % this.tsQueue.Length() {
                aTrade := this.tsQueue.RemoveAt(1)
                if (aTrade.timestamp + TraderTimeout * 1000 > A_Tickcount)
                    break

                ; Trade request expires after 60s.
                debug("Trade session expired (<b>{}</b>).", aTrade.player)
                aTrade.close()
            }

            this.tsActive[aTrade.player] := aTrade
            this.tsActiveList.Push(aTrade)
            this.beginTrade(aTrade)
        }
        this.showAll()
    }

    showAll() {
        y :=38
        for i, ts in this.tsActiveList {
            ts.show()
            WinGetPos,,, w, h, % "ahk_id " ts.hwnd
            WinMove, % "ahk_id " ts.hwnd,, ptask.x + (ptask.width - w) / 2, y
            y += h + 2
        }
    }

    onClose(wParam, lParam, msg, hwnd) {
        for i, ts in this.tsActiveList {
            if (ts.isClosed) {
                this.tsActive.Delete(ts.player)
                tt := this.tsActiveList.RemoveAt(i)
                this.showAll()
                break
            }
        }
    }

    onMessage(message, lParam) {
        message := StrGet(message)

        if (RegExMatch(message, "^(@[^ ]*) (\<.*\> |)([^ :]+): (.*)", matched)) {
            domain := matched1
            guild := matched2
            player := matched3
            message := matched4

            if (RegExMatch(message, "Hi, I'd like to buy your (.*)( for my (.*)) in ([a-zA-Z]*)", matched)
                || RegExMatch(message, "U)Hi, I would like to buy your (.*) (listed for (.*) )?in (.*) \((.*)\)(.*)", matched)) {
                if (ptask.league != matched4)
                    return

                if (domain == "@From") {
                    aTrade := new IncomingTradeSession(player, matched3, matched1, matched5, matched6)
                    aTrade.priority := 0
                } else {
                    if (player == ptask.player.name)
                        return

                    aTrade := new OutgoingTradeSession(player, matched1, matched3, matched6)
                    aTrade.priority := 1
                }

                aTrade.message := message
                aTrade.timestamp := A_Tickcount
                this.queue(aTrade)
            } else {
                this.tsActive[player].messages.Push(message)
            }
        } else if (RegExMatch(message, "This player has DND mode enabled.")) {
            if (this.isTrading) {
                this.tsCurrent.close()
                debug("  ! " message)
            }
        } else if (RegExMatch(message, _("You have left the party."), matched)) {
            if (this.tsCurrent.player == matched1)
                this.tsCurrent.isJoined := false
        } else if (RegExMatch(message, "(.*) has joined your party.", matched)) {
            this.tsActive[matched1].isJoined := true
        } else if (RegExMatch(message, _("(.*) has left the party."), matched)) {
            if (this.tsActive.HasKey(matched1)) {
                this.tsActive[matched1].isJoined := false
                this.tsActive[matched1].close()
            }
        } else if (RegExMatch(message, _("(.*) has joined the area."), matched)) {
            if (EnableAutoTrade && Not this.isTrading) {
                if (this.tsActive.HasKey(matched1)) {
                    this.isTrading := true
                    this.tsCurrent := this.tsActive[matched1]
                }
            }
        } else if (RegExMatch(message, _("Trade accepted."))) {
            if (this.isTrading) {
                this.tsHistory.Push(this.tsCurrent)
                Sleep, 500
                this.tsCurrent.thanks()
                this.tsCurrent.close()
                debug("    " _("Trade accepted"))
            }
        } else if (RegExMatch(message, _("Trade cancelled."))) {
            if (this.isTrading) {
                this.tsCurrent.isCancelled := true
                this.tsCurrent.isTrading := false
                debug("    " _("Trade cancelled"))
            }
        } else if (RegExMatch(message, "is not online.")
                   || RegExMatch(message, "You have been ignored by the target player.")
                   || RegExMatch(message, "That character is out of your league.")) {
            if (this.isTrading) {
                this.tsCurrent.close()
                debug("  ! " message)
            }
        } else if (RegExMatch(message, "You cannot trade with an item on the cursor.")) {
            error("You cannot trade with an item on the cursor")

            ; Drop off the item to inventory
            if (Not cursor.dropOff()) {
                inventory.stashItems(1, -1)
                return
            }

            ; Trade again
            this.activeTrade.__exec("/tradewith " this.activeTrade.player)
        } else if (RegExMatch(message, _("(.*) sent you a party invite"), matched)) {
            if (EnabledAutoTrade && Not this.isTrading) {
                if (this.tsActive.HasKey(matched1)) {
                    poe.activate()
                    dialog := Object(lParam)
                    acceptButton := dialog.getChild(3, 1)
                    loop, 5 {
                        if (Not dialog.isVisible())
                            break

                        Sleep, 500
                        acceptButton.getPos(x, y)
                        MouseClick, Left, x, y
                    }
                    this.tsCurrent := this.tsActive[matched1]
                    this.tsCurrent.isJoined := true
                    this.isTrading := true
                    debug("    Accepted party invite from <b style=""color:blue"">{}</b>", matched1)
                }
            } else {
                debug("*** Received a party invite from <b style=""color:blue"">{}</b>", matched1)
            }
        } else if (RegExMatch(message, _("(.*) sent you a trade request"), matched)) {
            if (EnableAutoTrade && Not this.isTrading) {
                poe.activate()
                dialog := Object(lParam)
                acceptButton := dialog.getChild(3, 1)
                loop, 5 {
                    if (Not dialog.isVisible())
                        break

                    Sleep, 500
                    acceptButton.getPos(x, y)
                    MouseClick, Left, x, y
                }
            }
        } else if (RegExMatch(message, _("You cannot currently access this player's area."))) {
            if (this.isTrading) {
                Sleep, 500
                this.tsCurrent.hideout()
            }
        }
    }
}

class IncomingTradeSession extends TradeSession {
    
    __new(player, item1, item2, position = "", comment = "") {
        global

        base.__new(player, item1, item2, position)
        if (TraderUICompact) {
            Gui, Font, cFEFE76 bold s10, Fontin SmallCaps
            Gui, Add, Text, % "y6 gL1 v" this.__var("whois"), % this.player
            Gui, Font, cFFFAFA bold s10
            Gui, Add, Text, ys x+10, %  "=> " this.item1.fullName 
            Gui, Font, c8787FE bold s8
            Gui, Add, Text, % "ys+3 x325 w25 Hwnd" this.__var("elapsedTimeHwnd"), 0s
            Gui, Font, bold s8
            Gui, Add, Button, % "x350 y5 w65 gL1 v" this.__var("trade"), % _("Trade")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("invite"), % _("Invite")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("kick"), % _("Kick")
            Gui, Add, Button, % "x+2 y5 gL1 v" this.__var("close"), X

            Gui, Font, cD20000 bold s10
            Gui, Add, Text, % "xs y30 w350 gL1 v" this.__var("checkItem"), % this.item2.fullName
            Gui, Font, bold s8
            Gui, Add, Button, % "x350 y30 w65 gL1 v" this.__var("1sec"), % _("1 sec")
            Gui, Add, Button, % "x+2 y30 w65 gL1 v" this.__var("thanks"), % _("Thanks")
            Gui, Add, Button, % "x+2 y30 w65 gL1 v" this.__var("sold"), % _("Sold")
            Gui, Add, Button, % "x+2 y30 gL1 v" this.__var("ask"), ?
        } else {
            Gui, Font, cFEFE76 bold s10, Fontin SmallCaps
            Gui, Add, Text, % "y6 w100 gL1 v" this.__var("whois"), % ellipsis(this.player, 12)
            Gui, Font, cFFFAFA bold s10
            Gui, Add, Text, ys, % this.item1.fullName " => "
            Gui, Font, cD20000 bold s10
            Gui, Add, Text, % "x+0 w300 gL1 v" this.__var("checkItem"), % this.item2.fullName

            Gui, Font, c8787FE bold s8
            Gui, Add, Text, % "ys+3 x500 w25 Hwnd" this.__var("elapsedTimeHwnd"), 0s
            Gui, Add, Button, % "x+2 y5 w62 gL1 v" this.__var("trade"), % _("Trade")
            Gui, Add, Button, % "x+2 y5 w62 gL1 v" this.__var("invite"), % _("Invite")
            Gui, Add, Button, % "x+2 y5 w61 gL1 v" this.__var("kick"), % _("Kick")
            Gui, Add, Button, % "x+2 y5 w61 gL1 v" this.__var("1sec"), % _("1 sec")
            Gui, Add, Button, % "x+2 y5 w61 gL1 v" this.__var("thanks"), % _("Thanks")
            Gui, Add, Button, % "x+2 y5 w61 gL1 v" this.__var("sold"), % _("Sold")
            Gui, Add, Button, % "x+2 y5 w17 gL1 v" this.__var("ask"), ?
            Gui, Add, Button, % "x+2 y5 gL1 v" this.__var("close"), X
        }

        if (comment) {
            Gui, Font, cA2C0D5 italic s10
            Gui, Add, Text, % "xs", % Trim(comment, " -*`t")
        }
    }

    close() {
        this.kick()
        base.close()
    }

    checkItem() {
        ptask.activate()
        if (Not ptask.stash.open())
            return

        if (this.tabName) {
            ptask.stash.switchTab(this.tabName)
            Sleep, 300
            tab := ptask.stash.Tab
            item := tab.getItem(this.left, this.top)
            if (Not item)
                return false
            tab.moveTo(item.index)
        }
    }
}

class OutgoingTradeSession extends TradeSession {
    
    __new(player, item1, item2, comment = "") {
        global

        base.__new(player, item1, item2)
        if (TraderUICompact) {
            Gui, Font, cFEFE76 bold s10, Fontin SmallCaps
            Gui, Add, Text, % "y6 gL1 v" this.__var("whois"), % this.player
            Gui, Font, cFFFAFA bold s10
            Gui, Add, Text, % "ys x+10 gL1 v"  this.__var("checkItem"), %  "<= " this.item2.fullName
            Gui, Font, c8787FE bold s8
            Gui, Add, Text, % "ys+3 x325 w25 Hwnd" this.__var("elapsedTimeHwnd"), 0s
            Gui, Add, Button, % "x350 y5 w65 gL1 v" this.__var("trade"), % _("Trade")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("hideout"), % _("Hideout")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("leave"), % _("Leave")
            Gui, Add, Button, % "x+2 y5 gL1 v" this.__var("close"), X

            Gui, Font, c47E635 bold s10
            Gui, Add, Text, % "xs y30 w350", % this.item1.fullName
            Gui, Font, bold s8
            Gui, Add, Button, % "x350 y30 w65 c020f29 gL1 v" this.__var("whisper"), % _("Whisper")
            Gui, Add, Button, % "x+2 y30 w65 gL1 v" this.__var("resend"), % _("Resend")
            Gui, Add, Button, % "x+2 y30 w65 gL1 v" this.__var("thanks"), % _("Thanks")
        } else {
            Gui, Font, cFEFE76 bold s10, Fontin SmallCaps
            Gui, Add, Text, % "y6 w100 gL1 v" this.__var("whois"), % ellipsis(this.player, 13)
            Gui, Font, cFFFAFA bold s10
            Gui, Add, Text, % "ys gL1 v"  this.__var("checkItem"), % this.item2.fullName " => " 
            Gui, Font, c47E635 bold s10
            Gui, Add, Text, x+0 w300, % this.item1.fullName

            Gui, Font, c8787FE bold s8
            Gui, Add, Text, % "ys+3 x500 w25 Hwnd" this.__var("elapsedTimeHwnd"), 0s
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("trade"), % _("Trade")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("hideout"), % _("Hideout")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("leave"), % _("Leave")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("whisper"), % _("Whisper")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("resend"), % _("Resend")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("thanks"), % _("Thanks")
            Gui, Add, Button, % "x+2 y5 gL1 v" this.__var("close"), X
        }

        if (comment) {
            Gui, Font, cA2C0D5 italic s10
            Gui, Add, Text, % "xs", % Trim(comment, " -*`t")
        }
    }

    close() {
        this.leave()
        base.close()
    }
}

ellipsis(text, width) {
    n := StrLen(text)
    if (n > width) {
        text := SubStr(text, 1, width - 1)
        text .= "..."
    }

    return text
}
