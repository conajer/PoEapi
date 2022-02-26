;
; Archnemsis.ahk, 2/13/2023 2:36 PM
;

ArchnemesisMods_json = 
(%
{
    "Toxic":              [1, "Monster has augmented Poison powers", "Generic Gems"],
    "Chaosweaver":        [1, "Monster is imbued with Chaos Damage and Resistance", "Gems"],
    "Frostweaver":        [1, "Monster is imbued with Cold Damage and Resistance", "Armour"],
    "Permafrost":         [1, "Monster has augmented Freeze and Chill powers", "Generic Armour"],
    "Hasted":             [1, "Monster is faster", "Generic"],
    "Deadeye":            [1, "Monster is Accurate and applies Marks", "Armour Trinkets"],
    "Bombardier":         [1, "Monster has augmented Projectile powers and periodically unleashes a barrage of mortars", "Weapon Armour"],
    "Flameweaver":        [1, "Monster is imbued with Fire Damage and Resistance", "Weapon"],
    "Incendiary":         [1, "Monster has augmented Ignite powers", "Generic Weapon"],
    "Arcane Buffer":      [1, "Monster has augmented Energy Shield powers and releases a stunning nova when Energy Shield breaks", "Essences"],
    "Echoist":            [1, "Monster repeats skills additional times", "Generic Currency"],
    "Stormweaver":        [1, "Monster is imbued with Lightning Damage and Resistance", "Trinkets"],
    "Dynamo":             [1, "Monster has augmented Shock powers", "Generic Trinkets"],
    "Bonebreaker":        [1, "Monster has augmented Stun powers. Monster hits are slower and can't be evaded", "Generic Weapon"],
    "Bloodletter":        [1, "Monster has augmented Bleed powers. Monster Maims nearby enemies", "Weapon Trinkets", "Items dropped from the Monster and its Minions are Corrupted"],
    "Steel-infused":      [1, "Monster is imbued with Physical Damage and Physical Damage Reduction", "Weapon"],
    "Gargantuan":         [1, "Monster is massive, granting more Life, Area of Effect, and Damage", "Currency"],
    "Berserker":          [1, "Monster Enrages as it loses life", "Uniques"],
    "Sentinel":           [1, "Monster is imbued with Block and Spell Block. Monster Triggers Reckoning when hit", "Armour Armour"],
    "Juggernaut":         [1, "Monster cannot be Slowed or Stunned. Monster gains Endurance Charges", "Harbinger"],
    "Vampiric":           [1, "Monster has augmented Life Leech powers", "Fossils"],
    "Overcharged":        [1, "Monster grants Charges to itself and Allies over time", "2 x Trinkets"],
    "Soul Conduit":       [1, "On death, Monster and nearby Allies are revived", "Maps"],
    "Opulent":            [1, "Monster is fabulously wealthy"],
    "Malediction":        [1, "Monster has a weakening Aura", "DivinationCards"],
    "Consecrator":        [1, "Monster periodically creates Consecrated Ground, healing allies and making them immune to curses and ailments", "Fragments"],
    "Frenzied":           [1, "Monster and allies periodically Enrage", "Generic Uniques"],

    "Heralding Minions":  [2, "Monster's minions summon invulnerable Lightning Totems on death", "2 x Fragments"],
    "Assassin":           [2, "Monster has augmented Critical powers and has Shroud Walker", "2 x Currency"],
    "Necromancer":        [2, "Monster can raise Undead. Minions are empowered and revive shortly after dying", "Generic", "Rewards are rolled 2 additional times, choosing the rarest result"],
    "Rejuvenating":       [2, "Monster has Life Regeneration. Periodically releases a wave that heals nearby allies and prevents enemy Life and Energy Shield recovery", "Currency", "Rewards are rolled 1 additional time, choosing the rarest result"],
    "Executioner":        [2, "Monster gains Damage based on missing enemy Life. Monster has an aura that prevents enemies recovering Life and Energy Shield above 50%", "Legion Breach"],
    "Hexer":              [2, "Monster is Hexproof. Monster is followed by a Hexing Effigy that creates Hexing areas", "2 x Essences"],
    "Drought Bringer":    [2, "Monster disables Flask effects on hit. Monster has an aura that drains Flask charges and prevents gaining Flask charges", "2 x Labyrinth"],
    "Entangler":          [2, "Monster creates Thorned Vines that slow and deal Chaos Damage over time.", "2 x Fossils"],
    "Frost Strider":      [2, "Monster leaves Chilling Ground in its wake. Minions create Frost Bearers on death", "3 x Armour"],
    "Ice Prison":         [2, "Monster periodically entraps players in a cage of ice", "2 x Armour", "Rewards are rolled 1 additional time, choosing the rarest result"],
    "Flame Strider":      [2, "Monster leaves Burning Ground in its wake. Minions create Flame Bearers on death", "3 x Weapon"],
    "Corpse Detonator":   [2, "Monster detonates nearby Corpses. Monster periodically creates Corpses", "2 x DivinationCards"],
    "Magma Barrier":      [2, "Monster is protected by a Magmatic shield that explodes when depleted. Monster periodically creates Volatile Flamebloods", "2 x Weapon", "Rewards are rolled 1 additional time, choosing the rarest result"],
    "Mirror Image":       [2, "Monster can create illusions of itself", "Scarabs", "Rewards are rolled 2 additional times, choosing the rarest result"],
    "Storm Strider":      [2, "Monster leaves Shocking Ground in its wake and spawns Lightning Mirages when hit. Minions create Storm Bearers on death", "3 x Trinkets"],
    "Mana Siphoner":      [2, "Monster is surrounded by a ring that deals Lightning Damage over time as well as draining Mana", "2 x Trinkets", "Rewards are rolled 1 additional time, choosing the rarest result"],
    "Corrupter":          [2, "Monster inflicts Corrupted Blood on Hit and when Hit. Minions create Corrupted Bubbles on death", "2 x Abyss", "Items dropped from the Monster and its Minions are Corrupted"],

    "Empowering Minions": [3, "Monster's minions are empowered. Minions empower the Monster with additional modifiers on death", "Blight Ritual"],
    "Trickster":          [3, "Monster has damage avoidance. Monster periodically Flees", "Currency Uniques DivinationCards"],
    "Temporal Bubble":    [3, "Monster is protected by a Temporal Bubble that severely slows those in it and cannot be damaged by those outside of it", "Heist Expedition"],
    "Treant Horde":       [3, "Monster's minions are replaced with powerful Treants. Some Damage taken from the monster is split between the Treants", "Generic", "Monster's Minions drop a randomly-chosen Reward Type"],
    "Soul Eater":         [3, "Monster empowers when nearby allies die. Periodically summons Phantasms", "2 x Maps"],
    "Evocationist":       [3, "Monster is imbued with Elemental Damage, Resistances and Ailments", "Generic Weapon Armour Trinkets"],
    "Invulnerable":       [3, "Monster and its Minions periodically become immune to all Damage. Minions cannot die while Monster is alive", "Delirium Metamorphosis"],
    "Crystal-skinned":    [3, "Monster triggers the creation of Crystals when hit. Crystals explode when the Monster dies", "2 x Harbinger"],
    "Empowered Elements": [3, "Monster cycles between imbued damage of a particular type and immunity to all other damage types", "2 x Uniques", "Rewards are rolled 1 additional time, choosing the rarest result"],
    "Effigy":             [3, "Monster creates an Effigy of the player. Damage dealt to the Effigy is also reflected to the bonded player", "2 x DivinationCards", "Rewards are rolled 1 additional time, choosing the rarest result"],

    "Lunaris-touched":    [4, "Monster uses the abilities of Lunaris", "Uniques", "All Reward Types have an additional reward"],
    "Solaris-touched":    [4, "Monster uses the abilities of Solaris", "Scarabs", "All Reward Types have an additional reward"],
    "Arakaali-touched":   [4, "Monster uses the abilities of Arakaali", "DivinationCards", "All Reward Types are Divination Cards"],
    "Brine King-touched": [4, "Monster uses the abilities of the Brine King", "3 x Armour", "Rewards are rolled 6 additional times, choosing the rarest result"],
    "Tukohama-touched":   [4, "Monster uses the abilities of Tukohama", "2 x Weapon Fragments", "Rewards are rolled 4 additional times, choosing the rarest result"],
    "Abberath-touched":   [4, "Monster uses the abilities of Abberath", "2 x Trinkets Maps", "Rewards are rolled 4 additional times, choosing the rarest result"],
    "Shakari-touched":    [4, "Monster uses the abilities of Shakari", "Uniques", "All Reward Types are Uniques"],
    "Innocence-touched":  [4, "Monster uses the abilities of Innocence", "3 x Currency", "All Reward Types are Currency"],
    "Kitava-touched":     [4, "Monster uses the abilities of Kitava", "Generic", "Rewards are doubled"]
}
)

