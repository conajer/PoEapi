;
; vendor.ahk, 9/29/2020 10:37 PM
;

addMenuItem("__vendor", _("Trade quality gems"), "tradeGems")
addMenuItem("__vendor", _("Trade divination cards"), "tradeDivinationCards")
addMenuItem("__vendor", _("Trade full rare sets"), "tradeFullRareSets")
addMenuItem("__vendor")
addMenuItem("__vendor", _("Open stacked decks"), "openStackedDecks")
addMenuItem("__vendor", _("Unstack Veiled Scarabs"), "unstackVeildedScarabs")
addMenuItem("__vendor")
addMenuItem("__vendor", _("Sort items"), "sortItems")
addMenuItem("__vendor", _("Dump useless items (< 1 Chaos)"), "dumpUselessItems")
addMenuItem("__vendor")
addMenuItem("__vendor", _("Dump inventory items"), "dumpInventoryItems")
addMenuItem("__vendor", _("Dump stash tab items"), "dumpStashTabItems")

Hotkey, IfWinActive, ahk_class POEWindowClass
Hotkey, F6, dumpInventoryItems
Hotkey, ^F6, dumpStashTabItems
Hotkey, F7, tradeFullRareSets
Hotkey, IfWinActive

addExtraMenu(_("Vendor"), ":__vendor")

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
                    MsgBox, % Format("{} ""{}"".", _("Need more"), type)
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

tradeGems() {
    ptask.activate()
    if (Not ptask.stash.open())
        return

    ptask.stash.switchTab(VendorTabGems)
    Sleep, 300
    tab := ptask.stash.Tab
    for i, item in tab.getItems() {
        if (item.isGem && item.price < 2) {
            if (item.level < 19 && item.quality > 0 && item.quality < 20) {
                tab.move(item)
                n += 1
            }
        }
    }

    if (n > 0 && ptask.getVendor().sell()) {
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
        Sleep, 300
        tab := ptask.stash.Tab
        for i, item in ptask.stash.Tab.getItems() {
            if (item.name == "Stack Deck")
                continue

            if (item.isDivinationCard && item.stackCount == item.stackSize)
                tab.move(item)
        }
    }

    for i, item in ptask.inventory.getItems() {
        if (item.isDivinationCard && item.stackCount == item.stackSize)
            n += 1
    }

    if (n > 0 && ptask.getVendor().tradeDivinationCards()) {
        Sleep, 200
        ptask.getIngameUI().getChildByName("card_trade").getChild(5).getPos(x, y)
        for i, item in ptask.inventory.getItems() {
            if (item.name == "Stack Deck")
                continue

            if (item.isDivinationCard && item.stackCount == item.stackSize) {
                ptask.inventory.move(item)
                Sleep, 100
                MouseClick(x, y)
                Sleep, 200
                item := ptask.inventories[28].getItemByIndex(1)
                if (Not item || Not ptask.inventory.nextCell(item.width, item.height))
                    return
                MouseMove, x, y - 150, 0
                Sleep, 50
                SendInput, {Ctrl down}
                Click
                SendInput, {Ctrl up}
                Sleep, 100
            }
        }
    }
}

tradeFullRareSets() {
    ptask.activate()
    if (Not ptask.stash.open())
        return

    rareSets := new FullRareSets()
    for i, tab in StrSplit(VendorTabFullRareSets, ",", " ") {
        ptask.stash.switchTab(tab)
        Sleep, 200
        for i, item in ptask.stash.Tab.getItems() {
            item.tab := tab
            rareSets.add(item)
        }
    }

    vendor := ptask.getVendor()
    loop {
        if (GetKeyState("Ctrl"))
            break

        rareItems := rareSets.get()
        if (Not rareItems || Not ptask.stash.open())
            break

        for i, item in rareItems {
            tab := ptask.stash.switchTab(item.tab)
            tab.move(item)
        }

        ptask.inventory.use(ptask.inventory.findItem(_("A Valuable Combination")))
        if (Not vendor.sell())
            break

        sell := ptask.getSell()
        for i, item in ptask.inventory.getItems()
            if (item.rarity == 2 && (Not item.isIdentified) && item.itemLevel >= 60)
                ptask.inventory.move(item)

        Sleep, 100
        offerItem := sell.getItems()[1]
        if (Not RegExMatch(offerItem.name, _("Chaos Orb") "|" _("Regal Orb") "|" _("Exalted Shard")))
            break

        debug(_("Received") " <b>{} {}</b>", offerItem.stackCount, offerItem.name)
        sell.accept(true)
        SendInput, %CloseAllUIKey%
    }
}

