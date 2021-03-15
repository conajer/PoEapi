;
; Pricer.ahk, 1/14/2021 12:23 PM
;

global pricer := new Pricer()

class Pricer extends WebGui {

    url := "https://poe.ninja/api/data/{}overview?league={}&type={}"

    types := { "Currency"           : {"catalog" : "currency", "type" : "Currency"}
             , "Fragments"          : {"catalog" : "currency", "type" : "Fragment"}
             , "Delirium Orbs"      : {"catalog" : "item", "type" : "DeliriumOrb"}
             , "Divination Cards"   : {"catalog" : "item", "type" : "DivinationCard"}
             , "Essences"           : {"catalog" : "item", "type" : "Essence"}
             , "Fossils"            : {"catalog" : "item", "type" : "Fossil"}
             , "Incubators"         : {"catalog" : "item", "type" : "Incubator"}
             , "Invitations"        : {"catalog" : "item", "type" : "Invitation"}
             , "Maps"               : {"catalog" : "item", "type" : "Map"}
             , "Oils"               : {"catalog" : "item", "type" : "Oil"}
             , "Prophecies"         : {"catalog" : "item", "type" : "Prophecy"}
             , "Resonators"         : {"catalog" : "item", "type" : "Resonator"}
             , "Scarabs"            : {"catalog" : "item", "type" : "Scarab"}
             , "Skill Gems"         : {"catalog" : "item", "type" : "SkillGem"}
             , "Unique Accessories" : {"catalog" : "item", "type" : "UniqueAccessory"}
             , "Unique Armours"     : {"catalog" : "item", "type" : "UniqueArmour"}
             , "Unique Flasks"      : {"catalog" : "item", "type" : "UniqueFlask"}
             , "Unique Jewels"      : {"catalog" : "item", "type" : "UniqueJewel"}
             , "Unique Maps"        : {"catalog" : "item", "type" : "UniqueMap"}
             , "Unique Weapons"     : {"catalog" : "item", "type" : "UniqueWeapon"}
             , "Watchstones"        : {"catalog" : "item", "type" : "Watchstone"} }

    league := ""
    lastUpdateTime := ""
    updatePeriod := 30 * 60000
    prices := {}

    __new() {
        base.__new()
        OnMessage(WM_AREA_CHANGED, ObjBindMethod(this, "__onAreaChanged"))
    }

    addPrice(p) {
        if (p.hasOwnProperty("chaosEquivalent")) {
            this.prices[p.currencyTypeName] := {"value" : p.chaosEquivalent}
        } else {
            if (p.mapTier) {
                if (Not (p.name ~= p.baseType))
                    this.prices["Unique " p.baseType " T" p.mapTier] := {"value" : p.chaosValue}
                pName := p.name " T" p.mapTier
            }
            else if (p.gemLevel && p.corrupted)
                pName := p.name " " p.gemLevel "/" p.gemQuality " corrupted"
            else if (p.gemLevel)
                pName := p.name " " p.gemLevel "/" p.gemQuality
            else
                pName := p.name
            this.prices[pName] := {"value" : p.chaosValue}
        }
    }

    getPrice(item) {
        qName := item.name
        ratio := 1
        if (item.isMap) {
            if (item.rarity < 3)
                qName := item.baseName
            if (item.isBlighted())
                qName := "Blighted " item.baseName
            qName .= " T" item.tier
            if (item.rarity == 3 && Not item.isIdentified)
                qName := "Unique " qName
        } else if (item.isGem) {
            level := item.level
            if (level < 20) {
                if (level >= 18)
                    level := 20
                else
                    level := 1
            }
            quality := item.quality
            if (quality < 20) {
                if (quality > 15)
                    quality := 20
                else
                    quality := 0                  
            }
            qName .= " " level "/" quality
            if (item.isCorrupted())
                qName .= " corrupted"
        } else if (item.isMapFragment && item.rarity > 0)
            qName := item.baseName

        return this.prices[qName].value * ratio
    }

    update() {
        http := ComObjCreate("WinHttp.WinHttpRequest.5.1")
        callback := ObjBindMethod(this, "addPrice")
        for name, t in this.types {
            url := Format(this.url, t.catalog, this.league, t.type)
            try {
                http.Open("GET", url, true)
                http.Send()
                http.WaitForResponse()
                parsed := this.document.parentWindow.JSON.parse(http.ResponseText)
                parsed.lines.forEach(callback)
                rdebug("#PRICER", "<b style=""background-color:gold;color:black"">Loading prices of {} ... {}</b>", name, parsed.lines.length)
            } catch {}
        }

        FormatTime, t,, MM/dd/yyyy hh:mm:ss
        this.lastUpdateTime := t
        SetTimer,, % this.updatePeriod
        rdebug("#PRICER", "<b style=""background-color:gold;color:black"">Total {} prices loaded.</b>", this.prices.Count())
    }

    dump(exp) {
        for name, p in this.prices {
            if (name ~= "i)"exp)
                debug(name ", " p.value)
        }
    }

    __onAreaChanged() {
        if (ptask.league != this.league) {
            this.prices := {}
            this.league := ptask.league
            t := ObjBindMethod(this, "update")
            SetTimer, %t%, -1000
        }
    }
}
