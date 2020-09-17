;
; canvas.ahk, 9/14/2020 8:50 AM
;
; Transparent canvas window overlapped on the Path of Exile window.
;

Class Canvas {

    __bgColor := EEAA99

    __new(ownerHwnd) {
        WinGetPos, x, y, w, h, ahk_id %ownerHwnd%
        Gui, __canvas:New, +Owner%ownerHwnd% +HwndHwnd -Caption +LastFound
        Gui, __canvas:Color, __bgColor
        Gui, __canvas:Show
        WinSet, TransColor, __bgColor
        WinMove,,, x, y, w, h

        this.Hwnd := Hwnd
        this.Hdc := DllCall("GetDC", "UInt", Hwnd)
        this.Width := w
        this.Height := h

        if (ShowCanvasBorder)
            this.drawRect(0, 0, w, h, 0xc390070)
    }

    drawLine(x1, y1, x2, y2, color = 0x7f7f7f, linewidth = 1) {
        hPen := DllCall("CreatePen", "Int", 0, "Int", linewidth, "Int", color)
        DllCall("SelectObject" , "UInt", this.Hdc , "UInt", hPen)
        this.__drawLine(x1, y1, x2, y2)
        DllCall("DeleteObject" , "UInt", hPen)
    }

    drawRect(x, y, w, h, color = 0x7f7f7f, linewidth = 1) {
        hPen := DllCall("CreatePen", "Int", 0, "Int", linewidth, "Int", color)
        DllCall("SelectObject" , "UInt", this.Hdc , "UInt", hPen)
        this.__drawRect(x, y, w, h)
        DllCall("DeleteObject" , "UInt", hPen)
    }

    drawGrid(x, y, w, h, r, c, color = 0x7f7f7f, linewidth = 1) {
        hPen := DllCall("CreatePen", "Int", 0, "Int", linewidth, "Int", color)
        DllCall("SelectObject" , "UInt", this.Hdc , "UInt", hPen)
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
    }

    clear() {
        cBrush := DllCall("gdi32.dll\CreateSolidBrush", "UInt", __bgColor )
        cRegion := DllCall("gdi32.dll\CreateRectRgn", "Int", 0 , "Int", 0, "Int", this.Width , "Int", this.Height)
        DllCall("gdi32.dll\FillRgn" , "UInt", this.Hdc , "UInt", cRegion , "UInt", cBrush)
        DllCall("DeleteObject" , "UInt", cRegion)
        DllCall("DeleteObject" , "UInt", cBrush)
    }

    __drawLine(x1, y1, x2, y2) {
        DllCall("MoveToEx" , "UInt", this.Hdc , "UInt", x1 , "UInt", y1, "Int", 0)
        DllCall("LineTo" , "UInt", this.Hdc, "UInt", x2, "UInt", y2)
    }

    __drawRect(x, y, w, h) {
        this.__drawLine(x, y, x + w - 1, y)
        this.__drawLine(x + w - 1, y, x + w - 1, y + h - 1)
        this.__drawLine(x + w - 1, y + h - 1, x, y + h - 1)
        this.__drawLine(x, y + h - 1, x, y)
    }
}
