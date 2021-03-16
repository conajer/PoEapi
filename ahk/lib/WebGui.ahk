;
; WebGui.ahk, 2/14/2021 11:32 PM
;

class AhkGui {

    __new(title = "", options = "") {
        title := title ? title : this.__Class
        Gui, New, +Resize -DPIScale +LastFound +HwndHwnd %options%, %title%
        this.hwnd := Hwnd

        this.onMessage(0x005, "__onSize")
        this.onMessage(0x010, "__onClose")
        this.onMessage(0x112, "__onClose")
    }

    activate() {
        WinActivate, % "ahk_id " this.Hwnd
    }

    close() {
        WinClose, % "ahk_id " this.Hwnd
    }

    show(options = "", title = "") {
        try {
            hwnd := this.Hwnd
            Gui, %hwnd%:Show, %options%, %title%
        } catch {}

        return this
    }

    hide() {
        DllCall("ShowWindow", "UInt", this.Hwnd, "Int", 0)
    }

    onMessage(number, methodName) {
        if (this.__handlers[number])
            OnMessage(number, this.__handlers[msg], 0)

        try {
            handler := ObjBindMethod(this, methodName)
            OnMessage(number, handler)
            this.__handlers[number] := handler
        } catch {}
    }

    __onClose(wParam, lParam, msg, hwnd) {
        if (this.Hwnd == hwnd) {
            if (msg == 0x112 && wParam != 0xf060)
                return

            hwnd := this.Hwnd
            Gui, %hwnd%:Destroy
            for number, handler in this.__handlers
                OnMessage(number, handler, 0)
        }
    }

    __onSize(wParam, lParam, msg, hwnd) {
        if (this.Hwnd == hwnd) {
            VarSetCapacity(r, 16)
            DllCall("GetClientRect", "UInt", hwnd, "Ptr", &r)
            this.onResize(NumGet(r, 0x8, "Int"), NumGet(r, 0xc, "Int"))
            return 0
        }
    }

    __var(name) {
        return "__obj" &this "_" name
    }

    __Get(key) {
        if (Not this.HasKey(key)) {
            var := this.__var(key)
            return (%var%)
        }
    }

    __event_wrapper() {
        global
L1:
        RegExMatch(A_GuiControl, "__obj([0-9]+)_(.*)", matched)
        obj := Object(matched1)
        obj[matched2]()  ; Call object's method
        return
    }
}

class WebGui extends AhkGui {

    __new(title = "", options = "", width = 800, height = 600) {
        global __mshtml

        base.__new(title, options)
        Gui, Margin, 0, 0
        Gui, Add, ActiveX, -Border w%width% h%height% v__mshtml, Shell.Explorer

        this.browser := __mshtml
        this.browser.navigate("about:<meta http-equiv=""X-UA-Compatible"" content=""IE=edge""/>")
        this.browser.silent := true
        this.document := this.browser.document
    }

    bind(id, event = "onclick", handler = "") {
        try {
            element := this.document.getElementById(id)
            if (element) {
                if (handler) {
                    if (IsObject(handler))
                        result := element.attachEvent(event, handler)
                    else if (IsFunc(this[handler]))
                        result := element.attachEvent(event, ObjBindMethod(this, handler))
                } else {
                    if (IsFunc(this[id]))
                        result := element.attachEvent(event, ObjBindMethod(this, id))
                    else
                        result := element.attachEvent(event, ObjBindMethod(this, event))
                }
            }
        } catch {}

        return result == 0
    }

    bindAll(name, event = "onclick") {
        try {
            elements := this.document.getElementsByClassName(name)
            loop, % elements.length {
                elem := elements.item(A_Index - 1)
                if (elem.id && IsFunc(this[elem.id]))
                    elem.attachEvent(event, ObjBindMethod(this, elem.id))
                else
                    elem.attachEvent(event, ObjBindMethod(this, event))
            }
        } catch {}

        try {
            elements := this.document.getElementsByTagName(name)
            loop, % elements.length {
                elem := elements.item(A_Index - 1)
                if (elem.id && IsFunc(this[elem.id]))
                    elem.attachEvent(event, ObjBindMethod(this, elem.id))
                else
                    elem.attachEvent(event, ObjBindMethod(this, event))
            }
        } catch {}
    }

    connect(id, obj = "") {
        try {
            element := this.document.getElementById(id)
            if (element) {
                ComObjConnect(element, obj)
                if (IsObject(obj))
                    obj.__element := element    ; keep a reference of element
            }

            return true
        } catch {
            return false
        }
    }

    open(url) {
        this.browser.navigate(url)
    }

