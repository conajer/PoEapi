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
        Menu, __debugMenu, Add
        Menu, __debugMenu, Add, Show log messages, toggleLogMessage

        Gui, Add, Button, x+2 y0 gpopupDebugCommands, Debug
    }
}

toggleLogMessage() {
    ShowLogMessage := !ShowLogMessage
}

popupDebugCommands() {
    Menu, __debugMenu, % ShowLogMessage ? "Check" : "Uncheck", Show log messages
    Menu, __debugMenu, Show
}

drawInventory() {
    ptask.inventory.draw()
}

listInventoryItems() {
    debug("Inventory:")
    items := ptask.inventory.getItems()
    if (items.Count() == 0) {
        debug("    No items.")
        Return
    }

    for i, item in items
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
    debug("Stash tab name: {}", ptask.stash.Tab.name)
    for i, item in ptask.stash.Tab.getItems()
        debug("    {:3d}. {}", item.index, item.name)
}

listStashTabs() {
    stashTabs := ptask.getStashTabs()
    if (stashTabs.Count() == 0) {
        debug("No stash tabs.")
        Return
    }

    debug("Stash tabs:")
    for i, tab in stashTabs {
        debug("    {:2d}. {:2d}, {:-32s}, {:#x}, {:#x} {:#x}", i
              , tab.index
              , (tab.folderId >= 0) ? "    " tab.name : tab.name
              , tab.type
              , tab.flags
              , tab.affinities)
    }
}

listVendorServices() {
    vendor := ptask.getVendor()
    if (Not vendor.name) {
        debug("No vendor selected.")
        return
    }

    debug("{}'s services:", vendor.name)
    for name in vendor.getServices()
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
        debug("    {}. {}", item.index, item.name)
}
