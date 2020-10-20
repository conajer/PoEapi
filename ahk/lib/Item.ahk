;
; Item.ahk, 9/15/2020 2:08 PM
;

class Item extends Entity {

    __Get(key) {
        if (RegExMatch(key, "i)baseType|subType|is.*")) {
            if (RegExMatch(this.path, "Metadata/Items/(([^/]*)s)/(([^/]*)s)*", matched)) {
                baseType := matched2
                if (matched2 == "Weapon")
                    this.GripType := (matched4 == "OneHandWeapon") ? "1H" : "2H"
                else if (matched2 == "Armour")
                    subType := matched4
                else if (matched1 == "Metamorphosis")
                    baseType := "Metamorph"
            } else if (RegExMatch(this.path, "Metadata/Items/([^/]*)/([^/]*)", matched)) {
                baseType := InStr(matched2, "Prophecy") ? "Prophecy" : matched1
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
