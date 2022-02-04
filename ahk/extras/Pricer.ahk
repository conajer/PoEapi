;
; Pricer.ahk, 1/14/2021 12:23 PM
;

global pricer := new Pricer()

class Pricer {

    url := "https://poe.ninja/api/data/{}overview?league={}&type={}&language={}"

    types := { "Currency"           : {"catalog" : "currency", "type" : "Currency"}
             , "Fragments"          : {"catalog" : "currency", "type" : "Fragment"}
             , "Divination Cards"   : {"catalog" : "item", "type" : "DivinationCard"}
             , "Artifacts  "        : {"catalog" : "item", "type" : "Artifact"}
             , "Prophecies"         : {"catalog" : "item", "type" : "Prophecy"}
             , "Oils"               : {"catalog" : "item", "type" : "Oil"}
             , "Incubators"         : {"catalog" : "item", "type" : "Incubator"}

             ; Equipment & Gems
             , "Unique Weapons"     : {"catalog" : "item", "type" : "UniqueWeapon"}
             , "Unique Armours"     : {"catalog" : "item", "type" : "UniqueArmour"}
             , "Unique Accessories" : {"catalog" : "item", "type" : "UniqueAccessory"}
             , "Unique Flasks"      : {"catalog" : "item", "type" : "UniqueFlask"}
             , "Unique Jewels"      : {"catalog" : "item", "type" : "UniqueJewel"}
             , "Skill Gems"         : {"catalog" : "item", "type" : "SkillGem"}

             ; Atlas
             , "Maps"               : {"catalog" : "item", "type" : "Map"}
             , "Blighted Maps"      : {"catalog" : "item", "type" : "BlightedMap"}
             , "Blight-ravaged Maps": {"catalog" : "item", "type" : "BlightRavagedMap"}
             , "Unique Maps"        : {"catalog" : "item", "type" : "UniqueMap"}
             , "Delirium Orbs"      : {"catalog" : "item", "type" : "DeliriumOrb"}
             , "Invitations"        : {"catalog" : "item", "type" : "Invitation"}
             , "Scarabs"            : {"catalog" : "item", "type" : "Scarab"}
             , "Watchstones"        : {"catalog" : "item", "type" : "Watchstone"}

             ; Crafting
             , "Base Types"         : {"catalog" : "item", "type" : "BaseType"}
             , "Fossils"            : {"catalog" : "item", "type" : "Fossil"}
             , "Resonators"         : {"catalog" : "item", "type" : "Resonator"}
             , "Beasts"             : {"catalog" : "item", "type" : "Beast"}
             , "Essences"           : {"catalog" : "item", "type" : "Essence"}
             , "Vials"              : {"catalog" : "item", "type" : "Vial"} }

    langNames := {"de": "ge", "pt-BR": "pt", "ko-KR": "ko"}
    influenceTypes := ["Shaper", "Elder", "Crusader", "Redeemer", "Hunter", "Warlord"]
    qualityTypes := {"Anomalous": 1, "Divergent": 2, "Phantasmal": 3}

    updatePeriod := 3600 * 1000

