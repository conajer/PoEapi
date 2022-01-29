;
; Trader.ahk, 10/24/2020 7:49 PM
;

global trader := new Trader()

global alias := { "alchemy"      : "Orb of Alchemy"
                , "alch"         : "Orb of Alchemy"
                , "alteration"   : "Orb of Alteration"
                , "augmentation" : "Orb of Augmentation"
                , "blessed"      : "Blessed Orb"
                , "chance"       : "Orb of Chance"
                , "chaos"        : "Chaos Orb"
                , "chisel"       : "Cartographer's Chisel"
                , "chromatic"    : "Chromatic Orb"
                , "chrome"       : "Chromatic Orb"
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

class TradeItem {

    __new(detailInfo) {
        if (Not detailInfo) {
            this.detailInfo := "???"
        } else {
            this.detailInfo := detailInfo
            this.count := 1
        }

        if (RegExMatch(detailInfo, "level ([0-9]+) ([0-9]+)% (.+)", matched)) {
            this.isGem := true
            this.level := matched1
            this.quality := matched2
            this.name := matched3
        } else if (RegExMatch(detailInfo, "(.+) \(T([0-9]+)\)", matched)) {
            this.isMap := true
            this.name := matched1
            this.tier := matched2
        } else if (RegExMatch(detailInfo, "([0-9.,]*)\s?([^-]+)", matched)) {
            this.name := alias[matched2] ? alias[matched2] : matched2
            if (matched1) {
                RegExMatch(matched1, "([0-9.]+)(,([0-9.]+))*", matched)
                this.count := matched1 matched3
            }
        }
    }
}

class TradeSession extends AhkGui {

    __new(player, item1, item2) {
        this.player := player
        this.item1 := new TradeItem(item1)
        this.item2 := new TradeItem(item2)

        this.__guiId := Format("__ts_{:x}", &this)
        tmpId := this.__guiId
        Gui, %tmpId%:New, -Caption +Toolwindow +AlwaysOnTop +Hwndhwnd +LastFound
        Gui, Color, c111318
        Gui, Margin, 5, 2
        WinSet, Transparent, 225

        this.hwnd := hwnd
        this.messages := []
        this.__getCurrencyTabName()
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

    trade() {
        ptask.sendKeys("/tradewith " this.player)
    }

    1sec() {
        this.whisper(TraderMessages["1sec"])
        this.priority += 1
    }

    ask() {
        this.whisper(Format(TraderMessages["ask"], this.item2.detailInfo, this.item1.detailInfo))
    }

    hideout() {
        ptask.sendKeys("/hideout " this.player)
        Sleep, 3000
        this.needReturn := true
    }

    invite() {
        ptask.activate()
        if (Not this.isJoined) {
            ptask.sendKeys("/invite " this.player)
            this.isInvited := true
            adebug("#" this.__guiId, "    {} {}...", _("Inviting"), this.player)
        }
    }

    kick() {
        if (this.isInvited || this.isJoined) {
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
            Sleep, 30
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

    __getCurrencyTabName() {
        if (Not this.currencyTabName) {
            if (Not CurrencyTabName)
                for i, tab in ptask.getStashTabs() {
                    if (tab.type == 3) {
                        this.currencyTabName := tab.name
                        break
                    }
                }
            else
                this.currencyTabName := CurrencyTabName
        }

        return this.currencyTabName
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

class TradeBroker {

    whisperFormat := "O)(?P<type>From|To) (?P<guild>\<.+\> |)(?P<player>[^ :]+)"
    tradeRequestFormat := "O)Hi, (I would|I'd) like to buy your (?P<item1>.+) (listed for|for my) (?P<item2>.+) in (?P<league>[^ ]+)\s?(\(stash tab (?P<tabName>.+); position: left (?P<left>[0-9]+), top (?P<top>[0-9]+)\))?(?P<postscript>.*)"
    whisperFrom := "From"

    translate(whisper, message, ByRef player, ByRef aTrade) {
        if (Not RegExMatch(whisper, this.whisperFormat, matched))
            return false

        whisperType := matched["type"]
        player := matched["player"]
        if (RegExMatch(message, this.tradeRequestFormat, matched)) {
            if (matched["League"] != _(ptask.league))
                return

            if (whisperType == this.whisperFrom) {
                aTrade := new IncomingTradeSession(player, matched["item2"], matched["item1"], matched["postscript"])
                if (matched["tabName"]) {
                    aTrade.left := matched["left"]
                    aTrade.top := matched["top"]
                    aTrade.tabName := StrReplace(matched["tabName"], """")
                }
            } else {
                if (player == ptask.player.name)
                    return
                aTrade := new OutgoingTradeSession(player, matched["item1"], matched["item2"], matched["postscript"])
            }

            return true
        }
    }
}

class TradeBroker_ru extends TradeBroker {

    whisperFormat := "O)(?P<type>От кого|Кому) (?P<guild>\<.+\> |)(?P<player>[^ :]+)"
    tradeRequestFormat := "O)Здравствуйте, хочу купить у вас (?P<item1>.+) за (?P<item2>.+) в лиге (?P<league>[^ ]+)\s?(\(секция (?P<tabName>.+); позиция: (?P<left>[0-9]+) столбец, (?P<top>[0-9]+) ряд\))?(?P<postscript>.*)"
    whisperFrom := "От кого"
}

class Trader {

    tsActive := {}      ; active trade sessions
    tsActiveList := []  ; ordered active trade session list
    tsQueue := []       ; trade session queue
    tsHistory := []     ; trade history
    tsCount := 0        ; total trade session count
    tradeBrokers := []  ; registered trade brokers

    __new() {
        OnMessage(0x02, ObjBindMethod(this, "onClose"))
        OnMessage(WM_NEW_MESSAGE, ObjBindMethod(this, "onMessage"))
        this.addBroker(new TradeBroker())
        this.addBroker(new TradeBroker_ru())
    }

    addBroker(broker) {
        this.tradeBrokers.Push(broker)
    }

    beginTrade(aTrade) {
        if (EnabledAutoTrade && ptask.InHideout) {
            this.tsCount += 1
            rdebug("#" aTrade.__guiId, Format("{:03d} {} <b>{}</b>", this.tsCount, _("Trade with"), aTrade.player))

            timer := ObjBindMethod(aTrade, "trade")
            SetTimer, %timer%, -500
        }
    }

    queue(aTrade) {
        ; ignore duplicated trade request.
        if (this.tsActive[aTrade.player])
            return

        for i, ts in this.tsQueue {
            if (aTrade.player == ts.player)
                return
        }

        this.tsQueue.Push(aTrade)
        rdebug("#" aTrade.__guiId, "Queued trade session with <b style=""color:blue"">{}</b> (<b>{}</b>/<b>{}</b>)"
               , aTrade.player, aTrade.item1.detailInfo, aTrade.item2.detailInfo)
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

                ; trade request expires after 60s.
                rdebug("#" aTrade.__guiId, "Trade session expired (<b>{}</b>).", aTrade.player)
                aTrade.close()
            }

            this.tsActive[aTrade.player] := aTrade
            this.tsActiveList.Push(aTrade)
            this.beginTrade(aTrade)
        }
        this.showAll()
    }

    showAll() {
        Critical
        y :=70
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
                this.tsActiveList.RemoveAt(i)
                this.showAll()
                break
            }
        }
    }

    onMessage(wParam, lParam) {
        message := StrGet(wParam)
        if (RegExMatch(message, "^@([^:]+): (.*)", matched)) {
            for i, broker in this.tradeBrokers {
                if (broker.translate(matched1, matched2, player, aTrade)) {
                    if (aTrade) {
                        aTrade.messages.Push(message)
                        aTrade.timestamp := A_Tickcount
                        this.queue(aTrade)
                    }
                    break
                }
            }

            if (Not aTrade && this.tsActive[player])
                this.tsActive[player].messages.Push(message)
        } else if (RegExMatch(message, _("You have left the party."), matched)) {
            for player, ts in this.tsActive {
                if (ts.isJoined) {
                    ts.isJoined := false
                    ts.close()
                    break
                }
            }
        } else if (RegExMatch(message, _("(.*) has joined your party."), matched)) {
            this.tsActive[matched1].isJoined := true
        } else if (RegExMatch(message, _("(.*) has left the party."), matched)) {
            this.tsActive[matched1].isJoined := false
            this.tsActive[matched1].close()
        } else if (RegExMatch(message, _("(.*) sent you a party invite"), matched)) {
            debug("*** Received a party invite from <b style=""color:blue"">{}</b>", matched1)
        }
    }
}

class IncomingTradeSession extends TradeSession {

    __new(player, item1, item2, postscript = "") {
        global

        base.__new(player, item1, item2)
        this.priority := 0

        if (TraderUICompact) {
            Gui, Font, cFEFE76 bold s10, Fontin SmallCaps
            Gui, Add, Text, % "y6 gL1 v" this.__var("whois"), % this.player
            Gui, Font, cFFFAFA bold s10
            Gui, Add, Text, ys x+10, %  "=> " this.item1.detailInfo 
            Gui, Font, c8787FE bold s8
            Gui, Add, Text, % "ys+3 x325 w25 Hwnd" this.__var("elapsedTimeHwnd"), 0s
            Gui, Font, bold s8
            Gui, Add, Button, % "x350 y5 w65 gL1 v" this.__var("trade"), % _("Trade")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("invite"), % _("Invite")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("kick"), % _("Kick")
            Gui, Add, Button, % "x+2 y5 gL1 v" this.__var("close"), X

            Gui, Font, cD20000 bold s10
            Gui, Add, Text, % "xs y30 w350 gL1 v" this.__var("checkItem"), % this.item2.detailInfo
            Gui, Font, bold s8
            Gui, Add, Button, % "x350 y30 w65 gL1 v" this.__var("1sec"), % _("1 sec")
            Gui, Add, Button, % "x+2 y30 w65 gL1 v" this.__var("thanks"), % _("Thanks")
            Gui, Add, Button, % "x+2 y30 w65 gL1 v" this.__var("sold"), % _("Sold")
            Gui, Add, Button, % "x+2 y30 gL1 v" this.__var("ask"), ?
        } else {
            Gui, Font, cFEFE76 bold s10, Fontin SmallCaps
            Gui, Add, Text, % "y6 w100 gL1 v" this.__var("whois"), % ellipsis(this.player, 12)
            Gui, Font, cFFFAFA bold s10
            Gui, Add, Text, ys, % this.item1.detailInfo " => "
            Gui, Font, cD20000 bold s10
            Gui, Add, Text, % "x+0 w300 gL1 v" this.__var("checkItem"), % this.item2.detailInfo

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

        if (postscript) {
            Gui, Font, cA2C0D5 italic s10
            Gui, Add, Text, % "xs", % Trim(postscript, " -*`t")
        }
    }

    close() {
        this.kick()
        base.close()
    }

    checkItem() {
        ptask.activate()
        if (Not ptask.stash.open()) {
            adebug("#" this.__guiId, "   ! Can't open stash")
            return false
        }

        itemName := this.item2.name
        if (this.left && this.top) {
            adebug("#" this.__guiId, "    Checking <b>{}</b> (stash tab ""{}"": left {}, top {})"
                  , this.item2.name, this.tabName, this.left, this.left)
            if (Not tab := ptask.stash.switchTab(this.tabName)) {
                Sleep, 500
                if (Not tab := ptask.stash.switchTab(this.tabName)) {
                    adebug("#" this.__guiId, "    ! No such tab '{}'", this.tabName)
                    return
                }
            }

            SendInput, ^{f}
            Clipboard := itemName
            SendInput, ^{v}{Enter}

            aItem := tab.getItem(this.left, this.top)
            if (Not aItem || Not (this.item2.name ~= aItem.qualifiedName()))
                aItem := tab.findItem(this.item2.name)

            if (Not aItem) {
                adebug("#" this.__guiId, "  ! Item not found: <b>{}</b>", this.item2.name)
                return false
            }

            tab.moveTo(aItem.index)
            clipboard := ""
            loop, 5 {
                Sleep, 200
                SendInput ^{c}
            } until clipboard

            if (RegExMatch(clipboard, "~(b/o|price) ([0-9.]+) (.+)", matched)
                || RegExMatch(this.tabName, "~(b/o|price) ([0-9.]+) (.+)", matched)) {

                if (this.item1.count >= matched2 && this.item1.name == alias[matched3])
                    return true

                adebug("#" this.__guiId, "  ! Price not match: {} {} / {} {}!!!", this.item1.count, this.item1.name, matched2, alias[matched3])
                return false
            }
            adebug("#" this.__guiId, "  ! No price tag for <b>{}</b>", this.item2.name)
        } else {
            SendInput, ^{f}
            SendInput, %itemName%{Enter}

            tab := ptask.stash.switchTab(this.currencyTabName)
            if (Not tab.findItem(this.item2.name)) {
                this.tabName := Format("~b/o {:g} chaos", this.item1.count / this.item2.count)
                tab := ptask.stash.switchTab(this.tabName)
            }

            aItem := tab.findItem(this.item1.name)
            if (aItem.name ~= "Chaos|Exalted") {
                totalPrice := tab.checkItem(aItem.name) * $(aItem)
                if (totalPrice > 10000) {
                    adebug("#" this.__guiId, "    !!! Too many <b>{}</b> (eq. {:.0f} chaos) in stash", this.item1.name, totalPrice)
                    return false
                }
            }

            total := tab.checkItem(this.item2.name)
            total += ptask.inventory.checkItem(this.item2.name)
            if (total >= this.item2.count) {
                tab.dump(this.item2.name, this.item2.count)
                return true
            }

            adebug("#" this.__guiId, "  ! Out of stock: {} <b>{}</b> in stash", total, this.item2.name)
        }

        return false
    }
}

class OutgoingTradeSession extends TradeSession {

    __new(player, item1, item2, postscript = "") {
        global

        base.__new(player, item1, item2)
        this.priority := 1

        if (TraderUICompact) {
            Gui, Font, cFEFE76 bold s10, Fontin SmallCaps
            Gui, Add, Text, % "y6 gL1 v" this.__var("whois"), % this.player
            Gui, Font, cFFFAFA bold s10
            Gui, Add, Text, % "ys x+10 gL1 v"  this.__var("checkItem"), %  "<= " this.item2.detailInfo
            Gui, Font, c8787FE bold s8
            Gui, Add, Text, % "ys+3 x325 w25 Hwnd" this.__var("elapsedTimeHwnd"), 0s
            Gui, Add, Button, % "x350 y5 w65 gL1 v" this.__var("trade"), % _("Trade")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("hideout"), % _("Hideout")
            Gui, Add, Button, % "x+2 y5 w65 gL1 v" this.__var("leave"), % _("Leave")
            Gui, Add, Button, % "x+2 y5 gL1 v" this.__var("close"), X

            Gui, Font, c47E635 bold s10
            Gui, Add, Text, % "xs y30 w350", % this.item1.detailInfo
            Gui, Font, bold s8
            Gui, Add, Button, % "x350 y30 w65 c020f29 gL1 v" this.__var("whisper"), % _("Whisper")
            Gui, Add, Button, % "x+2 y30 w65 gL1 v" this.__var("resend"), % _("Resend")
            Gui, Add, Button, % "x+2 y30 w65 gL1 v" this.__var("thanks"), % _("Thanks")
        } else {
            Gui, Font, cFEFE76 bold s10, Fontin SmallCaps
            Gui, Add, Text, % "y6 w100 gL1 v" this.__var("whois"), % ellipsis(this.player, 13)
            Gui, Font, cFFFAFA bold s10
            Gui, Add, Text, % "ys gL1 v"  this.__var("checkItem"), % this.item2.detailInfo " => " 
            Gui, Font, c47E635 bold s10
            Gui, Add, Text, x+0 w300, % this.item1.detailInfo

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

        if (postscript) {
            Gui, Font, cA2C0D5 italic s10
            Gui, Add, Text, % "xs", % Trim(postscript, " -*`t")
        }
    }

    checkItem() {
        ptask.activate()
        if (Not ptask.stash.open())
            return false

        ptask.stash.switchTab(this.currencyTabName)
        Sleep, 100
        tab := ptask.stash.Tab
        item := tab.findItem(this.item2.name)
        if (item.stackCount() < this.item2.count) {
            adebug("#" this.__guiId, "   ! No enough <b>{}</b> ({} in stock)", this.item2.name, item.stackCount())
            return false
        }

        MouseGetPos, x, y
        n := tab.dump(this.item2.name, Floor(this.item2.count))
        if (n < this.item2.count) {
            m := $(this.item2.name) * (this.item2.count - n)
            if (m > 0)
                m := tab.dump(_("Chaos Orb"), Ceil(m))
        }
        MouseMove, x, y, 0
        adebug("#" this.__guiId, "    Withdrew <b>{} {}{}</b>", n, this.item2.name, (m > 0 ? " + " m " " _("Chaos Orb") : ""))

        return true
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