global archnemesis := new Archnemesis()

class Recipe {

    __new(name, parts) {
        this.name := name
        this.parts := parts
    }

    contain(mod) {
        for i, part in this.parts {
            if (mod == part)
                return true
        }
        return false
    }
}

class Archnemesis extends WebGui {

    static recipes := [ ["Rejuvenating",        ["Gargantuan", "Vampiric"]]
                      , ["Corrupter",           ["Bloodletter", "Chaosweaver"]]
                      , ["Assassin",            ["Deadeye", "Vampiric"]]
                      , ["Hexer",               ["Chaosweaver", "Echoist"]]
                      , ["Entangler",           ["Toxic", "Bloodletter"]]
                      , ["Necromancer",         ["Bombardier", "Overcharged"]]
                      , ["Drought Bringer",     ["Malediction", "Deadeye"]]
                      , ["Frost Strider",       ["Frostweaver", "Hasted"]]
                      , ["Storm Strider",       ["Stormweaver", "Hasted"]]
                      , ["Flame Strider",       ["Flameweaver", "Hasted"]]
                      , ["Executioner",         ["Frenzied", "Berserker"]]
                      , ["Magma Barrier",       ["Incendiary", "Bonebreaker"]]
                      , ["Ice Prison",          ["Permafrost", "Sentinel"]]
                      , ["Heralding Minions",   ["Dynamo", "Arcane Buffer"]]
                      , ["Mana Siphoner",       ["Consecrator", "Dynamo"]]
                      , ["Mirror Image",        ["Echoist", "Soul Conduit"]]    
                      , ["Evocationist",        ["Flameweaver", "Frostweaver", "Stormweaver"]]
                      , ["Treant Horde",        ["Toxic", "Sentinel", "Steel-Infused"]]
                      , ["Invulnerable",        ["Sentinel", "Juggernaut", "Consecrator"]]
                      , ["Empowering Minions",  ["Necromancer", "Executioner", "Gargantuan"]]
                      , ["Effigy",              ["Hexer", "Malediction", "Corrupter"]]
                      , ["Crystal-skinned",     ["Rejuvenating", "Permafrost", "Berserker"]]
                      , ["Temporal Bubble",     ["Juggernaut", "Hexer", "Arcane Buffer"]]
                      , ["Empowered Elements",  ["Evocationist", "Steel-infused", "Chaosweaver"]]
                      , ["Trickster",           ["Overcharged", "Assassin", "Echoist"]]
                      , ["Soul Eater",          ["Necromancer", "Soul Conduit", "Gargantuan"]]  
                      , ["Tukohama-touched",    ["Bonebreaker", "Executioner", "Magma Barrier"]]
                      , ["Corpse Detonator",    ["Necromancer", "Incendiary"]]
                      , ["Abberath-touched",    ["Flame Strider", "Frenzied", "Rejuvenating"]]
                      , ["Lunaris-touched",     ["Frost Strider", "Invulnerable", "Empowering Minions"]]
                      , ["Solaris-touched",     ["Invulnerable", "Magma Barrier", "Empowering Minions"]]
                      , ["Arakaali-touched",    ["Corpse Detonator", "Entangler", "Assassin"]]
                      , ["Innocence-touched",   ["Lunaris-touched", "Solaris-touched", "Mirror Image", "Mana Siphoner"]]
                      , ["Shakari-touched",     ["Entangler", "Soul Eater", "Drought Bringer"]]
                      , ["Kitava-touched",      ["Abberath-touched", "Tukohama-touched", "Corrupter", "Corpse Detonator"]]
                      , ["Brine King-touched",  ["Ice Prison", "Storm Strider", "Heralding Minions"]] ]

