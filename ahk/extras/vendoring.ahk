;
; vendoring.ahk, 9/29/2020 10:37 PM
;

class FullRareSets {

    types := ["Weapon", "Helmet", "BodyArmour", "Gloves", "Boots", "Belt", "Amulet", "Ring"]

    __new() {
        this.items := {}
        for i, type in this.types
            this.items[type] := []
    }

    add(item) {
        if (item.rarity != 2 || item.isQuiver)
            return

        if (item.isWeapon || item.isShield) {
            if (item.gripType == "2H")
                this.items["Weapon"].Push(item)
            else
                this.items["Weapon"].InsertAt(1, item)
        } else {
            type := item.isArmour ? item.subType : item.baseType
            if (item.itemLevel < 75)
                this.items[type].Push(item)
            else
                this.items[type].InsertAt(1, item)
        }
    }

    get() {
        rareItems := []
        isChaosRecipe := false
        for type, items in this.items {
            n := (type ~= "Weapon|Ring") ? 2 : 1
            loop, % n {
                if (Not isChaosRecipe) {
                    item := items.Pop()
                    isChaosRecipe := item.itemLevel < 75
                } else {
                    item := items.RemoveAt(1)
                }

                if (Not item) {
                    debug("!!! {} <b style=""color=red"">{}</b>.", _("Need more"), type)
                    return
                }

                rareItems.Push(item)
                if (item.gripType == "2H")
                    break
            }
        }

        return rareItems
    }
}

addVendorButton() {
    Menu, __vendoringMenu, Add, % _("Trade quality gems"), tradeGems
    Menu, __vendoringMenu, Add, % _("Trade divination cards"), tradeDivinationCards
    Menu, __vendoringMenu, Add, % _("Trade full rare sets"), tradeFullRareSets
    Menu, __vendoringMenu, Add
    Menu, __vendoringMenu, Add, % _("Unstack divination cards"), unstackCards
    Menu, __vendoringMenu, Add
    Menu, __vendoringMenu, Add, % _("Dump inventory items"), dumpInventoryItems
    Menu, __vendoringMenu, Add, % _("Dump stash tab items"), dumpStashTabItems

    Gui, Add, Button, x+2 y0 gpopupVendorCommands, % _("Vendoring")

    Hotkey, IfWinActive, ahk_class POEWindowClass
    Hotkey, F6, dumpInventoryItems
    Hotkey, ^F6, dumpStashTabItems
    Hotkey, F7, tradeFullRareSets
    Hotkey, IfWinActive
}

popupVendorCommands() {
    Menu, __vendoringMenu, Show
}

tradeGems() {
    ptask.activate()
    if (Not ptask.stash.open())
        return

    ptask.stash.switchTab(VendorTabGems)
    tab := ptask.stash.Tab
    for i, item in tab.getItems() {
        if (item.isGem && item.level < 19 && item.quality > 0 && item.quality < 18)
            tab.move(item)
    }

    if (ptask.getVendor().sell()) {
        for i, item in ptask.inventory.getItems() {
            if (item.isGem && item.level < 19 && item.quality > 0)
                ptask.inventory.move(item)
        }
        ptask.getSell().accept()
    }
}

tradeDivinationCards() {
    ptask.activate()
    tab := ptask.stash.getTab(VendorTabDivinationCards)
    if (tab) {
        for i, item in tab.getItems() {
            if (item.isDivinationCard && item.stackCount == item.stackSize)
                n += 1
        }
    }

    if (Not tab || n > 0) {
        if (Not ptask.stash.open())
            return

        ptask.stash.switchTab(VendorTabDivinationCards)
        Sleep, 500

        for i, item in ptask.stash.Tab.getItems() {
            if (item.isDivinationCard && item.stackCount == item.stackSize)
                tab.move(item)
        }
    }

    if (ptask.getVendor().tradeDivinationCards()) {
        Sleep, 100
        ingameUI := ptask.getIngameUI()
        ingameUI.getChild(60, 5).getPos(x, y)
        for i, item in ptask.inventory.getItems() {
            if (item.isDivinationCard && item.stackCount == item.stackSize) {
                ptask.inventory.move(item)
                Sleep, 100
                MouseClick(x, y)
                Sleep, 200
                MouseMove, x, y - 150, 0
                Sleep, 50
                SendInput, ^{Click}
                Sleep, 100
            }
        }
    }
}

tradeFullRareSets() {
    ptask.activate()
    if (Not ptask.stash.open())
        return

    ptask.stash.switchTab(VendorTabFullRareSets)
    Sleep, 500

    rareSets := new FullRareSets()
    tab := ptask.stash.Tab
    for i, item in tab.getItems()
        rareSets.add(item)

    vendor := ptask.getVendor()
    sell := ptask.getSell()
    loop {
        rareItems := rareSets.get()
        if (Not rareItems || Not ptask.stash.open())
            break

        for i, item in rareItems
            tab.move(item)

        ptask.inventory.use(ptask.inventory.findItem(_("A Valuable Combination")))
        if (Not vendor.sell())
            break

        for i, item in ptask.inventory.getItems()
            if (item.rarity == 2 && (Not item.isIdentified) && item.itemLevel >= 60)
                ptask.inventory.move(item)

        Sleep, 100
        offerItem := sell.getItems()[1]
        if (Not RegExMatch(offerItem.name, _("Chaos") "|" _("Exalted Shard")))
            break

        debug(_("Received") " <b>{} {}</b>", offerItem.stackCount, offerItem.name)
        sell.accept(true)
        SendInput, %CloseAllUIKey%
    }
}

dumpInventoryItems() {
    ptask.activate()
    for i, item in ptask.inventory.getItems() {
        ptask.inventory.move(item)
    }
}

dumpStashTabItems() {
    ptask.activate()
    if (Not ptask.stash.open())
        return

    tab := ptask.stash.Tab
    loop, 2 {
        for i, item in tab.getChilds() {
            if (Not dumpAllItems || item.isHighlighted()) {
                item.getPos(x, y)
                MouseMove, x, y, 0
                Sleep, 30
                SendInput, ^{Click}
                Sleep, 30
                n += 1
            }
        }

        if (n > 0) break
        dumpAllItems := true
    }
}

unstackCards() {
    ptask.activate()
    if (Not ptask.stash.open())
        return

    tab := ptask.stash.Tab
    for i, item in tab.getItems() {
        if (item.name == _("Stacked Deck")) {
            loop, % item.stackCount {
                tab.moveTo(item.index)
                MouseClick, Right
                Sleep, 100

                if (Not ptask.inventory.drop())
                    return
            }
        }
    }
    ptask.stashItems()
}
