;
; debug.ahk, 9/21/2020 9:10 PM
;

addDebugButton() {
    if (ShowDebugMenu) {
        Menu, __debugMenu, Add, Draw inventory, drawInventory
        Menu, __debugMenu, Add, List inventory items, listInventoryItems
        Menu, __debugMenu, Add, List inventory slots, listInventorySlots
        Menu, __debugMenu, Add
        Menu, __debugMenu, Add, Draw stash tab, drawStashTab
        Menu, __debugMenu, Add, List stash tab items, listStashTabItems
        Menu, __debugMenu, Add, List stash tabs, listStashTabs
        Menu, __debugMenu, Add
        Menu, __debugMenu, Add, List vendor services, listVendorServices
        Menu, __debugMenu, Add
        Menu, __debugMenu, Add, List flasks, listFlasks
        Menu, __debugMenu, Add, List flask slot, listFlaskSlot

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
        debug("    {:2d}. {}", item.index, item.name)
}

listInventorySlots() {
    debug("Inventory slots:")
    for i, slot in ptask.inventories
        debug("    {:2d}. {}, {}, {}", slot.id, slot.rows, slot.cols, slot.items.Count())
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

listVendorServices() {
    vendor := ptask.getVendor()
    if (Not vendor.name) {
        debug("No vendor selected.")
        return
    }

    vendor.getServices()
    debug("{}'s services:", vendor.name)
    for name in vendor.services
        debug("    {}. {}", A_Index, name)
        vendor.draw()
}

listFlasks() {
    if (ptask.player.flasks.Count() == 0) {
        debug("No flasks.")
        Return
    }

    debug("Flasks:")
    for i, flask in ptask.player.flasks
        debug("    {}. {:2d}, {:2d}, {}, {}, {}"
              , flask.key
              , flask.item.charges
              , flask.chargesPerUse
              , flask.maxCharges
              , flask.duration
              , flask.item.name)
}

listFlaskSlot() {
    ptask.inventories[12].getItems()
    debug("Flask slot:")
    if (ptask.inventories[12].Count() == 0) {
        debug("    No items.")
        Return
    }

    for i, item in ptask.inventories[12].items
        debug("    {:2d}. {}", item.index, item.name)
}