    onResize(width, height) {
        try {
            GuiControl, Move, __mshtml, w%width% h%height%
            this.document.parentWindow.execScript("
            (
                var event = document.createEvent('UIEvents');
                event.initUIEvent('resize', true, false, window, 0);
                window.dispatchEvent(event);
            )")
        } catch {}
    }
}

Class IUnknown {

    __new(obj) {
        this.ptr := (ComObjType(obj) == 9) ? ComObjValue(obj) : obj
    }

    __vtable(index) {
        return NumGet(NumGet(this.ptr + 0), index * A_PtrSize)
    }
}

class IDispatch extends IUnknown {

    getTypeInfoCount() {
        DllCall(this.__vtable(3), "ptr", this.ptr, "uint*", n)
        return n
    }

    getTypeInfo() {
        DllCall(this.__vtable(4), "ptr", this.ptr, "uint", 0, "uint", 0, "ptr*", ptr)
        return new ITypeInfo(ptr)
    }
}

class ITypeInfo extends IUnknown {

    __new(ptr) {
        base.__new(ptr)
        DllCall(this.__vtable(12), "ptr", this.ptr, "int", -1, "ptr*", name, "ptr*", docString, "ptr", 0, "ptr", 0)
        this.name := StrGet(name, "utf-16")
        this.docString := StrGet(docString, "utf-16")
        this.getFuncs()
    }

    getTypeAttr() {
        DllCall(this.__vtable(3), "ptr", this.ptr, "ptr*", typeAttr)
        this.cFuncs := NumGet(typeAttr + 0, 40 + A_PtrSize, "short")
        this.cVars := NumGet(typeAttr + 0, 42 + A_PtrSize, "short")
        this.cImplTypes := NumGet(typeAttr + 0, 44 + A_PtrSize, "short")
        DllCall(this.__vtable(19), "ptr", this.ptr, "ptr", typeAttr)
    }

    getFuncs() {
        this.getTypeAttr()
        loop, % this.cImplTypes {
            DllCall(this.__vtable(8), "ptr", this.ptr, "int", A_Index - 1, "int*", refType)
            DllCall(this.__vtable(14), "ptr", this.ptr, "ptr", refType, "ptr*", ptr)

            implTypeInfo := new ITypeInfo(ptr)
            implTypeInfo.getFuncs()
            this.implTypes[A_Index] := implTypeInfo

            for name, method in implTypeInfo.methods
                this.methods[name] := method, method.inherited .= "*"

            for name, prop in implTypeInfo.properties
                this.properties[name] := prop, prop.inherited .= "*"
        }

        loop, % this.cFuncs {
            DllCall(this.__vtable(5), "ptr", this.ptr, "int", A_Index - 1, "ptr*", funcDesc)
            memid := NumGet(funcDesc + 0, "short")
            invkind := NumGet(funcDesc + 0, 4 + 3 * A_PtrSize, "int")
            cParams := NumGet(funcDesc + 0, 12 + 3 * A_PtrSize, "short")
            cParamsOpt := NumGet(funcDesc + 0, 14 + 3 * A_PtrSize, "short")
            DllCall(this.__vtable(20), "ptr", this.ptr, "ptr", funcDesc)
            DllCall(this.__vtable(12), "ptr", this.ptr, "int", memid, "ptr*", name, "ptr*", docString, "ptr", 0, "ptr", 0)

            name := StrGet(name, "utf-16")
            if (Not name)
                continue

            docString := StrGet(docString, "utf-16")
            if (invkind == 1) {
                this.methods[name] := { "memid"      : memid
                                      , "name"       : name
                                      , "cParams"    : cParams
                                      , "cParamsOpt" : cParamsOpt
                                      , "docString"  : docString }
            } else {
                if (this.properties.HasKey(name))
                    this.properties[name].invkind |= invkind
                else
                    this.properties[name] := {"invkind" : invkind, "docString" : docString}
            }
        }
    }
}

ComObjDump(obj) {
    i := new IDispatch(obj).getTypeInfo()
    if (Not i.ptr) {
        debug("Not a COM object")
        return
    }

    implTypeNames := ""
    implType := i
    loop, {
        if (Not implType)
            break
        implTypeNames .= " -> " implType.name
        implType := implType.implTypes[1]
    }
    debug("{:#x}{}", i.ptr, implTypeNames)

    for name, method in i.methods
        debug("    {:-24} {}   {}", name method.inherited, "Method", method.docString)

    for name, prop in i.properties {
        try {
            debug("    {:-24} {} {} {}", name prop.inherited, "Property", prop.invkind, obj[name])
        } catch {
            debug("    {:-24} {} {}", name prop.inherited, "Property", prop.invkind)
        }
    }
}