dumpInventoryItems() {
    ptask.activate()
    if (Not ptask.inventory.isOpened())
        return

    for i, item in ptask.inventory.getItems() {
        if (GetKeyState("Ctrl"))
            break
        ptask.inventory.move(item)
    }
}

dumpStashTabItems() {
    ptask.activate()
    if (Not ptask.stash.isOpened())
        return

    tab := ptask.stash.Tab
    if (Not items := tab.getHighlightedItems())
        items := tab.getItems()

    Sleep, 500
    for i, aItem in items {
        if (Not ptask.inventory.nextCell(aItem.width, aItem.height))
            break

        tab.moveTo(aItem.index)
        loop, {
            SendInput, {Ctrl down}
            SendInput, {Click}
            SendInput, {Ctrl up}
            Sleep, 100

            if (GetKeyState("Ctrl"))
                return

            if (Not ptask.getHoveredItem() || Not ptask.inventory.freeCells())
                break
        }
    }
}

openStackedDecks() {
    ptask.activate()

    inventory := ptask.inventory
    if (ptask.stash.isOpened()) {
        tab := ptask.stash.Tab
        if (tab.findItems(_("Stacked Deck")))
            inventory := tab
    }

    n := 0
    debug("Opening stacked decks:")
    for i, item in inventory.findItems(_("Stacked Deck")) {
        index := item.index
        stackCount := item.stackCount
        loop, {
            if (Not stackCount || Not ptask.inventory.freeCells())
                break

            inventory.moveTo(index)
            Sleep, 50
            stackCount := ptask.getHoveredItem().stackCount()
            SendInput, {Click Right}

            loop, 5 {
                Sleep, 100
                if (divinationCard := ptask.inventories[13].getItemByIndex(1))
                    break
            }
            if (Not divinationCard)
                continue

            stackCount--
            price := $(divinationCard)
            if (price > 5)
                debug("    {:2d}. <b style='color: red;'>{}, {:g}</b>", ++n, divinationCard.name, price)
            else if (price > 1)
                debug("    {:2d}. {}, {:g}", ++n, divinationCard.name, price)
            else
                debug("    {:2d}. <span style='color: grey;'>{}, {:g}</span>", ++n, divinationCard.name, price)

            toIndex := 0
            stackedCards := ptask.inventory.findItems(divinationCard.name)
            if (stackedCards.Length() > 0) {
                for i, item in stackedCards {
                    if (item.stackCount() < item.stackSize()) {
                        toIndex := item.index
                        break
                    }
                }
            }

            if (Not ptask.inventory.drop(toIndex) || GetKeyState("Ctrl"))
                break
            Sleep, 50
        }

        if (GetKeyState("Ctrl"))
            break
    }
    debug("Opened <b>{}</b> stacked decks.", n)
}

unstackVeildedScarabs() {
    ptask.activate()

    inventory := ptask.inventory
    if (ptask.stash.isOpened()) {
        tab := ptask.stash.Tab
        if (tab.findItems(_("Veiled Scarab")))
            inventory := tab
    }

    ptask.inventory.open()
    n := 0
    debug("Unstacking Veiled Scarabs:")
    for i, item in inventory.findItems(_("Veiled Scarab")) {
        index := item.index
        stackCount := item.stackCount
        loop, {
            if (Not stackCount || Not ptask.inventory.freeCells())
                break

            inventory.moveTo(index)
            Sleep, 50
            stackCount := ptask.getHoveredItem().stackCount()
            SendInput, {Click Right}

            loop, 5 {
                Sleep, 50
                if (scarab := ptask.inventories[13].getItemByIndex(1))
                    break
            }
            if (Not scarab)
                continue

            price := $(scarab)
            if (price > 5)
                debug("    {:2d}. <b style='color: red;'>{}, {:g}</b>", ++n, scarab.name, price)
            else if (price > 1)
                debug("    {:2d}. {}, {:g}", ++n, scarab.name, price)
            else
                debug("    {:2d}. <span style='color: grey;'>{}, {:g}</span>", ++n, scarab.name, price)

            toIndex := 0
            stackedScarabs := ptask.inventory.findItems(scarab.name)
            if (stackedScarabs.Length() > 0) {
                for i, item in stackedScarabs {
                    if (item.name == scarab.name && item.stackCount() < item.stackSize()) {
                        toIndex := item.index
                        break
                    }
                }
            }

            if (Not ptask.inventory.drop(toIndex) || GetKeyState("Ctrl"))
                break
            stackCount--
            Sleep, 50
        }

        if (GetKeyState("Ctrl"))
            break
    }
    debug("Unstacked <b>{}</b> Veilded Scarabs.", n)
}

