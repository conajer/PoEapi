; 
; Character.ahk, 9/15/2020 10:38 AM
;

class Flask {

    __new(item) {
        if (RegExMatch(item.baseName, "Life|Hybrid")) {
            this.IsLife := true
            this.type := "<b style=""color:red"">L</b>"
        } else if (RegExMatch(item.baseName, "Mana")) {
            this.IsMana := true
            this.type := "<b style=""color:blue"">M</b>"
        } else {
            this.IsUtility := true
            this.type := "<b>U</b>"
            if (RegExMatch(item.baseName, "Quicksilver")) {
                this.IsQuickSilver := true
                this.type := "<b style=""color:seagreen"">Q</b>"
            }
        }

        item.getComponents()
        FlaskInfo := item.components["Flask"]
        ChargesInfo := item.components["Charges"]

        reduced := RegExMatch(this.Mods, "([0-9]+)% reduced Charges used", matched) ? matched1 : 0
        increased := RegExMatch(this.Mods, "([0-9]+)% increased Duration", matched) ? matched1 : 0

        this.maxCharges := ChargesInfo.maxCharges
        this.chargesPerUse := Floor(ChargesInfo.chargesPerUse * (100 - reduced) / 100)
        this.duration := Flaskinfo.duration * (100 + item.quality + increased)
        this.endTime := A_Tickcount
        this.key := item.left
        this.index := item.index
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
        OnMessage(WM_FLASK_CHANGED, ObjBindMethod(this, "flaskChanged"))

        this.flasks := {}
        this.nearbyMonsters := 0
        this.expectCharges := 0
    }

    flaskChanged() {
        flask_items := ptask.flasks.getItems()
        loop, 5 {
            i := (A_Index << 1) - 1
            if (flask_items[i]) {
                if (flask_items[i] != this.flasks[A_Index].item) {
                    oldFlask := this.flasks[A_Index]
                    this.flasks[A_Index] := new Flask(flask_items[i])
                    this.flasks[A_Index].endTime := oldFlask.endTime
                }
            } else {
                this.flasks.Delete(A_Index)
            }
        }

        flaskTypes := ""
        loop, 5 {
            if (this.flasks[A_Index])
                flaskTypes .= "[" this.flasks[A_Index].type "]"
            else
                flaskTypes .= "[]"
        }
        rdebug("#FLASKS", "Flasks: {}", flaskTypes)
    }

    areaChanged(areaName, lParam) {
        for i, item in ptask.inventories[12].items
            this.flaskChanged(index, item)
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
        if (Not this.isDead) {
            debug("DIED!!!")
            this.isDead := true
        }
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
        if (A_Tickcount - this.last_moving < 200)
            this.keep_moving += 1
        else
            this.keep_moving := 0
        this.last_moving := A_Tickcount

        if (this.keep_moving < 10 || ptask.hasBuff("flask_utility_sprint"))
            return

        if (AlwaysRunning || this.nearbyMonsters > MonsterThreshold) {
            for i, aFlask in this.flasks {
                maxCharges := 0
                if (aFlask.IsQuicksilver) {
                    charges := aFlask.item.charges()
                    if (charges > aFlask.chargesPerUse && charges > maxCharges) {
                        selected.endTime := A_Tickcount + 1000
                        selected := aFlask
                        maxCharges := charges
                    } else {
                        aFlask.endTime := A_Tickcount + 1000
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
