;
; debug.ahk, 9/21/2020 9:10 PM
;

#Include, %A_ScriptDir%\extras\Eval.ahk

addMenuItem("__debug", _("Console"), "openConsole")
addMenuItem("__debug", _("IngameUI Inspector"), "openInspector")
addMenuItem("__debug")
addMenuItem("__debug", _("List inventory items"), "listInventoryItems")
addMenuItem("__debug", _("List inventory slots"), "listInventorySlots")
addMenuItem("__debug")
addMenuItem("__debug", _("List stash tab items"), "listStashTabItems")
addMenuItem("__debug", _("List stash tabs"), "listStashTabs")
addMenuItem("__debug")
addMenuItem("__debug", _("List vendor services"), "listVendorServices")
addMenuItem("__debug")
addMenuItem("__debug", _("List flasks"), "listFlasks")
addMenuItem("__debug", _("List flask slot"), "listFlaskSlot")
addMenuItem("__debug")
addMenuItem("__debug", _("List buffs"), "listBuffs")
addMenuItem("__debug", _("List entities"), "listEntities")

Hotkey, ^!d, openConsole
Hotkey, ^i, openInspector

addExtraMenu(_("Debug"), ":__debug")
logger.level := 0
logger.show()
OnMessage(WM_POEAPI_LOG, "onLog")

class IngameUIInspector extends AhkGui {

    elements := {}

    __new() {
        global

        base.__new("IngameUI Inspector")
        Gui, Margin, 5, 5
        Gui, Color, White
        Gui, Font, s9, Courier New
        Gui, Add, TreeView, % "r20 w400 h600 AltSubmit vIngameUITree gL1 v" this.__var("onSelected")
        Gui, Add, Edit, % "ys w300 h300 ReadOnly Multi Hwnd" this.__var("elementInfo")

        ingameUI := ptask.getIngameUI()
        this.addElement(ingameUI, 0, "IngameUI", 0, 2)
    }

    addElement(e, index, label = "", parentId = 0, depth = 1) {
        label := label ? "<" label "> " e.getText() : e.getText()
        itemId := TV_Add(Format("{:d}. {:X} {}", index, e.address, label), parentId, "Expand")
        e.getChilds()
        this.elements[itemId] := e

        if (depth > 0) {
            for i, c in e.childs
                this.addElement(c, i,, itemId, depth - 1)
        }
    }

    expandAll(itemId) {
        if (Not itemId)
            return

        TV_Modify(itemId, "Expand")
        itemId := TV_GetChild(itemId)
        loop, {
            if (Not itemId)
                break

            this.expandAll(itemId)
            itemId := TV_GetNext(itemId)
        }
    }

    onSelected() {
        if (A_GuiEvent == "S") {
            e := this.elements[A_EventInfo]
            r := e.getPos()
            GuiControl,, % this.elementInfo
                , % Format("Address: {:x}`nText: {}`nX: {}`nY: {}`nWidth: {}`nHeight: {}`nChilds: {}"
                , e.address, e.getText(), r.l, r.t, r.w, r.h, e.childs.Count())

            ptask.c.clear()
            e.draw(,, 1)
        } else if (A_GuiEvent == "DoubleClick") {
            e := this.elements[A_EventInfo]
            if (Not TV_GetChild(A_EventInfo) && e.childs.Count() > 0)
                for i, c in e.childs
                    this.addElement(c, i,, A_EventInfo, 1)
            this.expandAll(A_EventInfo)
        }
    }

    drawElement() {
        itemId := TV_GetSelection()
        this.elements[itemId].draw(,, 1)
    }
}

class Console extends AhkGui {

    static __history := {}
    static __index := 0

    __new() {
        global

        base.__new("Console")
        Gui, Margin, 5
        Gui, Font, s10, Calibri
        Gui, Add, ActiveX, Border VScroll r20 w700 v__mshtml, about:
        Gui, Add, Edit, % "-WantReturn w625 HwndinputHwnd v" this.__var("input")
        Gui, Add, Button, % "Default x+10 yp+0 gL1 v" this.__var("execute"), Execute

        Console.__history := StrSplit(db.load("console.history"), "`n")
        Console.__index := Console.__history.length() + 1
        this.hInput:= inputHwnd
        this.doc := __mshtml.Document
        this.doc.write("<pre style=""font-family:Consolas; font-size:18px; line-height:1.2"">")

        this.onMessage(0x100, "onKeyDown")
    }

    onKeyDown(keyCode, lParam, message, hwnd) {
        static EM_SETSEL = 0xb1
        static EM_SCROLL := 0xb5
        static EM_REPLACESEL := 0xC2

        if (hwnd == this.hInput) {
            if (keyCode == 0x26 && Console.__index > 1) {
                GuiControl,, % this.__var("input"), % this.__history[--Console.__index]
                len := DllCall("GetWindowTextLength", "uint", this.hInput)
                SendMessage, EM_SETSEL, len, len,, % "ahk_id " this.hInput
                return 0
            } else if (keyCode == 0x28 && Console.__index > 0 && Console.__index < this.__history.Count()) {
                GuiControl,, % this.__var("input"), % this.__history[++Console.__index]
                len := DllCall("GetWindowTextLength", "uint", this.hInput)
                SendMessage, EM_SETSEL, len, len,, % "ahk_id " this.hInput
                return 0
            } else if (keyCode == 0x1b) {
                GuiControl,, % this.__var("input")
            }
        }
    }

