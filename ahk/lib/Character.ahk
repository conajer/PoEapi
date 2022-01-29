; 
; Character.ahk, 9/15/2020 10:38 AM
;

class Flask {

    __new(item) {
        if (RegExMatch(item.path, "Life")) {
            this.IsLife := true
            this.type := "<b style=""color:red"">L</b>"
        } else if (RegExMatch(item.path, "Mana")) {
            this.IsMana := true
            this.type := "<b style=""color:blue"">M</b>"
        } else if (RegExMatch(item.path, "Hybrid")) {
            this.IsLife := this.IsMana := true
            this.type := "<b style=""color:magenta"">H</b>"
        } else {
            this.IsUtility := true
            this.type := "<b>U</b>"
            if (RegExMatch(item.path, "Utility6")) {
                this.IsQuickSilver := true
                this.type := "<b style=""color:seagreen"">Q</b>"
            }
        }

        item.getComponents()
        FlaskInfo := item.components["Flask"]
        ChargesInfo := item.components["Charges"]

        increased := reduced := 0
        if (RegExMatch(item.name, "Catalysed", matched))
            increased += 50
        else if (RegExMatch(item.name, "Bubbling", matched))
            increased += 135
        else if (RegExMatch(item.name, "Experimenter's", matched))
            increased += 40
        else if (RegExMatch(item.name, "Alchemist's", matched))
            increased -= 33
        else if (RegExMatch(item.name, "Chemist's", matched))
            reduced += 25
        else if (RegExMatch(item.name, "Dying Sun", matched))
            increased -= 50
        else if (RegExMatch(item.name, "Seething", matched))
            this.duration := 0

        this.maxCharges := ChargesInfo.maxCharges
        this.chargesPerUse := Floor(ChargesInfo.chargesPerUse * (100 - reduced) / 100)
        if (this.IsUtility)
            this.duration := Flaskinfo.duration * (100 + item.quality + increased)
        else
            this.duration := Floor(Flaskinfo.duration * 10000 / (100 + increased))
        this.endTime := A_Tickcount
        this.key := item.left
        this.item := item
    }

    use(chargesLimit = 0, forceToUse = false) {
        charges := this.item.charges()
        if (charges < this.chargesPerUse)
            return false

        if (Not chargesLimit)
            chargesLimit := ChargesPerUseLimit

        if (this.IsUtility) {
            if (forceToUse || (this.chargesPerUse <= chargesLimit && this.endTime <= A_Tickcount)) {
                SendInput, % this.key
                this.endTime := A_Tickcount + this.duration
                return true
            }
        } else if (this.IsLife) {
            if (Life < 80 && (Life < 50 || this.savedLife > Life || this.endTime < A_Tickcount)) {
                SendInput, % this.key
                this.endTime := A_Tickcount + this.duration
                this.savedLife := Life
                return true
            }
        } else {
            if (this.endTime <= A_Tickcount) {
                SendInput, % this.key
                this.endTime := A_Tickcount + this.duration
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

    whois() {
        return Format(_("{1} is a level {2} {3} in the {4} league")
                     , this.name, this.level, this.className, ptask.League)
    }

    flaskChanged() {
        flask_items := ptask.inventories[12].getItems()
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
        rdebug("#FLASKS", _("Flasks") ": {}", flaskTypes)
    }

    areaChanged(areaName, lParam) {
        areaName := StrGet(areaName)
        this.inAzuriteMine := (areaName ~= _("Azurite Mine"))
    }

    lifeChanged(life, lParam) {
        maximum := lParam & 0xffff
        reserved := lParam >> 16
        life := Round(life * 100 / (maximum - reserved))

        if (ptask.isActive && ptask.InMap && life < 100) {
            if (this.nearbyMonsters > MonsterThreshold)
                SendInput, %DefenseBuffSkillKey%

            if (AutoDropFlare) {
                darkness := ptask.hasBuff("delve_degen_buff")
                if ((darkness > MaxDarknessStacks) || (life < 80 && darkness > 0)) {
                    SendInput, %DropFlareKey%
                    Sleep, 100
                }
            }
        }

        if (life < LifeThreshold && A_TickCount > this.lifeFlaskTime + 1000) {
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
            if (selected) {
                selected.use()
                this.lifeFlaskTime := A_Tickcount
            }
        }
        this.life := life
    }

    manaChanged(mana, lParam) {
        maximum := lParam & 0xffff
        reserved := lParam >> 16
        mana := Round(mana * 100 / (maximum - reserved))

        if (mana < ManaThreshold && A_TickCount > this.manaFlaskTime + 1000) {
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
            if (selected) {
                selected.use()
                this.manaFlaskTime := A_Tickcount
            }
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
        rdebug("#MONSTERS", "<b>{}</b> " _("monsters") ", <b>{}</b> " _("charges"), numOfMonsters, charges)
    }

    minionChanged(numOfMinions) {
        this.nearbyMinions := numOfMinions
        rdebug("#MINIONS", "<b>{}</b> " _("minions"), numOfMinions)
    }

    onMove() {
        if (A_Tickcount - this.lastMoving < 300)
            this.keepMoving += 1
        else
            this.keepMoving := 0
        this.lastMoving := A_Tickcount

        if (this.keepMoving < 5 || ptask.hasBuff("flask_utility_sprint"))
            return

        if (AlwaysRunning || this.nearbyMonsters > MonsterThreshold) {
            for i, aFlask in this.flasks {
                maxCharges := 0
                if (aFlask.IsQuicksilver) {
                    charges := aFlask.item.charges()
                    if (charges >= aFlask.chargesPerUse && charges > maxCharges) {
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

    onUseSkill(skill, target) {
        ;debug("{}, {:x}", StrGet(skill), target)
    }

    onAttack() {
        if (this.nearbyMonsters > 5)
            SendInput, %DefenseBuffSkillKey%

        ;if (this.life > 95 && Not ptask.hasBuff("blood_rage"))
        ;    SendInput, {MButton}

        if (this.nearbyMonsters >= MonsterThreshold) {
            for i, aFlask in this.flasks {
                if (aFlask.IsUtility)
                    aFlask.use(0, this.life < LifeThreshold)
            }
        }
    }
}
