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
                <a href='#github'>GitHub</a>
                <a href='#latest_version'>Latest version</a>
                <a href='#donate'>Donate</a>
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

        this.document.querySelector("#version").innerText := version
        this.document.querySelector("#poeapi_version").innerText := poeapiVersion
        this.document.querySelector("#ahk_version").innerText := A_AhkVersion
    }

    ok() {
        this.close()
    }

    onclick(e) {
        switch (e.srcElement.href) {
        case "#github": url := "https://github.com/conajer/PoEapi"
        case "#latest_version": url := "https://github.com/conajer/PoEapi/releases/latest"
        case "#donate": url := "https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=TDU8JSQAXNTU6&currency_code=USD"
        }
        Run, % url
    }
}

class Hotkeys extends WebGui {

    __new() {
        base.__new("Hotkeys",, 640, 800)
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
                td:nth-child(1) { color: blue; text-align: right; font-weight: bold; }
                td:nth-child(2) { color: dimgray; padding: 0 15px; }
            </style>
        </head>
        <body>
            <div>
                <h2>Hotkeys</h2>
                <table>
                    <tr><td>``</td><td>Exit to character selection</td><tr>
                    <tr><td>a</td><td>Auto pickup/levelup gems</td><tr>
                    <tr><td>F1</td><td>Auto aruas</td><tr>
                    <tr><td>F2</td><td>Auto portal</td><tr>
                    <tr><td>F3</td><td>Auto identify/sell</td><tr>
                    <tr><td>Alt + F3</td><td>Identify all and sell</td><tr>
                    <tr><td>F4</td><td>Auto stash</td><tr>
                    <tr><td>F5</td><td>Hideout</td><tr>
                    <tr><td>Ctrl + F5</td><td>Azurite mine</td><tr>
                    <tr><td>Alt + F5</td><td>Menagerie</td><tr>
                    <tr><td>F6</td><td>Dump inventory items</td><tr>
                    <tr><td>Ctrl + F6</td><td>Dump highlighted or all items from current stash tab</td><tr>
                    <tr><td>F7</td><td>Sell full rare sets to vendor</td><tr>
                    <tr><td>F12</td><td>Toggle log window</td><tr>
                    <tr><td>Left Alt</td><td>Hold down the left Alt key to show price of items</td><tr>
                    <tr><td>Windows + d</td><td>Minimize PoE window</td><tr>
                    <tr><td>Ctrl + m</td><td>Toggle maphack</td><tr>
                    <tr><td>Ctrl + r</td><td>Reload script</td><tr>
                    <tr><td>Ctrl + q</td><td>Quit PoEapikit</td><tr>
                </table>
            </div>
            <span>
                <button id='ok'>OK</button>
            </span>
        </body>
        </html>
        )")
        this.document.close()
        this.bindAll("button")