    execute() {
        guiId := this.Hwnd
        Gui, %guiId%:Submit, NoHide
        if (Not Trim(this.input))
            return

        GuiControl,, % this.__var("input")
        this.doc.write(Format("<b>></b> {}`n", this.input))
        this.__history.Push(this.input)
        this.__history.lenght() > 100 ? this.__history.RemoveAt(1)
        Console.__index := this.__history.Count() + 1
        result := Eval(this.input)
        result := StrJoin(result, "`n")
        if (result) {
            if (IsObject(result)) {
                baseClasses := ""
                base := result.base
                loop {
                    if (Not base)
                        break
                    baseClasses .= " -> " (ObjRawGet(base, "__Class") ? base.__Class : Format("{:#x}", &base))
                    base := base.base
                }

                ellipsisMin := 15
                if (result.Count() > ellipsisMin + 5)
                    ellipsisMax := result.Count()
                this.doc.writeln(Format("<i style=""color:blue""><b>{:#x}{}:</b></i>", &result, baseClasses))
                for k, v in result {
                    try {
                        if (A_Index > ellipsisMin && A_Index < ellipsisMax) {
                            if (A_Index == ellipsisMin + 1)
                                this.doc.writeln("<i style=""color:blue"">    ...</i>")
                            continue
                        }

                        if (IsObject(v)) {
                            if (IsFunc(v))
                                this.doc.writeln(Format("<i style=""color:blue"">    <b>{}()</b></i>", k))
                            else if (v.Length() > 0)
                                this.doc.writeln(Format("<i style=""color:blue"">    <b>{}[{}]</b></i>", k, v.Length()))
                            else
                                this.doc.writeln(Format("<i style=""color:blue"">   *<b>{}</b></i>", k))
                        } else {
                            this.doc.writeln(Format("<i style=""color:blue"">    <b>{}</b>, {}</i>", k, v))
                        }
                    } catch {}
                }
            } else {
                this.doc.writeln(Format("<i style=""color:red"">{}</i>", result))
            }
        }
        this.doc.parentWindow.scrollTo(0, this.doc.body.scrollHeight)
    }

    __onClose(wParam, lParam, msg, hwnd) {
        if (this.Hwnd == hwnd) {
            for i, stmt in this.__history
                (A_Index == 1) ? histories := stmt : histories .= "`n" stmt
            db.store("console.history", histories)
            base.__onClose()
        }
    }
}

class Profiler {

    static __count := 0
    sequencer := []
    lastProbe := ""
    nProbes := 0
    probePoints := {}

    __new(title = "") {
        ++Profiler.__count
        this.title := title ? title : "Profiler " Profiler.__count
    }

    setProbe(name, label = "") {
        p := this.probePoints[name]
        if (not p) {
            this.sequencer.push(name)
            p := this.probePoints[name] := []
        }
        p.push({"label": label, "tval": A_Tickcount, "index": ++this.nProbes})
        this.lastProbe := p
    }

    analyze(durations) {
        n := durations.length()
        min := max := durations[1].tval
        total := 0
        for i, d in durations {
            d.tval < min ? min := d.tval
            d.tval > max ? max := d.tval
            total += d.tval
        }
        average := Ceil(n ? total / n : 0)

        for i, d in durations
            trace("   {}. <b style='color:{};'>{}{}</b> ms", i
                , d.tval > average ? "red" : "grey"
                , d.label ? d.label ": " : "", d.tval)
        debug("<b>total time</b>: {} ms", total)
        debug("<b>average</b>: {:.2f}, <b>min</b>: {}, <b>max</b>: {}", average, min, max)
    }

    list(b = "", a = "") {
        debug("{}:", this.title)
        if (not b) {
            for i, name in this.sequencer
                debug("    {}: {} samples", name, this.probePoints[name].length())
            t1 := this.probePoints[this.sequencer[1]][1].tval
            t2 := this.lastProbe[this.lastProbe.length()].tval
            debug("Total time: {} ms", t2 - t1)
            return
        }

        (not a) ? a := this.prev(b)
        durations := []
        if (this.probePoints[a].length() == this.probePoints[b].length()) {
            for i, t in this.probePoints[b]
                durations.push({"label": t.label, "tval": t.tval - this.probePoints[a][i].tval})
            debug("Time between <b>'{}'</b> and <b>'{}'</b>:", a, b)
        } else {
            for i, t in this.probePoints[b]
                (i > 1) ? durations.push({"tval": t.tval - this.probePoints[b][i - 1].tval})
            debug("Time interval of <b>'{}'</b>:", b)
        }

        this.analyze(durations)
    }

    prev(name) {
        for i, n in this.sequencer
            if (name == n)
                return this.sequencer[A_Index - 1]
    }

    next(name) {
        for i, n in this.sequencer
            if (name == n)
                return this.sequencer[A_Index + 1]
    }

