;
; Navi.ahk, 3/7/2021 5:26 PM
;

global extraMenus := []

addMenuItem(menu, itemName = "", handler = "", options ="") {
    Menu, %menu%, Add, %itemName%, %handler%, %options%
}

addExtraMenu(name, handler, options = "") {
    extraMenus.Push({"name":name, "handler":handler, "options":options})
}

class About extends WebGui {

    __new() {
        base.__new("About PoEapikit",, 700, 400)
        this.document.write("
        (%
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                * { font-family: Georgia, Serif; line-height: 1.5; }
                html, body { display: flex; flex-flow: column; height: 100%; background: #f0f0f0; margin: 0; }
                div { flex: 2 1 auto; background-color: white; font-size: 18px; border: 1px solid; margin: 5px 5px; padding: 0px 15px; }
                span { flex: 0 1 auto; margin: 0px 5px 5px; }
                button { font-family: Calibri; background-color: #e1e1e1; border: 1px solid #adadad; margin: 5px 2px; float: right; transition: 0.4s; padding: 0 30px; }
                button:focus { outline: solid; outline-width: 1px; outline-color: #0078d7; }
                button:hover { background-color: #e5f1fb; outline: solid; outline-width: 1px; outline-color: #0078d7; }
                a { target-new: window; }

                .deps { color: dimgray; }
            </style>
        </head>
        <body>
            <div>
                <h1>PoEapikit</h1>
                <span>Version <strong id='version'></strong> 64-Bit</span><br>
                <span class=deps>PoEapi version <strong id='poeapi_version'></strong></span><br>
                <span class=deps>AutoHotkey version <strong id=ahk_version></strong></span><br>
                <span>Donate:
                    <a href='#paypal'>Paypal</a>
                    <a href='#btc'>BTC</a>
                    <a href='#eth'>ETH</a>
                </span><br>
                <span>Links:
                    <a href='#github'>GitHub</a>
                    <a href='#patreon'>Patreon</a>
                    <a href='#discord'>Discord</a>
                </span>
            </div>
            <span>
                <button id='ok'>OK</button>
            </span>
        </body>
        </html>
        )")
        this.document.close()
        this.bindAll("button")
        this.bindAll("a")

        this._("#version").innerText := version
        this._("#poeapi_version").innerText := poeapiVersion
        this._("#ahk_version").innerText := A_AhkVersion
    }

    ok() {
        this.close()
    }

    onclick(e) {
        href := e.srcElement.href
        if (href ~= "#github")
            url := "https://github.com/conajer/PoEapi"
        else if (href ~= "#patreon")
            url := "https://www.patreon.com/conajer"
        else if (href ~= "#discord")
            url := "https://discord.gg/2gYnu7Nk4q"
        else if (href ~= "#paypal")
            url := "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=TDU8JSQAXNTU6&currency_code=USD"
        else if (href ~= "#btc")
            address := "1NibcggH597Nu9pP8aJ1Zcn7J94L5R1rp9"
        else if (href ~= "#eth")
            address := "0x4493e75ef1d7B86f8cf2177abf45A9F16FED2a21"

        if (address) {
            Clipboard := address
            Msgbox, 0, Copied, % e.srcElement.innerText ": " address
        }
        Run, % url
    }
}

class Hotkeys extends WebGui {

    __new() {
        base.__new("Hotkeys",, 900, 600)
        this.document.write("
        (%
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                * { font-family: Georgia, Serif; line-height: 1.5; }
                html, body { display: flex; flex-flow: column; height: 100%; background: #f0f0f0; margin: 0; }
                div { flex: 2 1 auto; background-color: white; font-size: 18px; border: 1px solid; margin: 5px 5px; padding: 0px 15px; overflow: auto; }
                span { flex: 0 1 auto; margin: 0px 5px 5px; }
                button { font-family: Calibri; background-color: #e1e1e1; border: 1px solid #adadad; margin: 5px 2px; float: right; transition: 0.4s; padding: 0 30px; }
                button:focus { outline: solid; outline-width: 1px; outline-color: #0078d7; }
                button:hover { background-color: #e5f1fb; outline: solid; outline-width: 1px; outline-color: #0078d7; }
                table { font-size: 16px; padding: 10px; }
                tr:nth-child(even) { background-color: #f7f7f7; }
                td:nth-child(2) { color: blue; text-align: right; font-weight: bold; }
                td:nth-child(3) { color: dimgray; padding: 0 15px; }
                input[type=text] { border: 1px solid #adadad; font-size: 16px; font-weight: bold; color: blue; text-align: right; padding: 0 5px 5px; }
                input[type=text]::-ms-clear { display: none; }
                input[type=text]:focus { border-color: #0078d7; }
                .hint { font-size: 14px; color: dimgray; margin: 15px; }
            </style>
        </head>
        <body>
            <div>
                <h2>Hotkeys</h2>
                <i class='hint'>* ^ for CTRL, + for SHIFT, ! for ALT and # for Windows key</i><br/>
                <i class='hint'>** Hotkeys from extra features or patreon only features CAN NOT be changed.</i> 
                <table id='hotkeys'>
                </table>
            </div>
            <span>
                <button id='cancel'>Cancel</button>
                <button id='ok'>OK</button>
            </span>
        </body>
        </html>
        )")
        this.document.close()
        this.list()
        this.bindAll("button")

        this._("#version").innerText := version
        this._("#poeapi_version").innerText := poeapiVersion
    }

    list() {
        this.hotkeyOptions := loadHotkeys()
        hotkeyTable := ""
        for i, hotkey in this.hotkeyOptions {
            hotkeyTable .= "<tr>"
            hotkeyTable .= Format("<td><input type='checkbox' id='{}' {}></td>", "hotkey_" i "_enabled", hotkey.enabled ? "checked" : "")
            hotkeyTable .= Format("<td><input type='text' id='{}' size=12 value='{}'></td>", "hotkey_" i "_keyname", hotkey.name)
            hotkeyTable .= "<td>" hotkey.description "</td>"
            hotkeyTable .= "</tr>"
        }
        hotkeyTable .= "<tr><td></td><td>F6</td><td>Dump inventory items</td></tr>"
        hotkeyTable .= "<tr><td></td><td>Ctrl+F6</td><td>Dump highlighted or all items from stash tab</td></tr>"
        hotkeyTable .= "<tr><td></td><td>F7</td><td>Sell full rare sets to vendor</td></tr>"
        hotkeyTable .= "<tr><td></td><td>Ctrl+d</td><td><b style='color:darkred'>[patreon only]</b> Check the price of selected item or items in nearby curio displays</td></tr>"
        hotkeyTable .= "<tr><td></td><td>Ctrl+Shift+d</td><td><b style='color:darkred'>[patreon only]</b> Check the price of items in current stash tab or inventory</td></tr>"
        this._("#hotkeys").innerHtml := hotkeyTable
    }

    ok() {
        for i, hotkey in this.hotkeyOptions {
            hotkey.name := this._("#hotkey_" i "_keyname").value
            hotkey.enabled := this._("#hotkey_" i "_enabled").checked
        }

        if (saveHotkeys(this.hotkeyOptions))
            this.close()
    }

    cancel() {
        this.close()
    }
}

class Feedback extends WebGui {

    static tLog := Feedback.log()

    __new() {
        base.__new("Send feedback",, 700, 400)
        this.document.write("
        (%
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                * { font-family: Georgia, Serif; line-height: 1.5; }
                html, body { display: flex; flex-flow: column; height: 100%; background: #f0f0f0; margin: 0; }
                textarea { flex: 2 1 auto; background-color: white; font-size: 16px; border: 1px solid #adadad; margin: 5px; padding: 5px; }
                textarea:focus { border-color: #0078d7; }
                input { border: 1px solid #adadad; padding: 0 5px 5px; }
                input:focus { border-color: #0078d7; }
                table { margin: 5px; }
                span { flex: 0 1 auto; font-size: 16px; margin: 5px; }
                button { font-family: Calibri; background-color: #e1e1e1; border: 1px solid #adadad; margin: 5px 2px; float: right; transition: 0.4s; padding: 0 30px; }
                button:focus { outline: solid; outline-width: 1px; outline-color: #0078d7; }
                button:hover { background-color: #e5f1fb; outline: solid; outline-width: 1px; outline-color: #0078d7; }
            </style>
        </head>
        <body>
            <span>Requests or Issues:</span>
            <textarea id=content></textarea>
            <span>E-mail (optional): <input type='email' id='email' size=32></input></span>
            <span>
                <button id='cancel'>Cancel</button>
                <button id='send'>Send</button>
            </span>
        </body>
        </html>
        )")
        this.document.close()
        this.content := this.document.getElementById("content")
        this.email := this.document.getElementById("email")
        this.content.focus()
        this.bindAll("button")

        if (ptask.player.name)
            this.name.value := ptask.player.name
    }

    log() {
        if (not t := Feedback.tLog) {
            t := ObjBindMethod(Feedback, "log")
            SetTimer, %t%, % 1800 * 1000
        }

        if (ptask.isReady) {
            lastLogTime := db.load("last_log_time")
            tPeriod -= lastLogTime, Days
            if (not lastLogTime || tPeriod > 0) {
                FormatTime, logTime, A_NowUTC, yyMMdd
                url := "https://docs.google.com/forms/d/e"
                curl.ajax(Format("{}/{}/formResponse?entry.1300113286={}&entry.1468854439={}&entry.512599937={}&entry.1915618276={}"
                                , url, "1FAIpQLSe8v9cU4Xq2WBDXxCVVl3CsCItLA32NL7-mK2UWhfjBQXhIBQ"
                                , ptask.player.className, curl.escape(ptask.league), logTime, language))
                db.store("last_log_time", A_NOW)
            }
        }

        return t
    }

    send() {
        curl.ajax(Format("https://docs.google.com/forms/d/e/{}/formResponse?entry.1794050980={}&entry.80285333={}"
                        , "1FAIpQLSfLcA6KG9gE7JoNOr75x2ZMXsa4SGyPqVVp9M78gsZhhWrB4Q"
                        , js.encodeURI(this.content.value)
                        , js.encodeURI(this.email.value)))
        this.close()
    }

    cancel() {
        this.close()
    }
}

class Navi extends WebGui {

    __new() {
        base.__new(, "+AlwaysOnTop +Toolwindow -Caption -Resize")
        Gui, Color, 0
        WinSet, TransColor, 0 210

        this.document.write("
        (%
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                * { font-family: Fontin SmallCaps, Serif; font-size: 18px; }
                body { background: black; border: 0; margin: 0; padding: 0; }
                canvas { position: fixed; left: 0; top: 0; right: 0; bottom: 0; z-index: -1; } 

                .nav { position: fixed; right: 0; }
                .nav table { border-collapse: collapse; }
                .nav_bar { display: none; border-collapse: collapse; }
                .nav_bar td { padding: 0 12px; }

                #toggle { color: lightgrey; background-color: brown; padding: 0 5px; }
                #area_time { font-family: Arail Narrow; font-weight: bold; color: #0c0c0c; background-color: lightblue; }
                .second { font-family: Arail Narrow; color: maroon; }
                #kill_counter { color: #0c0c0c; background-color: lightcyan; display: none; }
                #kill_counter:hover { background-color: cyan; }
                .kills { color: crimson; }
                .total { color: green; }
                #exp { color: #0c0c0c; background-color: yellow; display: none; }
                .gained_exp { color: blue; }
                #menu { color: white; background-color: #0c0c0c; }

                .stats { color: white; background: #1a1411; position: fixed; left: 50%; top: 80px; transform: translate(-50%, 0); width: 1000px; max-height: 600px; overflow: auto; }
                .stats table { text-align: right; }
                .stats tr { background: #120e0a; color: #b57741; padding: 5px; transition: .1s ease-in; } 
                .stats th { border-bottom: 2px solid brown; text-align: left; padding: .2rem; white-space: nowrap; }
                .stats td { padding: .5rem; }
                .stats tr:first-child { background: #1a1411; color: #d9aa6f; font-size: 1rem; }
                .stats tr:hover:not(:first-child) { background: #d9aa6f; color: #120e0a; }
                .statusbar { display: none; line-height: 1.5em; color: lightyellow; background: #1a1411; padding: 5px 15px; position: fixed; left: 50%; top: 80px; transform: translate(-50%, 0); width: 800px; max-height: 200px; overflow: hidden; }
            </style>
        </head>
        <body oncontextmenu='return false;'>
            <canvas></canvas>
            <div class=nav>
                <table><tr>
                    <td id='toggle'>&#187;</td>
                    <td><table class='nav_bar'><tr>
                        <td id='area_time'>--:--</td>
                        <td id='kill_counter'>Kills <b class=kills>0</b>/<b class=total>0<b></td>
                        <td id='exp'>Exp <b class='gained_exp'>+0.000%</b></td>
                        <td id='menu'>&#8801;</td>
                    </tr></table></td>
                </tr></table>
            </div>
            <div class='stats' align='center'>
                <table id='kill_stats'></table>
            </div>

            <div class='statusbar' align='left'>
            </div>

            <script>
                var canvas = document.querySelector('canvas');
                var ctx = canvas.getContext('2d', {alpha:false});
                var usedTime = 0, enterTime = 0;

                function setkills(kills, total, gainedExp) {
                    document.querySelector('.kills').innerText = kills;
                    document.querySelector('.total').innerText = total;
                    document.querySelector('.gained_exp').innerText = gainedExp;
                }

                ctx.fillRoundedRect = function (x, y, w, h, r) {
                    this.beginPath();
                    this.moveTo(x + r, y);
                    this.arcTo(x + w, y, x + w, y + h, r);
                    this.arcTo(x + w, y + h, x, y + h, r);
                    this.arcTo(x, y + h, x, y, r);
                    this.arcTo(x, y, x + w, y, r);
                    this.fill();
                }

                ctx.setFont = function (font) {
                    ctx.font = font;
                    ctx.fontHeight = parseInt(font.match('[0-9]+'));
                }

                ctx.clear = function () {
                    ctx.clearRect(0, 0, canvas.width, canvas.height)
                }

                ctx.drawLine = function (x1, y1, x2, y2, color) {
                    ctx.strokeStyle = color;
                    ctx.moveTo(x1, y1);
                    ctx.lineTo(x2, y2);
                    ctx.stroke();
                }

                ctx.drawRect = function (x, y, w, h, color) {
                    ctx.strokeStyle = color;
                    ctx.strokeRect(x, y, w, h);
                }

                ctx.drawGrid = function (x, y, w, h, rows, cols, color) {
                    ctx.strokeStyle = color;
                    ctx.strokeRect(x, y, w, h);

                    x1 = x + w;
                    for (let i = 1; i < rows; i++) {
                        y1 = y + i * (h / rows) - 1;
                        ctx.moveTo(x, y1);
                        ctx.lineTo(x1, y1);
                    }

                    y1 = y + h;
                    for (let i = 1; i < cols; i++) {
                        x1 = x + i * (w / cols) - 1;
                        ctx.moveTo(x1, y);
                        ctx.lineTo(x1, y1);
                    }
                    ctx.stroke();
                }

                ctx.drawText = function (text, x, y, color, backgroud, align, baseline) {
                    w = ctx.measureText(text).width;
                    x = x - w * (1 - align) / 2;
                    y = y + ctx.fontHeight * (baseline + 1) / 2;
                    if (backgroud) {
                        ctx.fillStyle = backgroud;
                        ctx.fillRoundedRect(x - 2.5, y - ctx.fontHeight, w + 5, ctx.fontHeight, 7);
                    }

                    ctx.fillStyle = color;
                    ctx.fillText(text, x, y);
                }

                setInterval(function() {
                    tt = usedTime + enterTime++;
                    document.querySelector('#area_time').innerHTML =
                        String('0' + parseInt(tt / 60)).slice(-2) + ':<span class=second>' +
                        String('0' + (tt % 60)).slice(-2) + '</span>';                        
                }, 1000);

                window.addEventListener('resize', function () {
                    canvas.width = window.innerWidth
                    canvas.height = window.innerHeight
                    ctx.textBaseline = 'bottom'
                    ctx.setFont('16px Fontin Smallcaps')
                });
            </script>
        </body>
        </html>
        )")
        this.document.close()
        this.window := this.document.parentWindow
        this.bind("menu", "onmouseenter")
        this.bind("menu", "onmouseleave")
        this.bind("menu", "onclick", "showMenu")
        this.bind("toggle",, "toggle")
        this.bind("kill_counter",, "showKillStats")
        this.bind("kill_stats", "onfocusout", "hideKillStats")

        addMenuItem("__main", _("Hideout"), ObjBindMethod(ptask, "sendKeys", "/hideout", 0))
        addMenuItem("__main", _("Sell items"), ObjBindMethod(ptask, "sellItems"))
        addMenuItem("__main", _("Stash"), ObjBindMethod(ptask, "stashItems"))
        if (extraMenus.Count()) {
            addMenuItem("__main")
            for i, m in extraMenus
                addMenuItem("__main", m.name, m.handler, m.options)
        }
        addMenuItem("__main")
        addMenuItem("__main", _("Send feedback..."), ObjBindMethod(this, "sendFeedback"))
        addMenuItem("__main", _("Hotkeys..."), ObjBindMethod(this, "hotkeys"))
        addMenuItem("__main", _("About") " PoEapikit...", ObjBindMethod(this, "about"))
        addMenuItem("__main")
        addMenuItem("__main", _("Reload"), "Reload")
        addMenuItem("__main", _("Quit"), "ExitApp")

        this.menu := this._("#menu")
        this.killStats := this._("#kill_stats")
        this.areaTime := this._("#area_time")
        this.kc := ptask.getPlugin("KillCounter")
        this.setVisible((db.load("nav.visible") != 0) ? true : false)
        this.onMessage(WM_KILL_COUNTER, "onKillCounter")
        this.onMessage(WM_AREA_CHANGED, "onAreaChanged")

        this.statusbar := this._(".statusbar")
        this.statusTimer := ObjBindMethod(this, "setStatus")

        this.usedTime := 0
        this.enterTime := A_Tickcount
    }

    getCanvas() {
        return this.window.ctx
    }

    about() {
        new About().show()
    }

    hotkeys() {
        new Hotkeys().show()
    }

    sendFeedback() {
        new Feedback().show()
    }

    close() {
        Menu, __main, Delete
        base.close()
    }

    show(options = "") {
        if (options ~= "Hide") {
            base.show(options)
            return
        }

        r := ptask.getClientRect()
        base.show(options " NoActivate")
        WinMove, % "ahk_id " this.Hwnd,, r.l, r.t, r.w, r.h
    }

    onmouseenter() {
        MouseGetPos, x, y, hwnd
        if (hwnd == this.Hwnd)
            this.menu.style.background := "red"
    }

    onmouseleave() {
        this.menu.style.background := "#0c0c0c"
    }

    showMenu() {
        Menu, __main, Show
    }

    setVisible(flag) {
        this.visible := flag
        this._("#toggle").innerHtml := flag ? "&#187;" : "&#171;"
        this._(".nav_bar").style.display := flag ? "table" : "none"
        db.store("nav.visible", this.visible, "All")
    }

    toggle() {
        this.setVisible(not this.visible)
    }

    showKillStats() {
        statsTable := "
        (
        <tr>
            <th>Time</th><th>Area Name</th><th>Level</th><th>Kills</th><th>Gained Exp(%)</th><th>Normal</th><th>Magic</th><th>Rare</th><th>Unique</th><th>Used Time</th>
        </tr>
        )"

        stats := this.kc.getStats()
        if (Not stats.Count()) {
            this.killStats.innerHtml := "<tr><td>No Kills</td></tr>"
            this.killStats.focus()
            return
        }

        for i, stat in stats {
            if (stat.totalMonsters == 0)
                continue

            statsTable .= "<tr>"
            statsTable .= "<td>"  stat.timestamp "</td>"
            statsTable .= "<td style=""text-align:left""><strong>"  stat.areaName "</strong></td>"
            statsTable .= "<td>"  stat.areaLevel "</td>"
            statsTable .= "<td>"  stat.totalKills "/" stat.totalMonsters "</td>"
            statsTable .= Format("<td>{} ({:.2f}%)</td>", stat.gainedExp, stat.gainedExp * 100 / levelExp[stat.playerLevel])
            statsTable .= "<td style=""color:#FFFAFA"">"  stat.normalKills "</td>"
            statsTable .= "<td style=""color:#8787FE"">"  stat.magicKills "</td>"
            statsTable .= "<td style=""color:#FEFE76"">"  stat.rareKills "</td>"
            statsTable .= "<td style=""color:#AF5F1C"">"  stat.uniqueKills "</td>"
            statsTable .= Format("<td>{}m {:02d}s</td>", stat.usedTime // 60, Mod(stat.usedTime, 60))
            statsTable .= "</tr>"
        }
        this.killStats.innerHtml := statsTable
        this.killStats.focus()
    }

    hideKillStats() {
        if (Not this.document.hasFocus())
            this.killStats.innerHtml := ""
    }

    updateTime() {
        try {
            tt := this.usedTime + (A_Tickcount - this.enterTime) / 1000
            , this.areaTime.innerHtml := Format("{:02d}:<span class='second'>{:02d}</span>", tt / 60, Mod(tt, 60))
        } catch {
            SetTimer,, Delete
        }
    }

    setStatus(text = "") {
        if (text) {
            this.statusbar.style.display := "block"
            this.statusbar.innerHtml .= text "<br>"
            t := this.statusTimer
            SetTimer, %t%, -3000
            this.statusbar.scrollTop := this.statusbar.scrollHeight
        } else {
            this.statusbar.innerHtml := ""
            this.statusbar.style.display := "none"
        }
    }

    onAreaChanged() {
        if (this.kc.enabled) {
            this._("#kill_counter").style.display := ptask.InMap ? "table-cell" : "none"
            this._("#exp").style.display := ptask.InMap ? "table-cell" : "none"
            stat := this.kc.getStat()
        } else {
            this._("#kill_counter").style.display := "none"
            this._("#exp").style.display := "none"
        }
        this.window.usedTime := stat ? stat.usedTime : 0
        this.window.enterTime := 0
    }

    onKillCounter(wParam, lParam) {
        try {
            level := lparam & 0xff
            , gainedExp := (lParam >> 16) * 100 / levelExp[level]
            , this.window.setKills(wParam & 0xffff, wParam >> 16, Format("{:+.3f}%", gainedExp))
        } catch {}
    }
}
