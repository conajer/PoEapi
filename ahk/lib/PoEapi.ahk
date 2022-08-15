;
; PoEapi.ahk, 9/10/2020 8:27 PM
;

if (FileExist("..\poeapi.dll")) {
    FileMove ..\poeapi.dll, poeapi.dll, true
}

if (Not pLib := loadLibrary("poeapi.dll")) {
    errCode := DllCall("GetLastError")
    if (errCode == 0xc1)
        Msgbox, % "You need 64-Bit AutoHotkey to run PoEapikit."
    else
        Msgbox, % errCode ": Failed to load poeapi.dll!"
    ExitApp
}

#Include, %A_ScriptDir%\lib\ahkpp.ahk
#Include, %A_ScriptDir%\lib\Item.ahk
#Include, %A_ScriptDir%\lib\InventoryGrid.ahk
#Include, %A_ScriptDir%\lib\PoETask.ahk

; PoEapi windows messages
global WM_POEAPI_LOG       := 0x9000
global WM_PLAYER_CHANGED   := 0x9001
global WM_PLAYER_LIFE      := 0x9002
global WM_PLAYER_MANA      := 0x9003
global WM_PLAYER_ES        := 0x9004
global WM_PLAYER_DIED      := 0x9005
global WM_USE_SKILL        := 0x9006
global WM_MOVE             := 0x9007
global WM_BUFF_ADDED       := 0x9008
global WM_BUFF_REMOVED     := 0x9009
global WM_AREA_CHANGED     := 0x900a
global WM_MONSTER_CHANGED  := 0x900b
global WM_MINION_CHANGED   := 0x900c
global WM_KILL_COUNTER     := 0x900d
global WM_DELVE_CHEST      := 0x900e
global WM_PICKUP           := 0x900f
global WM_FLASK_CHANGED    := 0x9010
global WM_HEIST_CHEST      := 0x9011
global WM_NEW_MESSAGE      := 0x9012
global WM_NEW_ITEM         := 0x9013
global WM_NEW_MONSTER      := 0x9014
global WM_INVENTORY_OPENED := 0x9015
global WM_STASH_CHANGED    := 0x9016
global WM_TRADE            := 0x9017
global WM_PTASK_ATTACHED   := 0x9100
global WM_PTASK_ACTIVE     := 0x9101
global WM_PTASK_LOADED     := 0x9102
global WM_PTASK_EXIT       := 0x9103

; Initialize ahkpp
ahkpp_init(pLib)

; Register PoEapi classes
ahkpp_register_class(PoETask)
ahkpp_register_class(PoEObject)
ahkpp_register_class(Entity)
ahkpp_register_class(Item)
ahkpp_register_class(Element)
ahkpp_register_class(Inventory)
ahkpp_register_class(Stash)
ahkpp_register_class(Vendor)
ahkpp_register_class(Sell)

class PoEObject extends AhkObj {

    __read(address, size) {
        return DllCall("poeapi\poeapi_read", "Ptr", address, "Int", size, "Ptr")
    }

    getByte(offset) {
        dataPtr := DllCall("poeapi\poeapi_read", "Ptr", this.address + offset, "Char", 1, "Ptr")
        return NumGet(dataPtr + 0, "Char")
    }

    getShort(offset) {
        dataPtr := DllCall("poeapi\poeapi_read", "Ptr", this.address + offset, "Short", 2, "Ptr")
        return NumGet(dataPtr + 0, "Short")
    }

    getInt(offset) {
        dataPtr := DllCall("poeapi\poeapi_read", "Ptr", this.address + offset, "Int", 4, "Ptr")
        return NumGet(dataPtr + 0, "Int")
    }

    getFloat(offset) {
        dataPtr := DllCall("poeapi\poeapi_read", "Ptr", this.address + offset, "Int", 4, "Ptr")
        return NumGet(dataPtr + 0, "Float")
    }

    getPtr(offset) {
        dataPtr := DllCall("poeapi\poeapi_read", "Ptr", this.address + offset, "Int", 8, "Ptr")
        return NumGet(dataPtr + 0, "Ptr")
    }

    getObject(offset) {
        dataPtr := DllCall("poeapi\poeapi_read", "Ptr", this.address + offset, "Int", 8, "Ptr")
        obj := new PoEobject()
        obj.address := NumGet(dataPtr + 0, "Ptr")
        return obj
    }