    reset() {
        this.__init()
    }
}

objdump(obj, prefix = "", depth = 0) {
    if (Not IsObject(obj)) {
        debug("Not an object")
        return
    }

    baseClasses := ""
    base := obj.base
    loop {
        if (Not base)
            break
        baseClasses .= " -> " (ObjRawGet(base, "__Class") ? base.__Class : Format("{:#x}", &base))
        base := base.base
    }

    if (Not prefix)
        debug("<b>{:#x}{}:</b>", &obj, baseClasses)
    for k, v in obj {
        try {
            if (IsObject(v)) {
                if (IsFunc(v))
                    debug("    {}<b>{}()</b>", prefix, k)
                else if (v.Length() > 0)
                    debug("    {}<b>{}[{}]</b>", prefix, k, v.Length())
                else
                    debug("   {}*<b>{}</b>", prefix, k)
            } else {
                debug("    {}<b>{}</b>, {}", prefix, k, v)
            }
        } catch {}

        if (depth > 0 && IsObject(v))
            objdump(v, prefix "    ", depth - 1)
    }
}

onLog(message) {
    trace(StrGet(message))
}

openConsole() {
    con := new Console()
    con.show()
}

openInspector() {
    inspector := new IngameUIInspector()
    inspector.show()
}

listInventoryItems() {
    debug(_("Inventory:"))
    items := ptask.inventory.getItems()
    if (items.Count() == 0) {
        debug("    " _("No items"))
        Return
    }

    for i, item in items {
        if (item.rarity > 1)
            debug("    {:2d}. {} <span style='color: grey;'>{}</span>", item.index, item.name, item.baseName)
        else
            debug("    {:2d}. {}", item.index, item.name)
    }
}

listInventorySlots() {
    debug(_("Inventory slots:"))
    for i, slot in ptask.inventories
        debug("    {:2d}. {}, {}, {}", slot.id, slot.rows, slot.cols, slot.items.Count())
}

listStashTabItems() {
    debug(_("Stash tab name:") " {}", ptask.stash.Tab.name)
    for i, item in ptask.stash.Tab.getItems() {
        price := $(item)
        
        if (item.stackSize > 0)
            itemInfo := Format("{} ({}/{})", item.name, item.stackCount, item.stackSize)
        else if (item.isGem)
            itemInfo := Format("{} {}/{}", item.name, item.level, item.quality)
        else if (item.rarity > 1 && item.isIdentified)
            itemInfo := item.name " " item.baseName
        else
            itemInfo := item.name

        if (price >= 10)
            debug("    {:3d}. <b style='color: red;'>{:-40s} {:.f}</b>", item.index, itemInfo, price)
        else if (price >= 1)
            debug("    {:3d}. {:-40s} {:g}", item.index, itemInfo, Round(price, 1))
        else if (Not price)
            debug("    {:3d}. <span style='color: grey;'>{:-40s}</span>", item.index, itemInfo)
        else
            debug("    {:3d}. <span style='color: grey;'>{:-40s} {:g}</span>", item.index, itemInfo, Round(price, 2))
    }
}

listStashTabs() {
    stashTabs := ptask.getStashTabs()
    if (stashTabs.Count() == 0) {
        debug(_("No stash tabs"))
        Return
    }

    debug(_("Stash tabs:"))
    for i, tab in stashTabs {
        debug("    {:2d}. {:2d}, <b>{:-32s}</b>, {:-#4x}, {:-#4x}, {:#x}", i
              , tab.index
              , tab.name
              , tab.type
              , tab.flags
              , tab.affinities)
        for j, t in tab.tabs
            debug("    {:2d}.{:d}.  {:2d}, {:-29s}, {:-#4x}, {:-#4x}, {:#x}", i, j
                  , t.index
                  , t.name
                  , t.type
                  , t.flags
                  , t.affinities)
    }
}

listVendorServices() {
    vendor := ptask.getVendor()
    if (Not vendor.name) {
        debug(_("No vendor selected."))
        return
    }

    debug(_("{}'s services:"), vendor.name)
    for name in vendor.getServices()
        debug("    {}. {}", A_Index, name)
}

listFlasks() {
    if (ptask.player.flasks.Count() == 0) {
        debug(_("No flasks"))
        Return
    }

    debug(_("Flasks:"))
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
    debug(_("Flask slot:"))
    if (ptask.inventories[12].Count() == 0) {
        debug("    " _("No items"))
        Return
    }

    for i, item in ptask.inventories[12].items
        debug("    {}. {}", item.index, item.name)
}

listBuffs() {
    buffs := ptask.getBuffs()
    if (buffs.Count() == 0) {
        debug("No buffs.")
        return
    }

    debug("Buffs:")
    for i, b in buffs {
        debug("    {:2d}. {:4.2f} {:4.2f} {:-4d} {}", i, b.duration, b.timer, b.charges, b.name)
    }
}

listEntities() {
    debug("All entities:")
    for i, e in ptask.getEntities("") {
        debug("{:4x}. <b>{}</b>, {}", e.id, e.name, e.path)
    }
}
