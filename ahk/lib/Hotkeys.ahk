;
; Hotkeys.ahk, 9/22/22 12:49 AM
;

; Hotkeys (^ for Ctrl, ! for Alt, + for Shift, # for window key)
;
;                           Enabled Prefix Key Name     Label              Description
;                           ------- ------ -----------  -----------------  --------------------------------------
global defaultHotkeys := [ [ true,  "",    "``",        "ExitGame",        "Exit to character selection"]
                         , [ true,  "~*",  "a",         "AutoPickup",      "Pickup nearby items"]
                         , [ true,  "$",   "q",         "QuickDefense",    "Quick defence actions"]
                         , [ true,  "~",   "s",         "LevelupGems",     "Level up skill gems"]
                         , [ true,  "~",   "w",         "Attack",          "Main attack skill"]
                         , [ false, "~",   "RButton",   "Attack",          "Secondary attack skill"]
                         , [ true,  "",    "#d",        "MinimizeWindow",  "Minimize PoE window"]
                         , [ true,  "",    "F1",        "AutoAuras",       "Auto aruas (ALT+F1 for performance stats)"]
                         , [ true,  "",    "F2",        "OpenPortal",      "Open portal"]
                         , [ true,  "*",   "F3",        "SellItems",       "Auto identify and sell items"]
                         , [ true,  "",    "F4",        "StashItems",      "Stash items"]
                         , [ true,  "",    "F5",        "Hideout",         "Enter hideout"]
                         , [ true,  "",    "^F5",       "Delve",           "Enter mine encampment"]
                         , [ true,  "",    "!F5",       "Menagerie",       "Enter menagerie"]
                         , [ true,  "",    "F12",       "ShowLog",         "Show log window"]
                         , [ true,  "~*",  "LAlt",      "ShowPrices",      "Show price of the item(s) in stash tab, inventory, etc."]
                         , [ true,  "*",   "^LButton",  "AutoCtrlClick",   "Hold to activate auto CTRL clicker"]
                         , [ true,  "~",   "+LButton",  "AutoShiftClick",  "Hold to activate auto SHIFT clicker"]
                         , [ true,  "",    "+RButton",  "AutoRButtonClick","Hold to activate auto right mouse button clicker"]
                         , [ true,  "~",   "^RButton",  "AutoFillPrice",   "Auto fill the price tag of the selected item"]
                         , [ false, "~*",  "^c",        "CopyItemName",    "Copy the selected item's name"]
                         , [ true,  "~",   "^f",        "HighlightItems",  "Highlight items in stash tab"]
                         , [ true,  "",    "^m",        "ToggleMaphack",   "Toggle maphack"]
                         , [ true,  "",    "^w",        "OpenWiki",        "Open wiki"]
                         , [ false, "",     "",         "tradeGems",       "Trade quality gems"]
                         , [ false, "",     "",         "tradeDivinationCards", "Trade divination cards"]
                         , [ false, "",     "",         "openStackedDecks","Open stacked decks"]
                         , [ false, "",     "",         "sortItems",       "Sort items"]
                         , [ true,  "",    "^r",        "Reload",          "Reload script"]
                         , [ true,  "",    "^q",        "ExitApp",         "Quit PoEapikit"] ]

class Hotkeys extends WebGui {

    __new() {
        base.__new("Hotkeys",, 800, 600)
        this.document.write("
        (%
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                * { font-family: Georgia, Serif; line-height: 1.5; }
                html, body { display: flex; flex-flow: column; height: 100%; background: #f0f0f0; margin: 0; }
                div { flex: 2 1 auto; font-size: 18px; margin: 5px 5px; padding: 0px 15px; overflow: auto; }
                span { flex: 0 1 auto; margin: 0px 5px 5px; }
                button { font-family: Calibri; background-color: #e1e1e1; border: 1px solid #adadad; min-width: 98px; margin: 5px 0 5px 10px; float: right; transition: 0.4s; padding: 0 30px; }
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

loadHotkeys() {
    try {
        hotkeyOptions := db.exec("SELECT * FROM hotkeys;")
        hotkeyIndex := []
        for i, hotkey in hotkeyOptions
            hotkeyIndex[hotkey.label] := hotkey

        for i, hotkey in defaultHotkeys {
            hkey := hotkeyIndex[hotkey[4]]
            if (Not hkey) {
                hkey := { "enabled": hotkey[1]
                        , "prefix": hotkey[2]
                        , "name": hotkey[3]
                        , "label": hotkey[4]
                        , "description": hotkey[5] }
                hotkeyOptions.Insert(i, hkey)
                hotkeyChanged := true
            } else if (hkey.prefix != hotkey[2]) {
                hkey.prefix := hotkey[2]
                hotkeyChanged := true
            }
        }

        if (hotkeyChanged)
            saveHotkeys(hotkeyOptions)
    } catch {
    } finally {
        if (Not hotkeyOptions) {
            db.exec("
            (
                DROP TABLE IF EXISTS hotkeys;
                CREATE TABLE hotkeys (
                    id INTEGER PRIMARY KEY,
                    enabled INTEGER,
                    prefix TEXT,
                    name TEXT,
                    label TEXT,
                    description TEXT);
            )")

            for i, hotkey in defaultHotkeys
                db.exec("INSERT INTO hotkeys VALUES ({}, {}, '{}', '{}', '{}', ""{}"");", i, hotkey*)
            hotkeyOptions := db.exec("SELECT * FROM hotkeys;")
        }
    }

    Hotkey, IfWinActive, ahk_class POEWindowClass
    for i, hotkey in hotkeyOptions {
        try {
            if (hotkey.enabled && hotkey.name)
                Hotkey, % hotkey.prefix . hotkey.name, % hotkey.label, On
        } catch e {
            hotkey.enabled := false
            db.exec("INSERT OR REPLACE INTO hotkeys VALUES ({}, {}, '{}', '{}', '{}', ""{}"");"
                    , i, hotkey.enabled, hotkey.prefix, hotkey.name, hotkey.label, hotkey.description)
            MsgBox, % "Hotkey error: " e.message
            return
        }
    }

    return hotkeyOptions
}

saveHotkeys(hotkeyOptions) {
    oldHotkeys := db.exec("SELECT * FROM hotkeys;")
    Hotkey, IfWinActive, ahk_class POEWindowClass
    for i, hotkey in hotkeyOptions {
        try {
            if (oldHotkeys[i].enabled && oldHotkeys[i].name)
                Hotkey, % oldHotkeys[i].prefix . oldHotkeys[i].name, Off
        } catch {}
        db.exec("INSERT OR REPLACE INTO hotkeys VALUES ({}, {}, '{}', '{}', '{}', ""{}"");"
                , i, hotkey.enabled, hotkey.prefix, hotkey.name, hotkey.label, hotkey.description)
    }

    ; reload the hotkeys
    return loadHotkeys()
}