    getString(offset, len) {
        dataPtr := this.__read(this.address + offset, (len + 1) * 2)
        return StrGet(dataPtr + 0)
    }

    getAString(offset, len) {
        dataPtr := this.__read(this.address + offset, len + 1)
        return StrGet(dataPtr + 0, "utf-8")
    }

    readString(offset, len = 0) {
        len := len > 0 ? len : this.getInt(offset + 0x10)
        address := this.getPtr(offset)
        dataPtr := this.__read(address, (len + 1) * 2)
        return StrGet(dataPtr + 0)
    }
}

class Entity extends PoEObject {

    getPos(ByRef x, ByRef y) {
        pos := this.__getPos()
        x := pos.x
        y := pos.y
    }
}

class Element extends PoEObject {

    getChild(params*) {
        VarSetCapacity(path, (params.Length() + 1) * 4)
        for i, n in params
            NumPut(n - 1, path, (i - 1) * 4, "Int")
        NumPut(-1, path, params.Length() * 4, "Int")

        return this.__Call("getChild", &path)
    }

    getPos(ByRef x = "", ByRef y = "") {
        r := this.getRect()
        x := r.l + r.w / 2
        y := r.t + r.h / 2

        return r
    }

    draw(label = "", color = "", depth = 0, x = 0.5, y = 0.5) {
        if (depth < 0)
            return

        Random, bgr, 0, 0xffffff
        bgr := color ? color : Format("#{:06x}", bgr)

        r := this.getPos()
        if (r.w < 0 || r.h < 0)
            return
        ptask.c.drawRect(r.l + x, r.t + y, r.w, r.h, bgr)
        if (label)
            ptask.c.drawText(label, r.l + x, r.t + y, bgr, "white", 1, 1)

        for i, e in this.getChilds() {
            if (e.isVisible()) {
                r := e.getPos()
                if (r.w != 317 && r.h != 317)
                    e.draw(label ? label "." i : i, color, depth - 1, x, y)
            }
        }
    }
}

class Inventory extends InventoryGrid {

    __new() {
        this.id := 1
    }

    open() {
        if (this.isOpened())
            return true

        SendInput, %InventoryKey%
        loop, 50 {
            if (this.isOpened())
                return true
            Sleep, 20
        }

        return false
    }

    openPortal() {
        item := this.findItem(_("Portal Scroll"))
        if (Not item)
            return

        if (Not this.isOpened()) {
            SendInput, %InventoryKey%
            loop, 10 {
                Sleep, 50
                if (this.isOpened()) {
                    closeInventory := true
                    break
                }
            }
        }

        MouseGetPos, tempX, tempY
        isLBttonPressed := GetKeyState("LButton")
        if (isLBttonPressed)
            SendInput {LButton up}

        this.use(item)
        if (closeInventory)
            SendInput, %InventoryKey%

        if (Not ptask.player.isMoving()) {
            loop, 3 {
                Sleep, 100
                portal := ptask.getNearestEntity("Portal")
                portal.getPos(x, y)
                if (x && y)
                    break
            }
            if (x && y)
                MouseMove, x, y + 100, 0
            return
        }

        MouseMove, tempX, tempY, 0
        if (isLBttonPressed)
            SendInput {LButton down}
    }

    identify(item) {
        return this.use(_("Scroll of Wisdom"), item)
    }

    drop(index = "") {
        if (ptask.getChat().isOpened())
            SendInput, {Enter}

        if (Not ptask.inventories[13].getItemByIndex(1))
            return false

        Critical
        this.open()
        if (index || index := this.nextCell()) {
            this.moveTo(index)
            MouseClick
            return true
        }

        return false
    }
}

class StashTab extends InventoryGrid {

