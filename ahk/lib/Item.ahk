;
; Item.ahk, 9/15/2020 2:08 PM
;

class Item extends Entity {

    __Get(key) {
        if (RegExMatch(key, "i)baseType|subType|Is.*")) {
            if (RegExMatch(this.path, "Metadata/Items/([^/]*)s/(([^/]*)s)*", matched)) {
                baseType := matched1
                if (matched1 == "Weapon")
                    this.GripType := (matched3 == "OneHandWeapon") ? "1H" : "2H"
                else if (matched1 == "Armour")
                    subType := matched3
            } else if (RegExMatch(this.path, "Metadata/Items/([^/]*)/([^/]*)", matched)) {
                baseType := matched1
                if (RegExMatch(matched2, "Prophecy"))
                    subType := "Prophecy"
            }

            this["BaseType"] := baseType
            this["Is" baseType] := true
            if (subType) {
                this["SubType"] := subType
                this["Is" subType] := true
            }
        }
    }
}
