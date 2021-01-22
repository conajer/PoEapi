;
; Pricer.ahk, 1/14/2021 12:23 PM
;

#Include, %A_ScriptDir%\lib\JSON.ahk

class Pricer {

    url := "https://poe.ninja/api/data/{}overview?league={}&type={}"
    currencyTypes := { "Currency"        : "Currency"
                     , "Fragment"        : "Fragments" }
    itemTypes :=     { "DeliriumOrb"     : "Delirium Orbs"
                     , "DivinationCard"  : "Divination Cards"

                     , "Essence"         : "Essences"
                     , "Fossil"          : "Fossils"
                     , "Incubator"       : "Incubators"
                     , "Oil"             : "Oils"
                     , "Prophecy"        : "Prophecies"
                     , "Resonator"       : "Resonators"
                     , "Scarab"          : "Scarabs"
                     , "UniqueAccessory" : "Unique Accessories"
                     , "UniqueArmour"    : "Unique Armours"
                     , "UniqueFlask"     : "Unique Flasks"
                     , "UniqueJewel"     : "Unique Jewels"
                     , "UniqueMap"       : "Unique Maps"
                     , "UniqueWeapon"    : "Unique Weapons"
                     , "Watchstone"      : "Watchstones" }
    league := ""
    lastUpdateTime := ""
    updatePeriod := 30 * 60000
    prices := {}

    __new() {
        OnMessage(WM_AREA_CHANGED, ObjBindMethod(this, "areaChanged"))
    }

    areaChanged() {
        if (ptask.league != this.league) {
            prices := {}
            this.league := ptask.league
            t := ObjBindMethod(this, "update")
            SetTimer, % t, -1000
        }
    }

    update() {
        http :=ComObjCreate("WinHttp.WinHttpRequest.5.1")
        for t, name in this.currencyTypes {
            url := Format(this.url, "currency", this.league, t)
            try {
                http.Open("GET", url, true)
                http.Send()
                http.WaitForResponse()

                parsed := JSON.Load(http.ResponseText)
                for i, p in parsed.lines
                    this.prices[p.currencyTypeName] := {"price" : p.receive.value, "type" : t}
                rdebug("#PRICER", "<b style=""background-color:gold;color:black"">Loaded prices of {} ... {}</b>"
                    , name, parsed.lines.Count())
            } catch {}
        }

        for t, name in this.itemTypes {
            url := Format(this.url, "item", this.league, t)
            try {
                http.Open("GET", url, true)
                http.Send()
                http.WaitForResponse()

                parsed := JSON.Load(http.ResponseText)
                for i, p in parsed.lines
                    this.prices[p.name] := {"price" : p.chaosValue, "type" : t}
                rdebug("#PRICER", "<b style=""background-color:gold;color:black"">Loaded prices of {} ... {}</b>"
                    , name, parsed.lines.Count())
             } catch {}
       }

        FormatTime, t,, MM/dd/yyyy hh:mm:ss
        this.lastUpdateTime := t
        SetTimer,, % this.updatePeriod
        rdebug("#PRICER", "<b style=""background-color:gold;color:black"">Total {} prices loaded.</b>", this.prices.Count())
    }

    __Get(item) {
        return this.prices[item.name].price
    }
}
