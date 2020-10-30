;
; InventoryGrid.ahk, 9/20/2020 5:01 PM
;

class InventoryGrid extends Element {

    __new(id) {
        this.id := id
        this.rows := ptask.inventories[id].rows
        this.cols := ptask.inventories[id].cols
        this.rect := this.getPos()
    }

    dump(regex = "", n = 0) {
        this.checkLayout()
        for i, aItem in this.getItems() {
            if (regex && Not RegExMatch(aItem.Name " " aItem.BaseName, "i)" regex))
                continue

            count := aItem.stackCount ? aItem.stackCount : 1
            if (n == 0 || (dumped + count) <= n) {
                this.moveItem(aItem)
                dumped += count
            }

            if (n && dumped >= n)
                return dumped
        }

        return dumped
    }

    findItem(regex, ByRef index = 1, rarity = 3) {
        for i, aItem in this.getItems() {
            if (aItem.Index < index)
                continue

            index := aItem.Index + 1
            if (aItem.rarity <= rarity && RegExMatch(aItem.name, "i)" regex))
                return aItem
        }
    }

    checkItem(regex) {
        for i, aItem in this.getItems() {
            if (RegExMatch(aItem.name, "i)" regex))
                n += aItem.stackCount ? aItem.stackCount : 1
        }

        return n
    }

    getItem(left, top) {
        ptask.inventories[this.id].getItems()
        return ptask.inventories[this.id].items[(left - 1) * this.rows + top]
    }

    getItemByIndex(index) {
        ptask.inventories[this.id].getItems()
        return ptask.inventories[this.id].items[index]
    }

    getItems() {
        ptask.inventories[this.id].getItems()
        return ptask.inventories[this.id].items
    }

    moveTo(index) {
        r := this.getRectByIndex(index)
        MouseMove, (r.l + r.r) / 2, (r.t + r.b) / 2, 0
        Sleep, 30
    }

    move(aItem) {
        r := this.getRectByIndex(aItem.index)
        MouseMove, (r.l + r.r) / 2, (r.t + r.b) / 2, 0
        Sleep, 30
        SendInput {Ctrl down}
        Click
        SendInput {Ctrl up}
        Sleep, 10
    }

    highlight(aItem) {
        r := this.getRectByIndex(aItem.Index)
        ptask.c.drawRect(r.l, r.t, r.w, r.h, 0x77B4E7)
    }

    drawItems() {
        for i, aItem in this.getItems()
            this.highlight(aItem)
    }

    getRectByIndex(index) {
        l := (index - 1) // this.rows + 1
        t := Mod(index - 1, this.rows) + 1

        w := this.rect.w / this.cols
        h := this.rect.h / this.rows
        x := this.rect.l + w * (l - 1)
        y := this.rect.t + h * (t - 1)

        return new Rect(x, y, w + 1, h + 1)
    }
}
