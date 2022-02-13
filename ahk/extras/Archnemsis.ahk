;
; Archnemsis.ahk, 2/13/2023 2:36 PM
;

class Archnemesis {
    static tiers := { "Arcane Buffer":      1
                    , "Berserker":          1
                    , "Bloodletter":        1
                    , "Bombardier":         1
                    , "Bonebreaker":        1
                    , "Chaosweaver":        1
                    , "Consecrator":        1
                    , "Deadeye":            1
                    , "Dynamo":             1
                    , "Echoist":            1
                    , "Flameweaver":        1
                    , "Frenzied":           1
                    , "Frostweaver":        1
                    , "Gargantuan":         1
                    , "Hasted":             1
                    , "Incendiary":         1
                    , "Juggernaut":         1
                    , "Malediction":        1
                    , "Opulent":            1
                    , "Overcharged":        1
                    , "Permafrost":         1
                    , "Sentinel":           1
                    , "Soul Conduit":       1
                    , "Steel-Infused":      1
                    , "Stormweaver":        1
                    , "Toxic":              1
                    , "Vampiric":           1
                    , "Rejuvenating":       2
                    , "Corrupter":          2
                    , "Assassin":           2
                    , "Hexer":              2
                    , "Entangler":          2
                    , "Necromancer":        2
                    , "Drought Bringer":    2
                    , "Hexer":              2
                    , "Evocationist":       2
                    , "Treant Horde":       2
                    , "Invulnerable":       2
                    , "Frost Strider":      2
                    , "Storm Strider":      2
                    , "Flame Strider":      2
                    , "Executioner":        2
                    , "Magma Barrier":      2
                    , "Ice Prison":         2
                    , "Heralding Minions":  2
                    , "Mana Siphoner":      2
                    , "Mirror Image":       2
                    , "Empowering Minions": 2
                    , "Effigy":             3
                    , "Crystal-Skinned":    3
                    , "Temporal Bubble":    3
                    , "Empowered Elements": 3
                    , "Trickster":          3
                    , "Soul Eater":         3
                    , "Tukohama-touched":   3
                    , "Corpse Detonator":   3
                    , "Abberath-touched":   3
                    , "Lunaris-touched":    4
                    , "Solaris-touched":    4
                    , "Arakaali-touched":   4
                    , "Innocence-touched":  4
                    , "Shakari-touched":    4
                    , "Kitava-touched":     4
                    , "Brine King-touched": 4 }

    isOpened() {
        return ptask.getIngameUI.getChild(47).isVisible()
    }

    list() {
        panel := ptask.getIngameUI().getChild(47, 3, 1, 1)
        for i, e in panel.getChilds() {
            RegExMatch(e.getObject(0x3c8).readString(0x230, 32), "[^\n\r]+", name)
            if (name) {
                tier := this.tiers[name]
                debug("{:2d}. {} {}", i, tier, name)
            }
        }
    }

    showTiers() {
        colors := ["blue", "gold", "red", "purple"]
        panel := ptask.getIngameUI().getChild(47, 3, 1, 1)
        for i, e in panel.getChilds() {
            RegExMatch(e.getObject(0x3c8).readString(0x230, 32), "[^\n\r]+", name)
            if (name) {
                r := e.getRect()
                tier := this.tiers[name]
                ptask.c.drawText(tier, r.r, r.b, "white", colors[tier], -1, -1)
            }
        }
    }
}
