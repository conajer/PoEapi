;
; debug.ahk, 9/21/2020 9:10 PM
;

addDebugButton() {
    if (ShowDebugMenu) {
        Menu, __debugMenu, Add, Draw inventory, drawInventory
        Menu, __debugMenu, Add, List inventory items, listInventoryItems
        Menu, __debugMenu, Add
        Menu, __debugMenu, Add, Draw stash tab, drawStashTab
        Menu, __debugMenu, Add, List stash tab items, listStashTabItems
        Menu, __debugMenu, Add, List stash tabs, listStashTabs
        Menu, __debugMenu, Add
        Menu, __debugMenu, Add, List flasks, listFlasks

        Gui, Add, Button, x+10 y0 gpopupDebugCommands, Debug
    }
}

popupDebugCommands() {
    Menu, __debugMenu, Show
}

drawInventory() {
    ptask.inventory.draw()
}

listInventoryItems() {
    ptask.inventories[1].getItems()
    debug("Inventory:")
    if (ptask.inventories[1].Count() == 0) {
        debug("    No items.")
        Return
    }

    for i, item in ptask.inventories[1].items
        debug("    {:2d}. {}", item.index , item.name)
}

drawStashTab() {
    ptask.stash.draw()
}

listStashTabItems() {
    
}

listStashTabs() {
    if (ptask.stashTabs.Count() == 0) {
        debug("No stash tabs.")
        Return
    }

    debug("Stash tabs:")
    for i, tab in ptask.stashTabs
        debug("    {}. {}, {}, {:#x}, {:#x}", i
              , tab.index
              , tab.name
              , tab.type
              , tab.flags)
}

listFlasks() {
    if (ptask.player.flasks.Count() == 0) {
        debug("No flasks.")
        Return
    }

    debug("Flasks:")
    for i, flask in ptask.player.flasks
        debug("    {}. {}, {}, {}, {}", i
              , flask.item.name
              , flask.chargesPerUse
              , flask.maxCharges
              , flask.duration)
}
