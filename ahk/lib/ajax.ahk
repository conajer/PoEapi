;
; ajax.ahk, 5/29/2021 5:08 PM
;

global JSON := new JSON()

ajax(url, method = "GET", data = "", contentType = "application/json") {
    static xmlhttp := ComObjCreate("MSXML2.XMLHTTP")

    xmlhttp.open(method, url, true)
    if (method == "POST")
        xmlhttp.setRequestHeader("Content-Type", contentType "; charset=UTF-8")
    xmlhttp.send(IsObject(data) ? JSON.stringify(data) : data)
    while (xmlhttp.readyState != 4)
        Sleep, 50

    responseType := xmlhttp.getResponseHeader("Content-Type")
    if (responseType ~= "html|xml")
        return xmlhttp.responseXML
    return xmlhttp.responseText
}

class JSON {

    __new() {
        this.document := ComObjCreate("HTMLFile")
        this.document.write("
        (%
        <!DOCTYPE html>
        <html>
        <meta http-equiv='X-UA-Compatible' content='IE=edge'>
        <head>
            <script>
                function copy(src, dst) {
                    if (typeof src !== 'object')
                        return src;

                    let isArray = Array.isArray(src);
                    for (var k in src) {
                        if (src.hasOwnProperty(k))
                            assign(dst, isArray ? k + 1 : k, src[k]);
                    }

                    return dst;
                }
            </script>
        </head>
        </html>
        )")

        this.document.parentWindow.assign := ObjBindMethod(this, "__assign")
    }

    parse(text) {
        parsed := this.document.parentWindow.JSON.parse(text)
        return this.__copy(parsed)
    }

    stringify(value, prefix = "") {
        if (Not IsObject(value))
            return this.document.parentWindow.JSON.stringify(value)

        if (ComObjType(value))
            return "{}"

        if (value.Length() > 0) {
            result .= "[`n"
            for i, v in value {
                if (A_Index > 1)
                    result .= ",`n"
                result .= prefix "  " this.stringify(v, prefix "  ")
            }
            return result "`n" prefix "]"
        } else {
            if (Not value.Count())
                return "{}"

            result .= "{`n"
            for k, v in value {
                if (A_Index > 1)
                    result .= ",`n"
                result .= prefix "  """ k """: " this.stringify(v, prefix "  ")
            }

            return result "`n" prefix "}"
        }
    }

    load(filename) {
        file := FileOpen(filename, "r")
        text := file.Read()
        file.Close()

        return this.parse(text)
    }

    __parse(text) {
        return this.document.parentWindow.JSON.parse(text)
    }

    __assign(obj, key, value) {
        obj[key] := IsObject(value) ? this.__copy(value) : value
    }

    __copy(src) {
        return this.document.parentWindow.copy(src, {})
    }
}
