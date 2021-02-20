;
; Updater.ahk, 2/9/2021 11:05 AM
;

class Updater {

    url := "https://api.github.com/repos/conajer/PoEapi/releases/latest"

    __new() {
        global

        styleSheet =
        (
        <style>
            * { font-family: Calibri; }
            body { background: #F0F0F0; margin: 10px 5px; }
            div { background-color: white; font-size: 18px; border: 1px solid; backgroud: white; height: 350px; padding: 0px 5px; }
            button { background-color: #E1E1E1; border: 1px solid #ADADAD; margin: 5px 2px; float: right; transition: 0.4s; }
            button:focus { outline: solid; outline-width: 1px; outline-color: #0078d7; }
            button:hover { background-color: #E5F1FB; outline: solid; outline-width: 1px; outline-color: #0078d7; }
        </style>
        )

        Gui, __updater:New, +HwndHwnd +LastFound, Updates available
        Gui, Margin, 0, 0
        Gui, Add, ActiveX, r25 w600 v__mshtml, Shell.Explorer

        this.Hwnd := hwnd
        __mshtml.Navigate("about:<meta http-equiv=""X-UA-Compatible"" content=""IE=edge""/>")
        this.doc := __mshtml.Document
        this.doc.write(styleSheet)
        this.doc.write("<div id=release_notes></div>")
        this.doc.write("<button id=""cancel"">Cancel</button>")
        this.doc.write("<button id=""update"">Update</button>")
        this.doc.getElementById("update").onclick := ObjBindMethod(this, "update")
        this.doc.getElementById("cancel").onclick := ObjBindMethod(this, "cancel")

        t := ObjBindMethod(this, "check")
        SetTimer, % t, -2000
    }

    check() {
        http :=ComObjCreate("WinHttp.WinHttpRequest.5.1")
        try {
            http.Open("GET", this.url, true)
            http.Send()
            http.WaitForResponse()

            this.release := JSON.Load(http.ResponseText)
            if (RegExMatch(this.release.name, "PoEapikit-(.*)", matched)) {
                if (matched1 > version) {
                    innerHtml .= "<H1>" this.release.name "</H1>"
                    innerHtml .= "<H3>Release Notes:</H3>"
                    innerHtml .= RegExReplace(this.release.body, "\r\n", "</br>")
                    this.doc.getElementById("release_notes").innerHtml := innerHtml
                    Gui, __updater:Show

                    return true
                }
            }
        } catch {}
        SetTimer,, % 3600 * 1000

        return false
    }

    copy(src, dest) {
        count := src.Items().Count
        loop, % count {
            item := src.Items().Item(A_Index - 1)
            if (item.Name != "Settings.ahk")
                dest.CopyHere(item, 4|16|1024)
            Gui, __updater:Show,, % Format("Installing ... {:.f}%", A_Index * 100 / Count)
        }
    }

    update() {
        assets := this.release.assets
        EnvGet, tempDir, TEMP

        Gui, __updater:Show,, % Format("Downloading {} ...", assets[1].name)
        UrlDownloadToFile, % assets[1].browser_download_url, % tempDir "\" assets[1].name
        if (ErrorLevel) {
            MsgBox,, Updater, % "Failed to download '" assets[1].name "'!"
            return
        }

        shell := ComObjCreate("Shell.Application")
        package := shell.NameSpace(tempDir "\" assets[1].name)
        if (package) {
            __Exit()
            src := package.Items().Item(0).GetFolder
            dest := shell.NameSpace(A_ScriptDir)
            this.copy(src, dest)

            MsgBox,, Updater, % "Update completed, press 'OK' to reload."
            Reload
        } else {
            MsgBox,, Updater, % "'" tempDir "\" assets[1].name "' is not a valid release package."
        }
    }

    cancel() {
        Gui, __updater:Show, Hide
    }
}

checkUpdates() {
    if (Not updater.check())
        Msgbox, % "PoEapikit is up to date, version " version " (64-bit)"
}
