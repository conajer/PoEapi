;
; ajax.ahk, 5/29/2021 5:08 PM
;

global js := new JScriptHost()

ajax(url, method = "GET", data = "", contentType = "application/json") {
    xmlhttp := ComObjCreate("MSXML2.XMLHTTP")
    xmlhttp.open(method, url, true)
    if (method == "POST")
        xmlhttp.setRequestHeader("Content-Type", contentType "; charset=UTF-8")
    xmlhttp.send(IsObject(data) ? JSON.stringify(data) : data)
    while (xmlhttp.readyState != 4)
        Sleep, 10

    responseType := xmlhttp.getResponseHeader("Content-Type")
    if (responseType ~= "html|xml")
        return xmlhttp.responseXML
    return xmlhttp.responseText
}

class JScriptHost {

    __new() {
        try {
            ObjRawSet(this, "__doc", ComObjCreate("HTMLFile"))
            this.__doc.write("
            (
                <!DOCTYPE html>
                <html>
                <meta http-equiv='X-UA-Compatible' content='IE=edge'>
                <head>
                </head>
                </html>
            )")
            ObjRawSet(this, "__js", this.__doc.parentWindow)
        } catch {
            ObjRawSet(this, "__js", {})
        }
    }

    __Get(name) {
        if (!ObjHasKey(this, name))
            return this.__js[name]
    }

    __Set(name, value) {
        if (!ObjHasKey(this, name))
            return this.__js[name] := value
    }

    __Call(name, args*) {
        if (!ObjHasKey(this, name))
            return this.__js[name].(args*)
    }
}

class JSON {

    static __json := JSON.__init()

    __init() {
        this.__js := new JScriptHost()
        this.__js.eval("
        (
            JSON.copy = function (dst, src) {
                if (typeof src !== 'object')
                    return src;

                if (Array.isArray(src)) {
                    for (let i = 0; i < src.length; ++i)
                        (typeof src[i] === 'object')
                            ? assign(dst, i + 1, src[i]) : dst[i + 1] = src[i];
                } else {
                    for (let k in src) {
                        (typeof src[k] === 'object')
                            ? assign(dst, k, src[k]) : dst[k] = src[k];
                    }
                }

                return dst;
            }
        )")
        this.__js.assign := ObjBindMethod(this, "__assign")
        return this.__js.JSON
    }

    parse(text) {
        return this.__json.copy({}, this.__parse(text))
    }

    stringify(value, space = "") {
        return this.__stringify(value, space, space ? "`n" : "")
    }

    load(filename) {
        FileRead, text, %filename%
        return this.parse(text)
    }

    __parse(text) {
        try {
            return text ? this.__json.parse(text) : ""
        } catch {
            return ""
        }
    }

    __stringify(value, space = "", eol = "", padding = "") {
        if (!IsObject(value))
            return this.__json.stringify(value)

        if (ComObjType(value))
            return "{}"

        margin := padding space
        if (value.Length() > 0) {
            nested := false
            result .= "["
            for i, v in value {
                if (IsObject(v)) {
                    nested := true
                    _eol := eol
                    if (A_Index > 1)
                        result .= eol ? "," eol : ", "
                    else
                        result .= eol
                    
                    result .= margin this.__stringify(v, space, eol, margin)
                } else {
                    if (A_Index > 1)
                        result .= _eol ? "," eol margin : ", "
                    _eol := ""
                    result .= this.__stringify(v, space, eol, margin)
                }
            }

            if (nested)
                return result eol padding "]"
            else
                return result "]"
        } else {
            result .= "{" eol
            for k, v in value {
                if (A_Index > 1)
                    result .= eol ? "," eol : ", "
                result .= margin """" k """: " this.__stringify(v, space, eol, margin)
            }

            return result eol padding "}"
        }
    }

    __assign(obj, key, value) {
        obj[key] := IsObject(value) ? this.__json.copy({}, value) : value
    }

    __copy(obj) {
        return this.__json.copy({}, obj)
    }
}