        this.document.querySelector("#version").innerText := version
        this.document.querySelector("#poeapi_version").innerText := poeapiVersion
    }

    ok() {
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
                
                .nav table { border-collapse: collapse; }
                .nav td { margin: 0 10px; padding: 1px 15px; }

                #area_time { font-family: Arail Narrow; font-weight: bold; color: #0c0c0c; background-color: lightgreen; }
                .second { font-family: Arail Narrow; color: maroon; }
                #kill_counter { color: #0c0c0c; background-color: lightcyan; }
                #kill_counter:hover { background-color: cyan; }
                .kills { color: crimson; }
                .total { color: green; }
                 
                .exp { color: #0c0c0c; background-color: yellow; }
                .gained_exp { color: blue; }
                
                #menu { color: white; background-color: #0c0c0c; }
                
                .stats { color: white; background: #1a1411; position: fixed; left: 50%; top: 80px; transform: translate(-50%, 0); width: 1000px; max-height: 600px; overflow: auto; }
                .stats table { text-align: right; }
                .stats tr { background: #120e0a; color: #b57741; padding: 5px; transition: .1s ease-in; } 
                .stats th { border-bottom: 2px solid brown; text-align: left; padding: .2rem; white-space: nowrap; }
                .stats td { padding: .5rem; }
                .stats tr:first-child { background: #1a1411; color: #d9aa6f; font-size: 1rem; }
                .stats tr:hover:not(:first-child) { background: #d9aa6f; color: #120e0a; }
            </style>
        </head>
        <body>
            <canvas></canvas>
            <div class=nav>
                <table class=nav align='right'>
                    <tr>
                        <td id='area_time'></td>
                        <td id='kill_counter'>Kills <b class=kills>0</b>/<b class=total>0<b></td>
                        <td class=exp>Exp <b class='gained_exp'>+0.000%</b></td>
                        <td id='menu'>&#8801;</td>
                    </tr>
                </table>
            </div>
            <div class='stats' align='center'>
                <table id='kill_stats'></table>
            </div>

            <script>
                var canvas = document.querySelector('canvas');
                var ctx = canvas.getContext('2d', {alpha:false});

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

                ctx.drawText = function (text, x, y, color, backgroud, align) {
                    w = ctx.measureText(text).width;
                    x = x - w * align / 2
                    if (backgroud) {
                        ctx.fillStyle = backgroud;
                        ctx.fillRoundedRect(x - 2.5, y - ctx.fontHeight, w + 5, ctx.fontHeight, 7);
                    }

                    ctx.fillStyle = color;
                    ctx.fillText(text, x, y);
                }

                window.addEventListener('resize', function () {
                    canvas.width = window.innerWidth
                    canvas.height = window.innerHeight
                    ctx.textBaseline = 'bottom'
                    ctx.setFont('18px Fontin Smallcaps')
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
        this.bind("kill_counter",, "showKillStats")
        this.bind("kill_stats", "onfocusout", "hideKillStats")

        addMenuItem("__main", _("Hideout"), ObjBindMethod(ptask, "sendKeys", "/hideout", 0))
        addMenuItem("__main", _("Sell items"), ObjBindMethod(ptask, "sellItems"))
        addMenuItem("__main", _("Stash"), ObjBindMethod(ptask, "stashItems"))
        addMenuItem("__main", _("Reload"), "Reload")
        if (extraMenus.Count()) {
            addMenuItem("__main")
            for i, m in extraMenus
                addMenuItem("__main", m.name, m.handler, m.options)
        }
        addMenuItem("__main")
        addMenuItem("__help", _("About PoEapikit"), ObjBindMethod(this, "about"))
        addMenuItem("__help", _("Hotkeys..."), ObjBindMethod(this, "hotkeys"))
        addMenuItem("__main", _("Help"), ":__help")
        addMenuItem("__main")
        addMenuItem("__main", _("Quit"), "ExitApp")

        this.menu := this.document.querySelector("#menu")
        this.killStats := this.document.querySelector("#kill_stats")
        this.areaTime := this.document.querySelector("#area_time")
        this.kc := ptask.getPlugin("KillCounter")
        this.onMessage(WM_KILL_COUNTER, "onKillCounter")
        this.onMessage(WM_AREA_CHANGED, "onAreaChanged")

        t := ObjBindMethod(this, "updateTime")
        SetTimer, %t%, 1000
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
        tt := this.usedTime + (A_Tickcount - this.enterTime) / 1000
        , this.areaTime.innerHtml := Format("{:02d}:<span class='second'>{:02d}</span>", tt / 60, Mod(tt, 60))
    }

    onAreaChanged() {
        stat := this.kc.getStat()
        this.usedTime := stat ? stat.usedTime : 0
        this.enterTime := A_Tickcount
    }

    onKillCounter(wParam, lParam) {
        level := lparam & 0xff
        , gainedExp := (lParam >> 16) * 100 / levelExp[level]
        , this.window.setKills(wParam & 0xffff, wParam >> 16, Format("{:+.3f}%", gainedExp))
    }
}
