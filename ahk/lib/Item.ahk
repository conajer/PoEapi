;
; Item.ahk, 9/15/2020 2:08 PM
;

class Item extends Entity {

    __Get(key) {
        if (RegExMatch(key, "Is.*")) {
            if (RegExMatch(this.path, "Metadata/Items/([^/]*)s/(([^/]*)s)*", matched)) {
                this.BaseType := matched1
                if (matched1 == "Weapon")
                    this.GripType := (matched3 == "OneHandWeapon") ? "1H" : "2H"
                else if (matched1 == "Armour")
                    this.SubType := matched3
            } else if (RegExMatch(this.path, "Metadata/Items/([^/]*)/([^/]*)", matched)) {
                this.BaseType := matched1
                if (RegExMatch(matched2, "Prophecy"))
                    this.SubType := "Prophecy"
            }

            this["Is" this.baseType] := true
            this["Is" this.subType] := true
        }
    }
}
