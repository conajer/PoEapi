; 
; Character.ahk, 9/15/2020 10:38 AM
;

class Flask {

    __new(item) {
        if (RegExMatch(item.baseName, "Life|Hybrid"))
            this.IsLife := true
        else if (RegExMatch(item.baseName, "Mana"))
            this.IsMana := true
        else {
            this.IsUtility := true
            if (RegExMatch(item.baseName, "Quicksilver"))
                this.IsQuickSilver := true
        }

        ChargesInfo := item.getComponent("Charges")
        FlaskInfo := item.getComponent("Flask")

        reduced := RegExMatch(this.Mods, "([0-9]+)% reduced Charges used", matched) ? matched1 : 0
        increased := RegExMatch(this.Mods, "([0-9]+)% increased Duration", matched) ? matched1 : 0

        this.maxCharges := ChargesInfo.maxCharges
        this.chargesPerUse := Floor(ChargesInfo.chargesPerUse * (100 - reduced) / 100)
        this.duration := Flaskinfo.duration * (100 + item.quality + increased)
        this.endTime := A_Tickcount
        this.key := item.x + 1
        this.item := item
    }

    use(chargesLimit = 0, forceToUse = false) {
        charges := this.item.charges()
        if (charges < this.chargesPerUse)
            return false

        if (Not chargesLimit)
            chargesLimit := ChargesPerUseLimit

        if (this.IsUtility) {
            if (forceToUse || (this.chargesPerUse < chargesLimit && this.endTime <= A_Tickcount)) {
                SendInput, % this.key
                this.endTime := A_Tickcount + this.duration
                return true
            }
        } else if (this.IsLife) {
            if (Life < 80 || this.savedLife > Life || this.endTime <= A_Tickcount) {
                SendInput, % this.key
                this.endTime := A_Tickcount + 2000
                this.savedLife := Life
                return true
            }
        } else {
            if (this.endTime <= A_Tickcount) {
                SendInput, % this.key
                this.endTime := A_Tickcount + 4000
                return true
            }
        }

        return false
    }
}

class Character {

    __new() {
        OnMessage(WM_PLAYER_LIFE, ObjBindMethod(this, "lifeChanged"))
        OnMessage(WM_PLAYER_MANA, ObjBindMethod(this, "manaChanged"))
        OnMessage(WM_PLAYER_DIED, ObjBindMethod(this, "playerDied"))
        OnMessage(WM_MOVE, ObjBindMethod(this, "onMove"))
        OnMessage(WM_AREA_CHANGED, ObjBindMethod(this, "areaChanged"))
        OnMessage(WM_USE_SKILL, ObjBindMethod(this, "onUseSkill"))
        OnMessage(WM_MONSTER_CHANGED, ObjBindMethod(this, "monsterChanged"))
        OnMessage(WM_MINION_CHANGED, ObjBindMethod(this, "minionChanged"))

        this.nearbyMonsters := 0
        this.expectCharges := 0
    }

    areaChanged(areaName, lParam) {
        this.flasks := {}
        flaskSlot := ptask.Inventories[12]
        flaskSlot.getItems()
        for index, item in flaskSlot.Items
            this.flasks.Push(new Flask(item))

        for i, aFlask in this.flasks
            debug("Flask {}: {}, {}, {}", aFlask.key, aFlask.item.baseName, aFlask.chargesPerUse, aFlask.maxCharges)
    }

    lifeChanged(life, lParam) {
        maximum := lParam & 0xffff
        reserved := lParam >> 16
        life := Round(life * 100 / (maximum - reserved))
        if (life < LifeThreshold) {
            maxUses := 0
            for i, aFlask in this.flasks {
                if (aFlask.IsLife) {
                    charges := aFlask.item.charges()
                    if (charges / aFlask.chargesPerUse > maxUses) {
                        selected := aFlask
                        maxUses := charges / aFlask.chargesPerUse
                    }
                }
            }
            selected.use()
        }
        this.life := life

        if(ptask.player.level > 90 && life < AutoQuitThreshold)
            ptask.logout()
    }

    manaChanged(mana, lParam) {
        maximum := lParam & 0xffff
        reserved := lParam >> 16
        mana := Round(mana * 100 / (maximum - reserved))

        if (mana < ManaThreshold) {
            maxUses := 0
            for i, aFlask in this.flasks {
                if (aFlask.IsMana) {
                    charges := aFlask.item.charges()
                    if (charges / aFlask.chargesPerUse > maxUses) {
                        selected := aFlask
                        maxUses := charges / aFlask.chargesPerUse
                    }
                }
            }
            selected.use()
        }
    }

    playerDied() {
        debug("DIED!!!")
    }

    monsterChanged(numOfMonsters, charges) {
        this.nearbyMonsters := numOfMonsters
        this.expectCharges := charges
        rdebug("#MONSTERS", "<b>{}</b> monsters, <b>{}</b> charges", numOfMonsters, charges)
    }

    minionChanged(numOfMinions) {
        this.nearbyMinions := numOfMinions
        rdebug("#MINIONS", "<b>{}</b> minions", numOfMinions)
    }

    onMove() {
        if (ptask.hasBuff("flask_bonus_movement_speed"))
            return

        if (AlwaysRunning || this.nearbyMonsters > MonsterThreshold) {
            for i, aFlask in this.flasks {
                maxCharges := 0
                if (aFlask.IsQuicksilver) {
                    charges := aFlask.item.charges()
                    if (charges > maxCharges) {
                        selected := aFlask
                        maxCharges := charges
                    }
                }
            }
            selected.use()
        }
    }

    onUseSkill() {
    }

    onAttack() {
        if (this.nearbyMonsters >= MonsterThreshold) {
            for i, aFlask in this.flasks {
                if (aFlask.IsUtility && Not aFlask.IsQuicksilver)
                    aFlask.use(this.expectCharges, this.life < LifeThreshold)
            }
        }
    }
}
