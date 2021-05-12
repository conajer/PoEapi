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
             , "Watchstones"        : {"catalog" : "item", "type" : "Watchstone"}
             , "Base Types"         : {"catalog" : "item", "type" : "BaseType"} }

    influenceTypes := ["Shaper", "Elder", "Crusader", "Redeemer", "Hunter", "Warlord"]
    league := ""
    lastUpdateTime := ""
    updatePeriod := 30 * 60000
    lowConfidence := false
    prices := {}

    __new() {
        base.__new()
        this.onMessage(WM_AREA_CHANGED, "__onAreaChanged")
    }

    addPrice(type, p) {
        try {
            if (p.hasOwnProperty("chaosEquivalent")) {
                this.prices[p.currencyTypeName] := {"value" : p.chaosEquivalent}
            } else {
                if (!lowConfidenceSparkline && p.sparkline.data.length == 0)
                    return
                pName := p.name
                switch type {
                case "Map":
                    if (Not (p.name ~= "Essence of")) {
                        if (Not (p.name ~= p.baseType))
                            this.prices[p.baseType " unique T" p.mapTier] := {"value" : p.chaosValue}
                        pName .= " T" p.mapTier
                    }
                case "SkillGem":
                    quality := p.hasOwnProperty("gemQuality") ? p.gemQuality : 0
                    if (p.hasOwnProperty("corrupted")) {
                        if (p.gemLevel > 1 && p.gemLevel < 20)
                            this.prices[pName " " p.gemLevel "/0 corrupted"] := {"value" : p.chaosValue}
                        pName .= " " p.gemLevel "/" quality " corrupted"
                    } else {
                        if (p.gemLevel > 1 && p.gemLevel < 20)
                            this.prices[pName " " p.gemLevel "/0"] := {"value" : p.chaosValue}
                        pName .= " " p.gemLevel "/" quality
                    }
                case "Prophecy":
                    if (p.hasOwnProperty("variant"))
                        pName .= " " p.variant
                case "UniqueWeapon":
                case "UniqueArmour":
                    if (p.hasOwnProperty("links"))
                        pName .= " " p.links "L"
                case "BaseType":
                    if (p.hasOwnProperty("variant"))
                        pName .= " " p.variant
                    if (p.levelRequired >= 82)
                        pName .= " " p.levelRequired
                }
                
                this.prices[pName] := {"value" : p.chaosValue}
            }
        } catch {
        }
    }

    getPrice(item) {
        if (Not IsObject(item))
            return this.prices[item].value

        qNames := [item.name]
        if (item.isMap) {
            if (item.rarity < 3)
                qNames[1] := item.baseName
            if (item.isBlighted())
                qNames[1] := "Blighted " item.baseName
            if (item.rarity == 3) {
                if (Not item.isIdentified)
                    qNames[1] := item.baseName " unique"
                qNames[2] := item.name
            }
            qNames[1] .= " T" item.tier
        } else if (item.isGem) {
            level := item.level
            , quality := item.quality
            , qNames[1] .= " " level "/" quality
            , qNames.Push(item.name " " level "/0")

            , level := (level < 19) ? 1 : 20
            , qNames.Push(item.name " " level "/" quality)

            , quality := (quality < 19) ? 0 : 20
            , qNames.Push(item.name " " level "/" quality)
            , qNames.Push(item.name " " level "/0")

            if (item.name ~= "Anomalous|Divergent|Phantasmal")
                qNames.Push(item.name " 20/20")

            if (item.isCorrupted()) {
                for i in qNames
                    qNames[i] .= " corrupted"
            }
        } else if (item.isMapFragment) {
            qNames[1] := item.baseName
        } else if (item.isProphecy) {
            if (item.name == "A Master Seeks Help") {
                prophecy := item.getComponent("Prophecy")
                if (RegExMatch(prophecy.predictionText, "You will find (.*) and complete (his|her) mission.", matched))
                    qNames[1] .= " " matched1
            }
        } else if (item.rarity == 3 && item.baseName ~= "Cluster Jewel") {
            mods := item.getMods()
            if (RegExMatch(mods[2], "ExpansionJewelEmptyPassiveUnique__?([0-9])", matched))
                qNames[1] .= " " (matched1 * 2 - 1) " passives"
        } else {
            if (item.links() >= 5) {
                qNames[1] .= " " item.links() "L"
            } else 
            
            if (item.rarity < 3 && (ilvl := item.itemLevel()) >= 82) {
                ilvl := (ilvl >= 86) ? 86 : ilvl
                qNames[1] := item.baseName
                influence := item.getInfluenceType()
                if (influence)
                    qNames[1] .= " " this.influenceTypes[Floor(Log(influence)/Log(2)) + 1]

                if (ilvl > 82)
                    qNames[2] := qNames[1] " " (ilvl - 1)
                qNames[1] .= " " ilvl
            }
        }

        for i, name in qNames {
            if (this.prices[name].value)
                return this.prices[name].value
        }
    }

    update() {
        http := ComObjCreate("WinHttp.WinHttpRequest.5.1")
        for name, t in this.types {
            url := Format(this.url, t.catalog, this.league, t.type)
            try {
                http.Open("GET", url, true)
                http.Send()
                http.WaitForResponse()
                parsed := this.document.parentWindow.JSON.parse(http.ResponseText)
                callback := ObjBindMethod(this, "addPrice", t.type)
                rdebug("#PRICER", "<b style=""background-color:gold;color:black"">Loading prices of {} ... {}</b>", name, parsed.lines.length)
                parsed.lines.forEach(callback)
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
            this.prices := {"Chaos Orb" : {"value" : 1}}
            this.league := ptask.league
            t := ObjBindMethod(this, "update")
            SetTimer, %t%, -1000
        }
    }
}
