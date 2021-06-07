;
; Item.ahk, 9/15/2020 2:08 PM
;

class Item extends Entity {

    fullName() {
        qName := this.name
        if (this.IsIdentified && this.rarity > 1)
            qName .= " " this.baseName
        if (this.IsMap)
            qName .= " (T" this.tier ")"
        if (this.IsGem)
            qName .= " Level " this.level " " this.quality "%" 

        return qName
    }

    __Get(key) {
        if (RegExMatch(key, "i)baseType|subType|is.*")) {
            if (RegExMatch(this.path, "Metadata/Items/(([^/]+)s)/((Gloves|Boots|[^/]+[^/s])s?)*", matched)) {
                baseType := matched2
                if (matched2 == "Weapon")
                    this.gripType := (matched4 == "OneHandWeapon") ? "1H" : "2H"
                else if (matched2 == "Armour")
                    subType := matched4
                else if (matched1 == "AtlasExiles")
                    baseType := "Currency"
                else if (matched1 == "Metamorphosis")
                    baseType := "Metamorph"
            } else if (RegExMatch(this.path, "Metadata/Items/([^/]*)/([^/]*)", matched)) {
                baseType := matched1
                if (matched2 == "HeistCoin")
                    baseType := "Currency"
                else if (InStr(matched2, "Prophecy"))
                    baseType := "Prophecy"
                else if (InStr(matched2, "CapturedMonster"))
                    baseType := "Beast"
            }

            this["baseType"] := baseType
            this["is" baseType] := true
            if (subType) {
                this["subType"] := subType
                this["is" subType] := true
            }
        }
    }
}
