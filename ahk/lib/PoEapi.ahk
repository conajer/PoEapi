;
; PoEapi.ahk, 9/10/2020 8:27 PM
;

if (FileExist("..\poeapi.dll")) {
    FileMove ..\poeapi.dll, poeapi.dll, true
}

if (Not DllCall("LoadLibrary", "Str", "poeapi.dll", "ptr")) {
    Msgbox, % DllCall("GetLastError") ": Load poeapi.dll failed!"
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
global WM_PTASK_ATTACHED   := 0x9100

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

    getByte(address) {
        dataPtr := DllCall("poeapi\poeapi_read", "Ptr", address, "Char", 1, "Ptr")
        return NumGet(dataPtr + 0, "Char")
    }

    getShort(address) {
        dataPtr := DllCall("poeapi\poeapi_read", "Ptr", address, "Short", 2, "Ptr")
        return NumGet(dataPtr + 0, "Short")
    }

    getInt(address) {
        dataPtr := DllCall("poeapi\poeapi_read", "Ptr", address, "Int", 4, "Ptr")
        return NumGet(dataPtr + 0, "Int")
    }

    getFloat(address) {
        dataPtr := DllCall("poeapi\poeapi_read", "Ptr", address, "Int", 4, "Ptr")
        return NumGet(dataPtr + 0, "Float")
    }

    getPtr(address) {
        dataPtr := DllCall("poeapi\poeapi_read", "Ptr", address, "Int", 8, "Ptr")
        return NumGet(dataPtr + 0, "Ptr")
    }

    getString(address, len) {
        dataPtr := this.__read(address, (len + 1) * 2)
        return StrGet(dataPtr + 0)
    }

    getAString(address, len) {
        dataPtr := this.__read(address, len + 1)
        return StrGet(dataPtr + 0, "utf-8")
    }

    readString(address, len = 0) {
        len := len > 0 ? len : this.getInt(address + 0x10)
        address := this.getPtr(address)
        dataPtr := this.__read(address, (len + 1) * 2)
        return StrGet(dataPtr + 0)
    }
}

class Entity extends PoEObject {

    getPos(ByRef x, ByRef y) {
        pos := this.__getPos()
        x := NumGet(pos + 0x0, "Int")
        y := NumGet(pos + 0x4, "Int")
    }
}

class Element extends PoEObject {

    getChild(params*) {
        element := this
        for i, n in params {
            element.getChilds()
            element := element.childs[n]
        }

        return element
    }

    getPos(ByRef x = "", ByRef y = "") {
        r := this.getRect()
        x := r.l + r.w / 2
        y := r.t + r.h / 2

        return r
    }

    draw(label = "", color = "", depth = 1) {
        if (depth < 0)
            return

        if (Not color)
            Random, bgr, 0, 0xffffff
        else
            bgr := color

        r := this.getPos()
        if (r.w < 0 || r.h < 0)
            return

        ptask.c.drawRect(r.l, r.t, r.w, r.h, bgr)
        if (label)
            ptask.c.drawText(r.l, r.t, 100, 20, label, bgr)

        for i, e in this.getChilds() {
            if (e.isVisible()) {
                r := e.getPos()
                if (r.w != 317 && r.h != 317)
                    e.draw(label ? label "." i : i, color, depth - 1)
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
        if (Not this.isOpened()) {
            SendInput, %InventoryKey%
            Sleep, 100
            closeInventory := true
        }

        MouseGetPos, tempX, tempY
        isLBttonPressed := GetKeyState("LButton")
        if (isLBttonPressed)
            SendInput {LButton up}

        item := this.findItem("Portal")
        if (Not item) {
            debug("!!! Out of ""Portal Scroll"".")
            return
        }

        this.use(item)
        if (closeInventory)
            SendInput {f}

        ;if (Not ptask.player.isMoving()) {
            Sleep, 100
            portal := ptask.getNearestEntity("Portal")
            portal.getPos(x, y)
            MouseMove, x, y + 100, 0
            return
        ;}

        MouseMove, tempX, tempY, 0
        if (isLBttonPressed)
            SendInput {LButton down}
    }

    identify(item, shift = false) {
        if (Not shift) {
            wisdom := this.findItem("Scroll of Wisdom")
            if (Not wisdom)
                return false

            this.moveTo(wisdom.index)
            MouseClick, Right
        }

        if (item) {
            this.moveTo(item.index)
            MouseClick, Left
        }

        return true
    }

    use(item, targetItem = "", n = 1) {
        if (Not item)
            return item

        if (n > 1)
            SendInput {Shift down}

        this.moveTo(item.index)
        Click, Right

        if (targetItem) {
            this.moveTo(targetItem.index)
            loop, % n {
                Click, Left
                Sleep, 100
            }
            SendInput {Shift up}

            return this.getItemByIndex(targetItem.index)
        }
    }
}

class StashTab extends InventoryGrid {
}

class SpecialStashTab extends StashTab {

    getRectByIndex(index) {
        index := Round((index - 1) / this.rows + 1)

        ; Currency tab
        if (this.type == 3)
            index := (index > 28) ? index - 1 : index

        return this.childs[index].getRect()
    }
}

class Stash extends Element {

    __new() {
        this.__getTabs()
        OnMessage(WM_AREA_CHANGED, ObjBindMethod(this, "__getTabs"))
    }

    Tab {
        Get {
            return this.getTab(ptask.stashTabs[this.activeTabIndex() + 1].name)
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
        if (Not this.tabs[tabName])
            this.__getTabs()
        return this.tabs[tabName]
        }

    switchTab(tabName) {
        if (Not this.isOpened())
            return

        activeTabIndex := this.activeTabIndex() + 1
        if (ptask.stashTabs[activeTabIndex].name != tabName) {
            tab := this.getTab(tabName)
            if (tab) {
            n := abs(activeTabIndex - tab.index)
            key := (activeTabIndex > tab.index) ? "Left" : "Right"
            SendInput {%key% %n%}
        }
        }

        loop, 3 {
            Sleep, 20
            if (this.activeTabIndex() == tab.index)
                break
        }

        return tab
    }

    __getTabs() {
        this.tabs := {}
        stashTabs := this.getChild(2)
        for i, tab in stashTabs.getChilds() {
            __tab := ptask.stashTabs[i]
            if (Not __tab.getId())
                continue

            if (__tab.type > 2 && __tab.type != 7) {
                tab := tab.getChild(1)
                tab.base := SpecialStashTab
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

    sell(vendorName = "NPC") {
        sell := ptask.getSell()
        if (sell.isOpened())
            return true

        if (Not this.isSelected()) {
            SendInput, %CloseAllUIKey%
            Sleep, 100
            if (Not ptask.select(vendorName))
                return false
        }

        this.services := ""
        loop, 30 {
            this.getServices()
            if (this.services) {
                service := this.services["Sell Items"]
                if (Not service)
                    return this.select("Navali")
                break
            }
            Sleep, 50
        }

        if (service) {
            service.getPos(x, y)
            MouseClick(x, y)

            loop, 10 {
                if (sell.isOpened())
                    return true
                Sleep, 50
            }
        }

        return false
    }
}

class Sell extends Element {

    accept(flag = false) {
        this.sellPanel.childs[6].getPos(x, y)
        if (flag)
            MouseClick, Left, x, y
        else
            MouseMove, x, y, 0
    }

    getItems() {
        this.__getItems()
        return this.items
    }

    getYourItems() {
        this.__getYourItems()
        return this.yourItems
    }
}

class InventorySlot extends AhkObj {

    getItems() {
        this.__getItems()
        return this.items
    }
}
