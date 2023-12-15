;
; Updater.ahk, 2/9/2021 11:05 AM
;

global updater := new Updater()

class Updater extends WebGui {

    url := "https://api.github.com/repos/conajer/PoEapi/releases/latest"

    __new() {
        base.__new("Updater",, 700, 400)
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
            </style>
        </head>
        <body>
            <div id=release_notes></div>
            <span>
                <button id='cancel'>Cancel</button>
                <button id='update'>Update</button>
            </span>
        </body>
        </html>
        )")
        this.document.close()
        this.bindAll("button")

        t := ObjBindMethod(this, "check")
        SetTimer, %t%, -2000
    }

    check() {
        if (Not version) {
            SetTimer,, -10000
            return
        }

        http := ComObjCreate("WinHttp.WinHttpRequest.5.1")
        try {
            http.Open("GET", this.url, true)
            http.Send()
            http.WaitForResponse()

            this.release := this.document.parentWindow.JSON.parse(http.ResponseText)
            if (RegExMatch(this.release.name, "PoEapikit-(.*)", matched)) {
                if (this.compareVersion(version, matched1) < 0) {
                    innerHtml .= "<H1>" this.release.name "</H1>"
                    innerHtml .= "<H3>Release Notes:</H3>"
                    innerHtml .= RegExReplace(this.release.body, "\r\n", "</br>")
                    this.document.getElementById("release_notes").innerHtml := innerHtml
                    this.show()

                    return true
                }
            }
        } catch {}
        SetTimer,, % 3600 * 1000

        return false
    }

    compareVersion(v1, v2) {
        v1 := StrSplit(v1, ".")
        v2 := StrSplit(v2, ".")

        loop, 3 {
            if (v1[A_Index] != v2[A_Index]) {
                r := Floor(v1[A_Index]) - Floor(v2[A_Index])
                return r ? r : ((v1[A_Index] > v2[A_Index]) ? 1 : -1)
            }
        }

        return 0
    }

    copy(src, dest) {
        count := src.Items().Count
        loop, % count {
            item := src.Items().Item(A_Index - 1)
            
            if (item.Name == "patreon.ahk") {
                if (FileExist(A_ScriptDir "\patreon.ahk"))
                    continue
            }

            if (item.Name == "patreon.dll" && ptask.isPatron)
                    continue
            }

            if (item.Name != "Settings.ahk")
                dest.CopyHere(item, 4|16|1024)
            this.show(, Format("Installing ... {:.f}%", A_Index * 100 / Count))
        }
    }

    update() {
        assets := this.release.assets
        EnvGet, tempDir, TEMP

        this.show(, Format("Downloading {} ...", assets[0].name))
        UrlDownloadToFile, % assets[0].browser_download_url, % tempDir "\" assets[0].name
        if (ErrorLevel) {
            MsgBox,, Updater, % "Failed to download '" assets[0].name "'!"
            return
        }

        shell := ComObjCreate("Shell.Application")
        package := shell.NameSpace(tempDir "\" assets[0].name)
        if (package) {
            __Exit()
            FileDelete, %A_ScriptDir%\extras\*.ahk
            FileDelete, %A_ScriptDir%\lib\*.ahk
            src := package.Items().Item(0).GetFolder
            dest := shell.NameSpace(A_ScriptDir)
            this.copy(src, dest)

            MsgBox,, Updater, % "Update completed, press 'OK' to reload."
            Reload
        } else {
            MsgBox,, Updater, % "'" tempDir "\" assets[0].name "' is not a valid release package."
        }
    }

    cancel() {
        this.hide()
    }
}