    static colors := ["#fefefe", "#8787fe", "gold", "#af5f1c"]

    __new() {
        base.__new(, "+AlwaysOnTop +Toolwindow -Caption -Resize")
        Gui, Color, 0
        WinSet, TransColor, 0

        this.document.write("
        (%
            <!DOCTYPE html>
            <html>
            <head>
                <style>
                    * { font-family: Fontin SmallCaps, Serif; font-size: 16px; }
                    body { background: black; border: 0; margin: 0; padding: 0; }
                    table { position: absolute; color: white; background: #1a1411; right: 50px; top: 75px; width: 800px; max-height: 250px; }
                    tr { background: #120e0a; padding: 5px; transition: .1s ease-in; } 
                    td { padding: 0.25rem 1rem; }
                    tr:hover { background: #483b28; }
                    .tooltip { position: relative; }
                    .tooltip .tooltiptext {
                        visibility: hidden;
                        width: 300px;
                        background-color: #0c0c0c;
                        text-align: center;
                        padding: 10px 5px 5px;
                        position: absolute;
                        z-index: 1;
                        bottom: 125%;
                        left: 50%;
                        margin-left: -150px;
                        border: 1 solid dargrey;
                        opacity: 0;
                        transition: opacity 0.3s;
                    }
                    .tooltip:hover .tooltiptext { visibility: visible; opacity: 1; }
                    .descrption { color: #8787fe; }
                    .reward { color: #aa9e82; }
                    .reward_buff { color: white; }
                    .tier1 { color: #fefefe; }
                    .tier2 { color: #8787fe; }
                    .tier3 { color: gold; }
                    .tier4 { color: #af5f1c; }
                    .highlight { padding: 2px 5px; border: 1px solid white; }
                    .disabled { color: darkgrey; font-size: .9em; }
                </style>
            </head>
            <body oncontextmenu='return false;'>
                <table id='recipe_combo'></table>
            </body>
            </html>
        )")

        global ArchnemesisMods_json
        this.Mods := JSON.parse(ArchnemesisMods_json)
    }

    add(modName, recipe, partName := "") {
        if (Not recipe)
            return

        stockedMods := this.getMods()
        tier := this.Mods[modName][1]
        relatedRecipes := Format("
        (
            <tr><td class='tier{} tooltip'>
                <b>{}</b>
                <span class='tooltiptext'>
                    <div class='descrption'>{}</div>
                    <div class='reward'>{}</div>
                    <div class='reward_buff'>{}</div>
                </span>
            </td><td>
        )", tier, modName, this.Mods[modName][2], this.Mods[modName][3], this.Mods[modName][4])
        for i, name in recipe {
            if (A_Index > 1)
                relatedRecipes .= " + "
            tier := this.Mods[_(name)][1]
            styleClass := stockedMods.HasKey(_(name)) ? "tier" tier : "disabled"
            if (name == partName)
                relatedRecipes .= Format("<span class='{} highlight'>{}</span>", styleClass, name)
            else
                relatedRecipes .= Format("<span class='{}'>{}</span>", styleClass, name)
        }
        this._("#recipe_combo").innerHTML .= relatedRecipes "</td></tr>"
    }

    isOpened() {
        return ptask.getIngameUI.getChild(47).isVisible()
    }

    getMods() {
        mods := {}
        panel := ptask.getIngameUI().getChild(47, 3, 1, 1)
        for i, e in panel.getChilds() {
            if (RegExMatch(e.getObject(0x3d0).readString(0x230, 32), "[^\n\r]+", name))
                mods[name] := mods.HasKey(name) ? ++mods[name] : 1
        }

        return mods
    }

    getRecipe(modName) {
        for i, recipe in this.recipes {
            if (recipe[1] == modName)
                return recipe[2]
        }
    }

    checkMod(modName) {
        for i, recipe in this.recipes {
            for i, partName in recipe[2] {
                if (modName == partName) {
                    this.add(recipe[1], recipe[2], modName)
                    this.checkMod(recipe[1])
                    break
                }
            }
        }
    }

    checkRecipe(recipe, mods) {
        for i, name in recipe {
            if (Not mods.HasKey(name))
                return false
        }

        return true
    }

    showRecipe(mod, recipe, mods) {
        result := ""
        for i, name in recipe {
            if (i > 1)
                result .= " + "
            tier := this.Mods[_(name)][1]
            color := mods.HasKey(name) ? this.colors[tier] : "darkgrey"
            result .= Format("<b style='color: {}'>{}</b>", color, name)
        }

        tier := this.Mods[_(mod)][1]
        debug("<b style='color: {}'>{}</b> = {}", this.colors[tier], mod, result)
    }

    listRecipes() {
        mods := this.getMods()
        for i, recipe in this.recipes {
            if (this.checkRecipe(recipe[2], mods)) {
                this.showRecipe(recipe[1], recipe[2], mods)
                mods.Push(recipe[1])
            }
        }
    }

    list() {
        debug("Archnemesis mods:")
        for name, n in this.getMods() {
            tier := this.Mods[_(name)][1]
            debug("    {:2d}. {} {} x {}", A_Index, tier, name, n)
            total += n
        }
        debug("Total <b>{}</b> mods.", total)
    }

    onclick(event) {
        if (Not this.isOpened()) {
            this.hide()
            return
        }

        modName := event.srcElement.innerText
        if (Not modName ~= "\+") {
            this._("#recipe_combo").innerHTML := ""
            this.add(modName, this.getRecipe(modName))
            this.checkMod(modName)
            this.bindAll("tr")

            ptask.activate()
            SendInput, ^{f}
            SendInput, %modName%{Enter}
        }
    }

    showTiers() {
        panel := ptask.getIngameUI().getChild(47, 3, 1, 1)
        for i, e in panel.getChilds() {
            if (RegExMatch(e.getObject(0x3d0).readString(0x230, 32), "[^\n\r]+", name)) {
                r := e.getRect()
                tier := this.Mods[_(name)][1]
                ptask.c.drawText(tier, r.r, r.b, "#0c0c0c", this.colors[tier], -1, -1)
            }
        }
    }

    show() {
        if (Not this.isVisible) {
            base.show("NoActivate")
            r := ptask.getWindowRect()
            WinMove, % "ahk_id " this.Hwnd,, r.l, r.t, r.w, r.h
            this.isVisible := true
        }

        e := ptask.getHoveredElement()
        if (RegExMatch(e.getObject(0x3d0).readString(0x230, 32), "[^\n\r]+", name)) {
            if (name != this.selected) {
                this._("#recipe_combo").innerHTML := ""
                this.add(name, this.getRecipe(name))
                this.checkMod(name)
                this.bindAll("tr")
                this.selected := name
            }
        } else {
            this._("#recipe_combo").innerHTML := ""
            this.showTiers()
        }
    }

    hide() {
        this.selected := ""
        this._("#recipe_combo").innerHTML := ""
        this.isVisible := false
        base.hide()
    }
}
