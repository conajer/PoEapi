;
; Pricer.ahk, 1/14/2021 12:23 PM
;

global pricer := new Pricer()

class Pricer {

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
             , "Beasts"             : {"catalog" : "item", "type" : "Beast"}
             , "Watchstones"        : {"catalog" : "item", "type" : "Watchstone"}
             , "Base Types"         : {"catalog" : "item", "type" : "BaseType"} }

    influenceTypes := ["Shaper", "Elder", "Crusader", "Redeemer", "Hunter", "Warlord"]
    league := ""
    lastUpdateTime := ""
    updatePeriod := 30 * 60000
    lowConfidence := false
    prices := {}

    __new() {
        OnMessage(WM_AREA_CHANGED, ObjBindMethod(this, "__onAreaChanged"))
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
                    pName .= " T" p.mapTier
                case "UniqueMap":
                    pName .= " T" p.mapTier
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
                case "UniqueWeapon":
                    if (p.hasOwnProperty("links"))
                        pName .= " " p.links "L"
                case "UniqueArmour":
                    if (p.hasOwnProperty("links"))
                        pName .= " " p.links "L"
                case "BaseType":
                    if (p.levelRequired >= 82)
                        pName .= " " p.levelRequired
                }

                if (p.hasOwnProperty("variant"))
                    pName .= " " p.variant

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
        } else if (item.isMapFragment || item.isBeast) {
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
        } else if (Not item.isCurrency) {
            if (item.links() >= 5) {
                qNames.Push(qNames[1])
                qNames[1] .= " " item.links() "L"
            }

            if (item.rarity < 3 && (ilvl := item.itemLevel()) >= 82) {
                qNames[1] := item.baseName
                ilvl := (ilvl >= 86) ? 86 : ilvl
                if (ilvl > 82)
                    qNames[2] := qNames[1] " " (ilvl - 1)
                qNames[1] .= " " ilvl

                if (influence := item.getInfluenceType()) {
                    influenceNames := ""
                    loop, 6 {
                        if (influence & (1 << (A_Index - 1)))
                            influenceNames .= "/" this.influenceTypes[A_Index]
                    }
                    qNames[1] .= " " SubStr(influenceNames, 2)
                    qNames[2] .= " " SubStr(influenceNames, 2)
                }
            }
        }

        for i, name in qNames {
            if (value := this.prices[name].value)
                return value
        }
    }

    update(league) {
        Sleep, 3000
        if (league != this.league) {
            SetTimer,, Delete
            return
        }

        if (Not ptask.isReady) {
            SetTimer,, -60000
            return
        }

        try {
            for name, t in this.types {
                url := Format(this.url, t.catalog, league, t.type)
                parsed := JSON.__parse(ajax(url))
                rdebug("#PRICER", "<b style=""background-color:gold;color:black"">Loading prices of {} ... {}</b>", name, parsed.lines.length)
                parsed.lines.forEach(ObjBindMethod(this, "addPrice", t.type))
            }
        } catch {
            SetTimer,, -1000
            return
        }
        this.prices["Ritual Splinter"] := {"value" : this.prices["Ritual Vessel"].value / 100}

        FormatTime, t,, MM/dd/yyyy hh:mm:ss
        this.lastUpdateTime := t
        SetTimer,, % this.updatePeriod
        rdebug("#PRICER", "<b style=""background-color:gold;color:black"">Total {} prices loaded.</b>", this.prices.Count())
    }

    dump(exp, limit = 100) {
        for name, p in this.prices {
            if (name ~= "i)"exp) {
                debug(name ", " p.value)
                n += 1
                if (n >= limit)
                    break
            }
        }
    }

    __onAreaChanged() {
        if (ptask.league ~= "SSF")
            return

        if (ptask.league != this.league) {
            this.prices := {"Chaos Orb" : {"value" : 1}}
            this.league := ptask.league
            t := ObjBindMethod(this, "update", this.league)
            SetTimer, %t%, -1000
        }
    }
}