dumpUselessItems() {
    ptask.activate()
    if (Not ptask.stash.open())
        return

    tab := ptask.stash.Tab
    for i, e in tab.getChilds() {
        if (e.item && Not e.item.isCurrency) {
            if (e.item.price && e.item.price < 1) {
                debug("{}, {}", e.item.name, e.item.price)
                e.getPos(x, y)
                MouseMove, x, y, 0
                Sleep, 30

                SendInput, ^{Click}
                Sleep, 50
                if (ptask.inventory.freeCells() == 0)
                    return
            }
        }
    }
}


sortItems() {
    ptask.activate()
    if (ptask.stash.isOpened()) {
        tab := ptask.stash.Tab
        if (tab.type > 2 && tab.type != 7)
            return
    } else if (ptask.inventory.isOpened()) {
        tab := ptask.inventory
    } else {
        return
    }
    
    vals := []
    items := tab.getItems()
    loop, % tab.rows * tab.cols {
        aItem := items[A_Index]
        if (aItem.width > 1 || aItem.height > 1)
            items.Delete(A_Index)
    }

    for i, aItem in items
        vals[aItem.Index] := aItem.price

    debug("Begin sorting items...")
    t0 := A_Tickcount

    loop % tab.rows * tab.cols {
        if (GetKeyState("Ctrl"))
            break

        offset := A_Index
        selected := A_Index
        loop % tab.rows * tab.cols - offset {
            if (Not items[A_Index + offset])
                continue

            if (Not items[selected]) {
                selected := A_Index + offset
                continue
            }

            if (vals[A_Index + offset] > vals[selected]
                || (vals[A_Index + offset] == vals[selected]
                    && items[A_Index + offset].baseName <= items[selected].baseName)) {
                selected := A_Index + offset
            }
        }

        if (A_Index == selected || items[selected].baseName == items[A_Index].baseName) {
            if (Not items[A_Index]) {
                if (itemPicked) {
                    tab.moveTo(A_Index)
                    SendInput, {Click}
                    Sleep, 50
                    ;debug("    Placed " A_Index ", " itemPicked.baseName)
                }
                break
            }

            if (Not itemPicked
                || valPicked < vals[A_Index]
                || (valPicked == vals[A_Index] && itemPicked.baseName >= items[A_Index].baseName))
                continue
        }

        if (Not itemPicked) {
            tab.moveTo(selected)
            SendInput, {Click}
            Sleep, 30
            ;debug("    Picked up " selected ", " items[selected].baseName)
            vals[selected] := ""
            items[selected] := ""
        } else if (valPicked < vals[selected] || (valPicked == vals[selected] && itemPicked.baseName > items[selected].baseName)) {
            tab.moveTo(selected)
            SendInput, {Click}
            Sleep, 30
            ;debug("    Swaped with " selected ", " items[selected].baseName)
            vals[selected] := valPicked
            items[selected] := itemPicked
        }

        tab.moveTo(A_Index)
        SendInput, {Click}
        Sleep, 50
        ;if (items[A_Index])
        ;    debug("    Replaced " A_Index ", " items[A_Index].baseName)
        ;else
        ;    debug("    Placed " A_Index ", " itemPicked.baseName)
        valPicked := vals[A_Index]
        itemPicked := items[A_Index]
    }

    t1 := A_Tickcount
    debug("Sorting completed (total {} microseconds).", t1 - t0)
}
