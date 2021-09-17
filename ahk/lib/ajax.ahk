;
; ajax.ahk, 5/29/2021 5:08 PM
;

ajax(url, method = "GET", data = "", contentType = "application/json") {
    static xmlhttp := ComObjCreate("MSXML2.XMLHTTP")

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

class JSON {

    static __json := JSON.__init()

    __init() {
        this.document := ComObjCreate("HTMLFile")
        this.document.write("
        (%
        <!DOCTYPE html>
        <html>
        <meta http-equiv='X-UA-Compatible' content='IE=edge'>
        <head>
            <script>
                JSON.copy = function (dst, src) {
                    if (typeof src !== 'object')
                        return src;

                    if (Array.isArray(src)) {
                        for (let i = 0; i < src.length; ++i)
                            (typeof src[i] === 'object')
                                ? assign(dst, i + 1, src[i]) : dst[i + 1] = src[i];
                    } else {
                        for (let k in src)
                            (typeof src[k] === 'object')
                                ? assign(dst, k, src[k]) : dst[k] = src[k];
                    }

                    return dst;
                }
            </script>
        </head>
        </html>
        )")

        this.document.parentWindow.assign := ObjBindMethod(this, "__assign")
        return this.document.parentWindow.JSON
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

    decodeURI(encodedURI) {
        return this.document.parentWindow.decodeURI(encodedURI)
    }

    encodeURI(uri) {
        return this.document.parentWindow.encodeURI(uri)
    }

    __parse(text) {
        return text ? this.__json.parse(text) : ""
    }

    __stringify(value, space = "", eol = "", padding = "") {
        if (Not IsObject(value))
            return this.__json.stringify(value)

        if (ComObjType(value))
            return "{}"

        margin := padding space
        if (value.Length() > 0) {
            result .= "[" eol
            for i, v in value {
                if (A_Index > 1)
                    result .= "," eol
                result .= margin this.__stringify(v, space, eol, margin)
            }

            return result eol padding "]"
        } else {
            result .= "{" eol
            for k, v in value {
                if (A_Index > 1)
                    result .= "," eol
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
