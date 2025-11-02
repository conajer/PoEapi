;
; InventoryGrid.ahk, 9/20/2020 5:01 PM
;

class InventoryGrid extends Element {

    __new(id) {
        this.id := id
    }

    id {
        Get {
            return this.id := this.getId()
        }

        Set {
            if (value > 0) {
                ObjRawSet(this, "id", value)
                if (Not ptask.inventories[this.id])
                    ptask.getInventorySlots()

                this.rows := ptask.inventories[this.id].rows
                this.cols := ptask.inventories[this.id].cols
                this.rect := this.getRect()

                return this.id
            }
        }
    }

    use(ByRef aItem, ByRef target = "") {
        if (aItem && Not IsObject(aItem)) {
            if (Not aItem := this.findItem(aItem))
                return false
        }

        if (Not GetKeyState("Shift") || Not target) {
            if (Not aItem || Not IsObject(aItem))
                return false

            this.moveTo(aItem.index)
            Click, Right
        }

        if (target) {
            this.moveTo(target.index)
            Click, Left
            Sleep, 100
            
            aItem := this.getItemByIndex(aItem.index)
            target := this.getItemByIndex(target.index)
        }

        return true
    }

    dump(regex = "", n = 0) {
        dumped := 0
        for i, aItem in this.findItems(regex) {
            if (Not this.isVisible())
                break

            count := aItem.stackCount ? aItem.stackCount : 1
            this.move(aItem)
            dumped += count
            if (n > 0 && dumped >= n)
                break
        }

        return dumped
    }

    findItem(regex, ByRef index = 1, rarity = 3) {
        for i, aItem in this.getItems() {
            if (aItem.Index < index)
                continue

            index := aItem.Index + 1
            if (aItem.rarity <= rarity && (InStr(aItem.qualifiedName(), regex)
                || RegExMatch(aItem.qualifiedName(), "i)" regex)))
                return aItem
        }
    }

    findItems(regex, rarity = 3) {
        result := []
        for i, aItem in this.getItems() {
            if (aItem.rarity <= rarity && (InStr(aItem.qualifiedName(), regex)
                || RegExMatch(aItem.qualifiedName(), "i)" regex)))
                result.Push(aItem)
        }

        return result.Count() ? result : ""
    }

    checkItem(regex) {
        for i, aItem in this.findItems(regex)
            n += aItem.stackCount ? aItem.stackCount : 1

        return n
    }

    getItem(left, top) {
        return this.getItemByIndex((left - 1) * this.rows + top)
    }

    getItemByIndex(index) {
        if (Not ptask.inventories[this.id])
            ptask.getInventorySlots()

        item := ptask.inventories[this.id].getItemByIndex(index)
        item.price := $(item)
        return item
    }

    getItems() {
        if (Not ptask.inventories[this.id])
            ptask.getInventorySlots()

        this.items := ptask.inventories[this.id].getItems()
        for i, item in this.items
            item.price := $(item)
        return this.items
    }

    moveTo(index) {
        r := this.getRectByIndex(index)
        MouseMove, (r.l + r.r) / 2, (r.t + r.b) / 2, 0
        Sleep, 30
    }

    move(aItem) {
        r := this.getRectByIndex(aItem.index)
        x := (r.l + r.r) / 2
        y := (r.t + r.b) / 2

        SendInput, {Ctrl down}
        loop, 5 {
            MouseMove, x, y, 0
            SendInput, {Click}
            Sleep, 20
            if (Not this.getItemByIndex(aItem.index) || Not ptask.getHoveredElement().enabled)
                break
        }
        SendInput, {Ctrl up}
    }

    highlight(aItem) {
        r := this.getRectByIndex(aItem.Index)
        ptask.c.drawRect(r.l + .5, r.t + 1.5, r.w - .5, r.h - .5, "#f7e478")
    }

    drawItems() {
        for i, aItem in this.getItems()
            this.highlight(aItem)
    }

    getRectByIndex(index) {
        aItem := this.items[index]
        , l := (index - 1) // this.rows + 1
        , t := Mod(index - 1, this.rows) + 1
        , w := this.rect.w / this.cols
        , h := this.rect.h / this.rows
        , x := this.rect.l + w * (l - 1)
        , y := this.rect.t + h * (t - 1)
        , w := w * (aItem ? aItem.width : 1)
        , h := h * (aItem ? aItem.height : 1)

        return new Rect(x, y, w, h)
    }

    getLayout() {
        return ptask.inventories[this.id].getLayout()
    }

    freeCells() {
        return ptask.inventories[this.id].freeCells()
    }

    nextCell(w = 1, h = 1) {
        return ptask.inventories[this.id].nextCell(w, h)
    }

    getChilds() {
        this.__Call("getChilds")
        this.childs.RemoveAt(1)
        _items := []
        for i, item in this.getItems()
            _items[item.address] := item
        for i, e in this.childs {
            e.item := _items[e.getItem()]
            e.index := e.item.index
        }

        return this.childs
    }
}