    __new() {
        db.exec("
            (
            CREATE TABLE IF NOT EXISTS items (
                id INTEGER PRIMARY KEY,
                literal_id INTEGER UNIQUE,
                base_type TEXT,
                item_type TEXT);
            
            CREATE VIEW IF NOT EXISTS v_items AS
                SELECT items.id, literals.text AS name, base_type, item_type
                FROM items INNER JOIN literals ON items.literal_id=literals.id;

            CREATE TABLE IF NOT EXISTS item_prices (
                id INTEGER REFERENCES items(id),
                details TEXT,
                price REAL,
                is_blighted DEFAULT 0,
                is_corrupted INTEGER DEFAULT 0,
                is_unique INTEGER DEFAULT 0,
                map_tier INTEGER,
                gem_level INTEGER,
                quality_type INTEGER DEFAULT 0,
                quality INTEGER,
                ilvl INTEGER,
                links INTEGER,
                variant TEXT,
                league TEXT NOT NULL);
            )")

        OnMessage(WM_AREA_CHANGED, ObjBindMethod(this, "__onAreaChanged"))
    }

    getPrice(item) {
        if (not IsObject(item))
            return this.__getPrice(item)[1].price

        if (item.isMap) {
            name := (item.rarity == 3) ? item.name : item.baseName
            result := this.__getPrice(name, "ORDER BY map_tier DESC")
            for i, r in result {
                if (r.map_tier == item.tier) {
                    if ((item.rarity == 3) ^ r.is_unique || item.isBlighted() ^ r.is_blighted)
                        continue
                    return r.price
                } 
            }
        } else if (item.isGem) {
            result := this.__getPrice(item.name, "ORDER BY gem_level DESC, quality DESC")
            for i, r in result {
                if ((item.isCorrupted() ^ r.is_corrupted) || (item.qualityType() != r.quality_type))
                    continue
                if (item.level >= r.gem_level && item.quality >= r.quality)
                    return r.price
            }
        } else if (item.isProphecy) {
            result := this.__getPrice(item.name)
            if (item.name == "A Master Seeks Help") {
                prophecy := item.getComponent("Prophecy")
                RegExMatch(prophecy.predictionText, "You will find (.*) and complete (his|her) mission.", matched)
                for i, r in result {
                    if (r.variant == matched1)
                        return r.price
                }
            }
            return result[1].price
        } else if (item.rarity == 3 && item.baseName ~= "Cluster Jewel") {
            result := this.__getPrice(item.name)
            mods := item.getMods()
            RegExMatch(mods[2], "ExpansionJewelEmptyPassiveUnique__?([0-9])", matched)
            for i, r in result {
                if (r.variant == (matched1 * 2 - 1) " passives")
                    return r.price
            }
            return result[1].price
        } else if (not item.isCurrency) {
            result := this.__getPrice((item.rarity == 3) ? item.name : item.baseName, "ORDER BY ilvl DESC")
            if (not result)
                return

            if (item.rarity < 3) {
                ilvl := (item.ilvl >= 86) ? 86 : item.ilvl
                if (ilvl && ilvl < 82)
                    return

                if (itype := item.getInfluenceType()) {
                    loop, 6 {
                        if (itype & (1 << (A_Index - 1))) {
                            if (influences) {
                                influences .= "/" _(this.influenceTypes[A_Index])
                                break
                            }
                            influences .= _(this.influenceTypes[A_Index])
                        }
                    }
                }
            }

            for i, r in result {
                if (r.links && r.links != item.links)
                    continue
                if (r.rarity < 3 && r.variant && r.variant != influences)
                    continue
                if (ilvl >= r.ilvl)
                    return r.price
            }
        } else {
            return this.__getPrice(item.name)[1].price
        }
    }

    findPrices(regexp, limit = 100) {
        return db.exec("
            (
            SELECT * FROM v_item_prices
                WHERE name LIKE ""%{}%"" AND league='{}'
                LIMIT {};
            )", regexp, this.league, limit)
    }

    addPrice(name, baseType, itemType, columns) {
        if (not id := this.__items[name]) {
            id := db.get("
                (
                INSERT INTO literals (text)
                VALUES (""{1}"") ON CONFLICT (text) DO NOTHING;

                INSERT INTO items (literal_id, base_type, item_type)
                VALUES ((SELECT id FROM literals WHERE text=""{1}""),
                        ""{2}"", '{3}')
                    ON CONFLICT (literal_id) DO UPDATE SET id=id
                RETURNING id;
                )", name, baseType, itemType)
            this.__items[name] := id
        }
        this.__items[_(name)] := this.__items[name]

        colNames := "id"
        values := id
        for name, value in columns
            colNames .= ", " name, values .= ", '" value "'"
        db.exec("INSERT INTO item_prices ({}, league) VALUES ({}, '{}');", colNames, values, this.league)
    }

    update(league) {
        if (league != ptask.league) {
            SetTimer,, Delete
            return
        }

        if (not ptask.isReady) {
            SetTimer,, -60000
            return
        }

        this.league := ptask.league
        if (not this.__items || language != this.lang) {
            this.__items := {}
            this.lang := language

            db.exec("
                (
                DROP VIEW IF EXISTS v_item_prices;
                CREATE VIEW v_item_prices AS
                    SELECT translations.text AS name, items.base_type, item_prices.*
                    FROM item_prices
                        INNER JOIN items USING (id)
                        INNER JOIN translations
                            ON translations.id=items.literal_id AND translations.language='{1}'
                    WHERE league='{2}'
                UNION
                    SELECT v_items.name, v_items.base_type, item_prices.*
                    FROM item_prices INNER JOIN v_items USING (id)
                    WHERE league='{2}';
                )"
                , this.lang, this.league)

            for i, r in db.exec("SELECT id, name FROM v_items;")
                this.__items[r.name] := this.__items[_(r.name)] := r.id
        }

        lang := db.load("pricer.language")
        lastUpdateTime := db.load("pricer.last_update_time")
        tPeriod -= lastUpdateTime, seconds
        tBegin := A_Tickcount
        total := db.get("SELECT count(*) AS total FROM item_prices WHERE league='{}';", this.league)
        if (total < 256 || not lastUpdateTime || (tPeriod >= this.updatePeriod / 1000) || lang != this.lang) {
            JSON.eval("
            (
                function parse(type, json, cb) {
                    result = JSON.parse(json);
                    dict = result.language.translations;
                    for (let i = 0; i < result.lines.length; ++i) {
                        alert(result.lines[i].name);
                    }
                }
            )")
            try {
                db.exec("BEGIN TRANSACTION")
                db.exec("DELETE FROM item_prices WHERE league='{}';", this.league)
                lang := this.langNames[this.lang] ? this.langNames[this.lang] : this.lang
                for name, t in this.types {
                    url := Format(this.url, t.catalog, this.league, t.type, lang)
                    parsed := JSON.__parse(ajax(url))
                    rdebug("#PRICER", "<b style='background-color:gold;color:black'>Loading item prices of {} ... {}</b>", name, parsed.lines.length)
                    dict := JSON.__copy(parsed.language.translations)
                    parsed.lines.forEach(ObjBindMethod(this, "__addPrice", t.type, dict))
                    if (t.type == "Currency" && dict["Chaos Orb"])
                        db.addTranslation("Chaos Orb", dict["Chaos Orb"])
                    if (t.type == "BaseType") {
                        for i, t in this.influenceTypes
                            db.addTranslation(t, dict[t])
                    }
                }
                this.addPrice("Chaos Orb", "", t.type, {"price": 1})
                this.__prices := {}
                db.store("pricer.language", this.lang)
                db.store("pricer.last_update_time", A_NOW)
                db.exec("END TRANSACTION")
            } catch {
                db.exec("ROLLBACK")
                SetTimer,, -60000
                return
            }
        }

        SetTimer,, % this.updatePeriod
        total := db.get("SELECT count(*) AS total FROM item_prices WHERE league='{}';", this.league)
        rdebug("#PRICER", "<b style='background-color:gold;color:black'>Total {} item prices loaded (in {} ms).</b>", total, A_Tickcount - tBegin)
    }

    __addPrice(type, dict, p) {
        if (p.hasOwnProperty("currencyTypeName")) {
            dict.hasKey(p.currencyTypeName)
                ? db.addTranslation(p.currencyTypeName, dict[p.currencyTypeName])
            p.hasOwnProperty("receive")
                ? this.addPrice(p.currencyTypeName, "", type, {"price": p.receive.value})
                : this.addPrice(p.currencyTypeName, "", type, {"price": p.chaosEquivalent})
        } else {
            if (p.sparkline.data.length == 0 || (p.itemClass != 6 && InStr(p.detailsId, "-relic")))
                return

            dict.hasKey(p.name) ? db.addTranslation(p.name, dict[p.name]) : ""
            , cols := {"price": p.chaosValue}
            , InStr(type, "Unique") ? cols.is_unique := true : 0
            , p.hasOwnProperty("variant") ? cols.variant := p.variant : ""
            switch (type) {
            case "BaseType":
                cols.ilvl := p.levelRequired
                , cols.details := "ilvl " cols.ilvl
            case "Map":
                cols.map_tier := p.mapTier
                , cols.details := "T" cols.map_tier
            case "BlightedMap":
                type := "Map"
                , p.name := RegExReplace(p.name, "Blighted ")
                , cols.map_tier := p.mapTier
                , cols.is_blighted := true
                , cols.details := "T" cols.map_tier
            case "UniqueMap":
                type := "Map"
                , cols.map_tier := p.mapTier
                , this.addPrice(p.baseType, p.baseType, type, cols)
            case "SkillGem":
                type := "Gem"
                , cols.gem_level := p.gemLevel
                , cols.quality := p.hasOwnProperty("gemQuality") ? p.gemQuality : 0
                , cols.details := "Level " cols.gem_level " " cols.quality "%"
                , cols.is_corrupted := p.hasOwnProperty("corrupted") ? true : false
                if (RegExMatch(p.name, "(Anomalous|Divergent|Phantasmal) (.*)", matched))
                    p.name := matched2
                    , cols.quality_type := this.qualityTypes[matched1]
                    , cols.variant := matched1
            case "UniqueWeapon":
                if (p.hasOwnProperty("links"))
                    cols.links := p.links
                    , cols.details := cols.links "L"
            case "UniqueArmour":
                if (p.hasOwnProperty("links"))
                    cols.links := p.links
                    , cols.details := cols.links "L"
            }

            p.hasOwnProperty("itemType") ? type := p.itemType : ""
            , p.hasOwnProperty("baseType") ? baseType := p.baseType : ""
            , this.addPrice(p.name, baseType, type, cols)
        }
    }

    __getPrice(name, orderBy = "") {
        if (not result := this.__prices[name]) {
            if (id := this.__items[name]) {
                result := db.exec("SELECT * FROM item_prices WHERE id={} AND league='{}' {}"
                                 , id, this.league, orderBy)
                this.__prices[name] := result
            }
        }

        return result
    }

    __onAreaChanged() {
        if (ptask.league ~= "SSF")
            return

        if (ptask.league != this.league || language != this.lang) {
            t := ObjBindMethod(this, "update", ptask.league)
            SetTimer, %t%, -1000
        }
    }
}