    dump(regex, n = 0) {
        dumped := 0
        for i, aItem in this.getItems() {
            if (n && dumped >= n)
                break

            if (Not this.isVisible() || Not ptask.inventory.freeCells())
                break

            if (aItem.name ~= "i)"regex) {
                stackCount := aItem.stackCount()
                if (Not stackCount) {
                    this.move(aItem)
                    dumped += 1
                    continue
                }

                stackSize := aItem.stackSize()
                k := (n && (n - dumped < stackCount)) ? n - dumped : stackCount
                while (k > 0) {
                    m := k
                    this.moveTo(aItem.index)
                    hoveredItem := ptask.getHoveredItem()
                    if (Not hoveredItem || Not hoveredItem.name ~= "i)"regex)
                        break

                    while (m > 0) {
                        if (m >= stackSize || k == stackCount) {
                            SendInput, {Ctrl down}
                            SendInput, {Click}
                            SendInput, {Ctrl up}
                            Sleep, 30
                            m -= stackSize
                        } else {
                            aItem := this.getItemByIndex(aItem.index)
                            l := stackCount - aItem.stackCount()
                            if (l < k - m) {
                                m := k - l
                                continue
                            }

                            Sleep, 30
                            SendInput, {Shift down}
                            SendInput, {Click}
                            SendInput, {Shift up}
                            SendInput, %m%{Enter}
                            Sleep, 100
                            
                            if (Not ptask.inventory.drop()) {
                                this.moveTo(aItem.index)
                                SendInput, {Click}
                                return dumped
                            }

                            m -= m
                        }

                        if (Not ptask.inventory.freeCells())
                            break
                    }

                    Sleep, 50
                    aItem := this.getItemByIndex(aItem.index)
                    m := aItem ? stackCount - aItem.stackCount() : stackCount
                    dumped += m
                    stackCount -= m
                    k -= m

                    if (m == 0 || Not ptask.inventory.freeCells())
                        break
                }
            }
        }

        return dumped
    }

    getHighlightedItems() {
        items := []
        for i, e in this.getChilds() {
            if (e.isHighlighted)
                items[e.index] := e.item
        }

        return items.Length() > 0 ? items : ""
    }
}

class SpecialStashTab extends StashTab {

    getChilds() {
        this.getItems()
        childs := []
        for i, e in this.__Call("getChilds") {
            if (e.getChilds().Length() == 2) {
                e.index := e.childs[2].getIndex(this.rows)
                , e.item := this.items[e.index]
                , e.isHighlighted := e.childs[2].isHighlighted()
                , childs[e.index] := e
            }
        }
        this.childs := childs

        return this.childs
    }

    getRectByIndex(index) {
        return this.getChilds()[index].getRect()
    }
}

class FolderTab extends SpecialStashTab {

    id {
        Get {
            return this.getTab().id
        }
    }

    getChilds() {
        return this.getTab().getChilds()
    }

    getRectByIndex(index) {
        return this.getTab().getRectByIndex(index)
    }

    getTab() {
        this.__Call("getChilds")
        tabIndex := this.childs[2].getInt(0x9e8)
        __tab := ptask.stashTabs[this.index].tabs[tabIndex + 1]
        if (Not __tab.getId())
            return

        if (this.tab.id == __tab.id)
            return this.tab

        tab := this.childs[2].getChild(2, tabIndex + 1)
        if (__tab.type > 2 && __tab.type != 7) {
            tab := tab.getChild(1)
            if (__tab.type == 3)
                tab.base := CurrencyTab
            else if (__tab.type == 9)
                tab.base := FragmentTab
            else
                tab.base := SpecialStashTab
        } else {
            tab := tab.getChild(1, 1)
            tab.base := StashTab
        }

        tab.id := __tab.id
        tab.name := __tab.name
        tab.index := i
        tab.type := __tab.type

        this.tab := tab
        this.id := tab.id

        return tab
    }

    hasTab(name) {
        for i, t in ptask.stashTabs[this.index].tabs {
            if (t.name == name)
                return true
        }

        return false
    }
}

class CurrencyTab extends SpecialStashTab {

    getChilds() {
        this.getItems()
        childs := []
        for i, e in this.__Call("getChilds") {
            if (e.getChilds().Length() == 2) {
                e.index := e.childs[2].getIndex(this.rows)
                , e.item := this.items[e.index]
                , e.isHighlighted := e.childs[2].isHighlighted()
                , childs[e.index] := e
            } else {
                for j, e in e.getChilds() {
                    if (e.getChilds().Length() == 2) {
                        e.index := e.childs[2].getIndex(this.rows)
                        , e.item := this.items[e.index]
                        , e.isHighlighted := e.childs[2].isHighlighted()
                        , childs[e.index] := e
                    }
                }
            }
        }
        this.childs := childs

        return this.childs
    }
}

class FragmentTab extends SpecialStashTab {

    getChilds() {
        this.getItems()
        childs := []
        for i, e in this.__Call("getChilds") {
            if (e.isVisible()) {
                for i, e in e.__Call("getChilds") {
                    if (e.getChilds().Length() == 2) {
                        e.index := e.childs[2].getIndex(this.rows)
                        , e.item := this.items[e.index]
                        , e.isHighlighted := e.childs[2].isHighlighted()
                        , childs[e.index] := e
                    }
                }
                break
            }
        }
        this.childs := childs

        return this.childs
    }
}

