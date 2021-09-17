;
; curl.ahk, 8/30/2021 12:41 AM
;

class curl extends AhkObj {

    static __options := ""
    static __stypes := ["* ", "< ", "> ", "{ ", "} ", "{ ", "} "]

    __new() {
        base.__new()
        this.setRequestHeader("user-agent", "curl/" this.version)

        if (Not curl.__options) {
            curl.__options := this.getAllOptions()
        }
    }

    setopt(name, data) {
        opt := this.__options[name]
        if (opt.type == 4) ; char* to zero terminated buffer
            StrPut(Trim(data), &data, "utf-8")
        return this.__setopt(opt.id, (opt.type < 3) ? data : &data)
    }

    ajax(url, method = "GET", data = "", contentType = "application/json") {
        static __curl = ""

        (not __curl) ? __curl := new curl()
        (method = "POST") ? __curl.post(url, data, contentType) : __curl.get(url)
        return __curl.responseText
    }

    __debug(type, data, size) {
        if (type < 3) {
            loop, parse, % StrGet(data, size - 1, "utf-8"), `n, `r
                debug(this.__stypes[type + 1] " " A_LoopField)
        }
    }
}
