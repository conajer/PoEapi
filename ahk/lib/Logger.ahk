;
; logger.ahk, 8/30/2020 1:33 PM
;

class Logger extends WebGui {

    static __l := Logger.__new()
    static level := 1

    __new(title = "", filename = "") {
        base.__new(title,, 900, 600)
        this.document.write("
        (%
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                * { font-family: Courier New, Monospace; }
                body, html { height: 100%; background-color: #f0f0f0; overflow: hidden; }
                div { background-color: white; border: 1px solid; margin: 5px; padding: 0px 10px; overflow: auto; position: fixed; left: 0; top: 0; right: 0; bottom:0; }
                pre { font-size: 15px; line-height: 1.2; }

                .timestamp { color: dimgray; }
            </style>
        </head>
        <body>
            <div>
                <pre>
        )")
        this.div := this.document.querySelector("div")

        this.filename := filename
        this.logFile := FileOpen(filename, "a", "utf-8")
        this.logId := 0

        x := y := 50
        SysGet, n, MonitorCount
        if (n > 1) {
            SysGet, primary, MonitorPrimary
            m := (primary == 1) ? 2 : 1
            SysGet, coords , Monitor, %m%
            x += coordsLeft
            y += coordsTop
        }
        this.show("Hide x" x " y" y)
    }

    log(aText, id = "", level = 0) {
        ; Increase logId by 1
        this.logId += 1
        id := (Not id) ? "#" this.logId : id

        FormatTime, t,, MM/dd hh:mm:ss
        aText := Format("<span id=""{}""><span class=timestamp>{}</span> {}</span>"
                       , id, t, aText)

        if (level >= this.level) {
            this.document.writeln(aText)
            this.div.scrollTop := this.div.scrollHeight

            return id
        }
    }

    rlog(id, aText, append = false, level = 0) {
        element := this.document.getElementById(id)
        if (Not element)
            return this.log(aText, id, level)

        FormatTime, t,, MM/dd hh:mm:ss
        if (append)
            element.innerHtml .= Format("<br><span style=""color:black"">{}</span> {}", t, aText)
        else
            element.innerHtml := Format("<span style=""color:black"">{}</span> {}", t, aText)
    }

    show(options = "", title = "") {
        if (options || title) {
            base.show(options, title)
        } else {
            hwnd := this.Hwnd
            WinGet, state, MinMax, ahk_id %hwnd%
            Gui %hwnd%: +AlwaysOnTop
            base.show((state == -1) ? "NoActivate" : "NA")
            Gui %hwnd%: -AlwaysOnTop
        }
    }

    __onClose(wParam, lParam, msg, hwnd) {
    }
}

trace(string, args*) {
    return logger.log(Format("[T] <span style=""color: grey"">" string "</span>", args*),, 0)
}

debug(string, args*) {
    return logger.log(Format("[D] <i>" string "</i>", args*),, 1)
}

rdebug(id, string, args*) {
    logger.rlog(id, Format("[D] <i>" string "</i>", args*), false, 1)
}

adebug(id, string, args*) {
    logger.rlog(id, Format("[D] <i>" string "</i>", args*), true, 1)
}

syslog(string, args*) {
    return logger.log(Format("[I] " string, args*),, 2)
}

rsyslog(id, string, args*) {
    logger.rlog(id, Format("[I] " string, args*), true, 2)
}

error(string, args*) {
    SendInput {F8}
    return logger.log(Format("[E]<span style=""color: red""> !!! " string "</span>", args*),, 3)
}
