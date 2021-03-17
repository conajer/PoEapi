;
; Hud.ahk, 2/28/2021 2:57 PM
;

class Hud extends AhkGui {

    __new() {
        base.__new("", "+AlwaysOnTop +Toolwindow +E0x20 -Caption -Resize")
        Gui, Color, 0
        WinSet, TransColor, 0
    }

    clear() {
        VarSetCapacity(r, 16)
        DllCall("GetClientRect", "uint", this.Hwnd, "ptr", &r)
        DllCall("InvalidateRect", "uint", this.Hwnd, "ptr", &r, "int", true)
        DllCall("UpdateWindow", "uint", this.Hwnd)
    }

    show(options = "") {
        if (options ~= "Hide") {
            base.show(options)
            return
        }

        base.show(options " NoActivate")
        r := ptask.getClientRect()
        WinMove, % "ahk_id " this.Hwnd,, r.l, r.t, r.w, r.h
        ptask.bindHud(this.Hwnd)
    }
}
