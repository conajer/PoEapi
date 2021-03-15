;
; logger.ahk, 8/30/2020 1:33 PM
;

class Logger extends WebGui {

    __new(title = "", filename = "", level = 0) {
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

                .timestamp { color: dimgray;
            </style>
        </head>
        <body>
            <div>
                <pre>
        )")
        this.div := this.document.querySelector("div")

        this.filename := filename
        this.logFile := FileOpen(filename, "a", "utf-8")
        this.level := level
        this.logId := 0
        this.show("Hide x10 y50")
    }

    log(aText, id = "", level = 0) {
        if (id == "") {
            ; Increment logId by 1
            this.logId += 1
            id := "#" this.logId
        }

        FormatTime, t,, MM/dd hh:mm:ss
        aText := Format("<span id=""{}""><span class=timestamp>{}</span> {}</span>"
                       , id, t, aText)

        if (level >= LogLevel) {
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
}

debug(string, args*) {
    logger.log(Format("[D] <i>" string "</i>", args*), 0)
}

rdebug(id, string, args*) {
    logger.rlog(id, Format("[D] <i>" string "</i>", args*), false, 0)
}

adebug(id, string, args*) {
    logger.rlog(id, Format("[D] <i>" string "</i>", args*), true, 0)
}

syslog(string, args*) {
    logger.log(Format("[I] " string, args*),, 1)
}

rsyslog(id, string, args*) {
    logger.rlog(id, Format("[I] " string, args*), true, 1)
}

error(string, args*) {
    logger.log(Format("[E]<span style=""color: red""> !!! " string "</span>", args*), 2)
    SendInput {F8}
}