class Stash extends Element {

    Tab {
        Get {
            activeTabIndex := this.activeTabIndex() + 1
            return this.getTab(ptask.stashTabs[activeTabIndex].name)
        }
    }

    open() {
        if (this.isOpened())
            return true

        SendInput, %CloseAllUIKey%
        Sleep, 100
        if (ptask.select("/Stash")) {
            loop 20 {
                if (this.isOpened())
                    return true
                Sleep, 100
            }
        }

        return false
    }

    getTab(tabName) {
        if (Not this.hasTab(tabName))
            return
        if (Not this.tabs.hasKey(tabName))
            this.__getTabs()
        return this.tabs[tabName]
    }

    switchTab(tabName) {
        if (Not this.isOpened() || Not tabName)
            return

        if (Not this.tabs.hasKey(tabName)) {
            if (Not this.tabs || this.hasTab(tabName))
                this.__getTabs()

            for i, t in this.tabs {
                if (t.type == 0x10 && t.hasTab(tabName)) {
                    tabName := t.name
                    break
                }
            }
        }

        activeTabIndex := this.activeTabIndex() + 1
        if (ptask.stashTabs[activeTabIndex].name != tabName) {
            if (this.tabs.hasKey(tabName)) {
                tabIndex := this.tabs[tabName].index
            } else {
                for i, tab in ptask.stashTabs {
                    if (tab.name == tabName) {
                        tabIndex := tab.index + 1
                        break
                    }
                }
            }

            if (Not tabIndex)
                return

            n := abs(activeTabIndex - tabIndex)
            key := (activeTabIndex > tabIndex) ? "Left" : "Right"
            SendInput {%key% %n%}
        }

        return this.tabs[tabName]
    }

    __getTabs() {
        ptask.getStashTabs()
        this.tabs := {}
        stashTabs := this.getChild(1, 2, 2)
        for i, tab in stashTabs.getChilds() {
            __tab := ptask.stashTabs[i]
            if (Not __tab.getId() && __tab.type != 16)
                continue

            if (__tab.type == 3) {
                tab := tab.getChild(1)
                tab.base := CurrencyTab
            } else if (__tab.type > 2 && __tab.type != 7) {
                tab := tab.getChild(1)
                tab.base := (__tab.type == 16) ? FolderTab : SpecialStashTab
            } else {
                tab := tab.getChild(1, 1)
                tab.base := StashTab
            }

            tab.id := __tab.id
            tab.name := __tab.name
            tab.index := i
            tab.type := __tab.type
            this.tabs[tab.name] := tab
        }
    }
}

class Vendor extends Element {

    selectNPC(name = "") {
        if (this.isSelected()) {
            if (Not name || this.name ~= name)
                return true
        }
        
        SendInput, %CloseAllUIKey%
        Sleep, 100
        return ptask.select(name ? name : "NPC")
    }

    selectService(name) {
        loop, 50 {
            if (this.isSelected() && this.getServices().Count()) {
                if (Not this.services.hasKey(name))
                    break

                this.services[name].getPos(x, y)
                MouseClick(x, y)
                return true
            }
            Sleep, 50
        }

        return false
    }

    sell() {
        if (ptask.getSell().isOpened())
            return true

        if (Not this.selectNPC(ptask.areaName == "The Rogue Harbour" ? "Faustus" : ""))
            return

        if (this.selectService(_("Sell Items"))) {
            loop, 10 {
                if (ptask.getSell().isOpened())
                    return true
                Sleep, 50
            }
        }

        return false
    }

    tradeDivinationCards() {
        if (this.selectNPC(_("Lilly Roth"))) {
            return this.selectService(_("Trade Divination Cards"))
        }

    }
}

class Sell extends Element {

    accept(flag = false) {
        this.getChild(6).getPos(x, y)
        MouseMove, x, y, 0
        if (flag) {
            Sleep, 50
            MouseClick, Left, x, y
        }
    }
}

class NotificationPopup extends Element {

    accept(flag = false) {
        this.getChild(3, 1, 1).getPos(x, y)
        MouseMove, x, y, 0
        if (flag) {
            Sleep, 50
            MouseClick, Left, x, y
        }
    }
}
