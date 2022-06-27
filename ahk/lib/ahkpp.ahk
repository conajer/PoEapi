; ahkpp.ahk, 9/6/2020 5:28 PM

global __ahkpp_classes := {"AhkObj" : AhkObj}
global __ahkpp_types := [ "Char"          ; AhkChar      
                         ,"Short"         ; AhkShort     
                         ,"Int"           ; AhkInt       
                         ,"UChar"         ; AhkUChar     
                         ,"UShort"        ; AhkUShort    
                         ,"UInt"          ; AhkUInt      
                         ,"Int64"         ; AhkInt64     
                         ,"AStr"          ; AhkString    
                         ,"WStr"          ; AhkWString   
                         ,"Double"        ; AhkFloat     
                         ,"Double"        ; AhkDouble    
                         ,"Ptr"           ; AhkPointer   
                         ,"UPtr"          ; AhkObject    
                         ,"AStr"          ; AhkStringPtr 
                         ,"WStr"          ; AhkWStringPtr
                         ,"Char" ]        ; AhkBool      
global __ahkpp_value := ""
global ahkpp_new, ahkpp_get, ahkpp_set, ahkpp_call, ahkpp_set_callbacks

ahkpp_init(pLib) {
    for i, name in ["new", "get", "set", "call", "set_callbacks"]
        ahkpp_%name% := DllCall("GetProcAddress", "Ptr", pLib, "AStr", "ahkpp_" name, "Ptr")

    DllCall(ahkpp_set_callbacks
        , "Ptr", RegisterCallback("__New")
        , "Ptr", RegisterCallback("__Delete", "F")
        , "Ptr", RegisterCallback("__Get", "F")
        , "Ptr", RegisterCallback("__Set", "F")
        , "Ptr", RegisterCallback("__Call", "F"))
}

ahkpp_register_class(classObj) {
    if (Not IsObject(classObj) || Not classObj.__Class)
        return false

    __ahkpp_classes[classObj.__Class] := classObj
    return true
}

class AhkObj {

    __New() {
        DllCall(ahkpp_new, "Ptr", Object(this), "Str", this.__Class)
        this.__self := &this
    }

    __Get(key, key2 = "") {
        __properties := ObjRawGet(this, "__properties")
        __methods := ObjRawGet(this, "__methods")

        if (valType := __properties[key]) {
            value := DllCall(ahkpp_get, "Ptr", this.__self, "Str", key, valType)
            if (valType == "UPtr") {
                value := Object(value)
                if (Not value) {
                    value := {}
                    DllCall(ahkpp_set, "Ptr", this.__self, "Str", key, "UPtr", Object(value))
                }
            }
        } else if (m := __methods[key]) {
            if (m.params.Length() == 0) {
                result := DllCall(ahkpp_call, "Ptr", this.__self, "Str", key, m.returnType)
                value := (m.returnType == "UPtr") ? Object(result) : result
            }
        } else if (m := __methods["get" key]) {
            if (m.params.Length() == 0) {
                result := DllCall(ahkpp_call, "Ptr", this.__self, "Str", "get" key, m.returnType)
                value := (m.returnType == "UPtr") ? Object(result) : result
            }
        }

        if (value != "")
            return key2 ? value[key2] : value
    }

    __Set(key, value) {
        if (valType := this.__properties[key]) {
            if (valType == "UPtr") {
                value := Object(value)
            }

            return DllCall(ahkpp_set, "Ptr", this.__self, "Str", key, valType, value)
        } else if (this.__methods["set" key]) {
            name := "set" key
            if (this.__methods[name].params.Length() == 1) {
                valType := this.__methods[name].params[1]
                DllCall(ahkpp_call, "Ptr", this.__self, "Str", name, valType, value)

                return value
            }
        }
    }

    __Call(name, params*) {
        if (m := this.__methods[name]) {
            if (m.params.Length() != params.Length()) {
                MsgBox, % this.__Class "." name "(): invalid number of parameters, should be " m.params.Length() " parameters."
                return
            }

            args := ["Ptr", this.__self, "Str", name]
            for i, p in m.params
                args.Push(p, params[A_Index])
            args.Push(m.returnType)
            result := DllCall(ahkpp_call, args*)

            return (m.returnType == "UPtr") ? Object(result) : result
        }
    }
}

__New(className, baseClassName) {
    className := StrGet(className)
    obj := {}
    if (className != "") {
        baseClassName := StrGet(baseClassName)
        if (baseClassName != "") {
            if (__ahkpp_classes[className]) {
                obj := __ahkpp_classes[className]
            } else {
                if (__ahkpp_classes[baseClassName]) {
                    obj.base := __ahkpp_classes[baseClassName]
                    obj.__Class := className
                    __ahkpp_classes[className] := obj
                }
            }
        } else if (className != "AhkObj") {
            if (__ahkpp_classes[className]) {
                obj.base := __ahkpp_classes[className]
                obj.__Init()
                obj.__self := &obj
            } else if (IsObject(%className%)) {
                obj.base := %className%
            } else {
                obj.base := {"__Class": className}
            }
        }
    }

    return Object(obj)
}

__Delete(obj) {
    ObjRelease(obj)
}

__Get(obj, key) {
    obj := Object(obj)
    key := StrGet(key)
    __ahkpp_value := obj ? obj[key] : %key%
    if (IsObject(__ahkpp_value))
        return Object(__ahkpp_value)
    else if (__ahkpp_value)
        return &__ahkpp_value
}

__Set(obj, key, params*) {
    obj := Object(obj)
    offset := 0
    params := NumGet(params + offset, "Ptr")

    while (key) {
        key := StrGet(key)
        if (key == "")
            key := obj.Length() + 1

        type := NumGet(params + offset + 8, "Int")
        switch type {
        case 8: ; String
            value := StrGet(NumGet(params + offset, "Ptr"), "utf-8")
        case 9: ; Unicode  String
            value := StrGet(NumGet(params + offset, "Ptr"))
        case 12: ; Pointer
            ptr := NumGet(params + offset, "Ptr")
            value := Format("{:#x}", ptr)
        case 13: ; Object
            objPtr := NumGet(params + offset, "Ptr")
            value := objPtr ? Object(objPtr) : {}
        default:
            value := NumGet(params + offset, __ahkpp_types[type])
        }

        if (obj)
            obj[key] := value
        else
            %key% := value

        key := NumGet(params + offset + 16, "Ptr")
        offset += 24
    }
}

__Call(obj, name, params*) {
    params := NumGet(params + 0, "Ptr")
    args := []

    type := NumGet(params + 0, "Int")
    offset := 8
    while (type) {
        switch type {
        case 8: ; String
            args.Push(StrGet(NumGet(params + offset, "Ptr"), "utf-8"))
        case 9: ; Unicode  String
            args.Push(StrGet(NumGet(params + offset, "Ptr")))
        case 13: ; Object
            objPtr := NumGet(params + offset, "Ptr")
            args.Push(objPtr ? Object(objPtr) : "")
        default:
            args.Push(NumGet(params + offset, __ahkpp_types[type]))
        }

        type := NumGet(params + offset + 8, "Int")
        offset += 16
    }

    Object(obj)[StrGet(name)](args*)
}
