;
; canvas.ahk, 9/14/2020 8:50 AM
;
; Transparent canvas window overlapped on the Path of Exile window.
;

Class Canvas extends AhkGui {

    __bgColor := EEAA99

    __new(ownerHwnd) {
        guiID := Format("__canvas{:x}", &this)
        r := this.getClientRect(ownerHwnd)
        WS_EX_TRANSPARENT := 0x20
        Gui, %guiID%:New, +AlwaysOnTop +Toolwindow +HwndHwnd +E%WS_EX_TRANSPARENT% -Caption +LastFound
        Gui, %guiID%:Color, __bgColor
        Gui, %guiID%:Show
        WinSet, TransColor, __bgColor
        WinMove,,, r.l, r.t, r.w, r.h

        this.Hwnd := Hwnd
        this.Hdc := DllCall("GetDC", "UInt", Hwnd)
        this.Cdc := DllCall("CreateCompatibleDC", "UInt", this.Hdc)
        hBitmap := DllCall("CreateCompatibleBitmap", "UInt", this.Hdc, "Int", r.w, "Int", r.h)
        DllCall("SelectObject", "UInt", this.Cdc, "UInt", hBitmap)
        this.updateDC := true
        this.Width := r.w
        this.Height := r.h

        if (ShowCanvasBorder)
            this.drawRect(0, 0, this.Width, this.Height, 0xffbf00)
    }

    destroy() {
        guiID := Format("__canvas{:x}", &this)
        Gui, %guiID%:Destroy
    }

    beginPaint() {
        this.updateDC := false
    }

    endPaint() {
        DllCall("BitBlt", "UInt", this.Hdc, "Int", 0, "Int", 0, "Int", this.Width, "Int", this.Height
                , "UInt", this.Cdc, "Int", 0, "Int", 0, "Int", 0xcc0020)
        this.updateDC := true
    }

    getClientRect(hwnd) {
        VarSetCapacity(r, 16)
        DllCall("GetClientRect", "UInt", hwnd, "UInt", &r)
        DllCall("ClientToScreen", "UInt", hwnd, "UInt", &r)

        left := NumGet(r, 0, "Int")
        top := NumGet(r, 4, "Int")
        width := NumGet(r, 8, "Int")
        height := NumGet(r, 12, "Int")

        return new Rect(left, top, width, height)
    }

    drawLine(x1, y1, x2, y2, color = 0x7f7f7f, linewidth = 1) {
        hPen := DllCall("CreatePen", "Int", 0, "Int", linewidth, "Int", color)
        DllCall("SelectObject" , "UInt", this.Cdc , "UInt", hPen)
        this.__drawLine(x1, y1, x2, y2)
        DllCall("DeleteObject" , "UInt", hPen)

        if (this.updateDC)
            DllCall("BitBlt", "UInt", this.Hdc, "Int", x, "Int", y, "Int", w, "Int", h
                    , "UInt", this.Cdc, "Int", x, "Int", y, "Int", 0xcc0020)
    }

    drawRect(x, y, w, h, color = 0x7f7f7f, linewidth = 1) {
        hPen := DllCall("CreatePen", "Int", 0, "Int", linewidth, "Int", color)
        DllCall("SelectObject" , "UInt", this.Cdc , "UInt", hPen)
        this.__drawRect(x, y, w, h)
        DllCall("DeleteObject" , "UInt", hPen)

        if (this.updateDC)
            DllCall("BitBlt", "UInt", this.Hdc, "Int", x, "Int", y, "Int", w, "Int", h
                    , "UInt", this.Cdc, "Int", x, "Int", y, "Int", 0xcc0020)
    }

    drawGrid(x, y, w, h, r, c, color = 0x7f7f7f, linewidth = 1) {
        hPen := DllCall("CreatePen", "Int", 0, "Int", linewidth, "Int", color)
        DllCall("SelectObject", "UInt", this.Cdc , "UInt", hPen)
        this.__drawRect(x, y, w, h)

        x1 := x + w
        loop % r {
            y1 := y + A_Index * (h/r) - 1
            this.__drawLine(x, y1, x1, y1)
        }

        y1 := y + h
        loop % c {
            x1 := x + A_Index * (w/c) - 1
            this.__drawLine(x1, y, x1, y1)
        }
        DllCall("DeleteObject" , "UInt", hPen)

        if (this.updateDC)
            DllCall("BitBlt", "UInt", this.Hdc, "Int", x, "Int", y, "Int", w, "Int", h
                    , "UInt", this.Cdc, "Int", x, "Int", y, "Int", 0xcc0020)
    }

    drawText(x, y, w, h, s, color = 0x7f7f7f, linewidth = 1) {
        DllCall("SetTextColor", "UInt", this.Cdc , "UInt", color)
        DllCall("SetBkMode", "UInt", this.Cdc , "UInt", 2)
        VarSetCapacity(r, 16)
        NumPut(x, r, 0x0, "Int")
        NumPut(y, r, 0x4, "Int")
        NumPut(x + w, r, 0x8, "Int")
        NumPut(y + h, r, 0xc, "Int")
        r := DllCall("DrawText", "UInt", this.Cdc , "Str", s, "Int", -1, "Ptr", &r, "Int", 0x120)

        if (this.updateDC)
            DllCall("BitBlt", "UInt", this.Hdc, "Int", x, "Int", y, "Int", w, "Int", h
                    , "UInt", this.Cdc, "Int", x, "Int", y, "Int", 0xcc0020)
    }

    clear() {
        cBrush := DllCall("gdi32.dll\CreateSolidBrush", "UInt", __bgColor )
        cRegion := DllCall("gdi32.dll\CreateRectRgn", "Int", 0 , "Int", 0, "Int", this.Width , "Int", this.Height)
        DllCall("gdi32.dll\FillRgn" , "UInt", this.Hdc , "UInt", cRegion , "UInt", cBrush)
        DllCall("DeleteObject" , "UInt", cRegion)
        DllCall("DeleteObject" , "UInt", cBrush)
    }

    __drawLine(x1, y1, x2, y2) {
        DllCall("MoveToEx" , "UInt", this.Cdc , "UInt", x1 , "UInt", y1, "Int", 0)
        DllCall("LineTo" , "UInt", this.Cdc, "UInt", x2, "UInt", y2)
    }

    __drawRect(x, y, w, h) {
        this.__drawLine(x, y, x + w - 1, y)
        this.__drawLine(x + w - 1, y, x + w - 1, y + h - 1)
        this.__drawLine(x + w - 1, y + h - 1, x, y + h - 1)
        this.__drawLine(x, y + h - 1, x, y)
    }
}
