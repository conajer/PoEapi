; ahkpp.ahk, 9/6/2020 5:28 PM

global __ahkpp_classes := {"AhkObj" : AhkObj}
global __ahkpp_value := ""

ahkpp_new(className) {
    obj := DllCall("poeapi\ahkpp_new", "Ptr", 0, "Str", className, "Ptr")
    return Object(obj)
}

ahkpp_register_class(classObj) {
    if (Not IsObject(classObj) || Not classObj.__Class)
        return false

    __ahkpp_classes[classObj.__Class] := classObj
    return true
}

class AhkObj {

    __New() {
        DllCall("poeapi\ahkpp_new", "Ptr", &this, "Str", this.__Class)
    }

    __Get(key) {
        if (key != "__properties" && key != "__methods") {
            if (this.__properties.HasKey(key)) {
                valType := this.__properties[key]
                value := DllCall("poeapi\ahkpp_get", "Ptr", &this, "Str", key, valType)
                if (valType == "UPtr") {
                    if (value)
                        return Object(value)

                    value := {}
                    DllCall("poeapi\ahkpp_set", "Ptr", &this, "Str", key, "UPtr", Object(value))
                }

                return value
            } else if (this.__methods.HasKey(key)) {
                if (this.__methods[key].Count() == 1) {
                    retType := this.__methods[key][1]
                    result := DllCall("poeapi\ahkpp_call", "Ptr", &this, "Str", key, retType)
                    return (retType == "UPtr") ? Object(result) : result
                }
            }
        }
    }

    __Set(key, value) {
        if (this.__properties.HasKey(key)) {
            valType := this.__properties[key]
            if (valType == "UPtr") {
                value := Object(value)
            }

            return DllCall("poeapi\ahkpp_set", "Ptr", &this, "Str", key, valType, value)
        }
    }
    
    __Call(name, params*) {
        if (this.__methods.HasKey(name)) {
            T := this.__methods[name]
            if (params.Count() != T.Count() - 1) {
                MsgBox, % name ": invalid number of parameters, should be " T.Count() - 1 " parameters."
                return
            }

            switch params.Count() {
            case 0: result := DllCall("poeapi\ahkpp_call", "Ptr", &this, "Str", name, T[1])
            case 1: result := DllCall("poeapi\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2])
            case 2: result := DllCall("poeapi\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3])
            case 3: result := DllCall("poeapi\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4])
            case 4: result := DllCall("poeapi\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4], params[4], T[5])
            case 5: result := DllCall("poeapi\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4], params[4], T[5], params[5], T[6])
            case 6: result := DllCall("poeapi\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4], params[4], T[5], params[5], T[6], params[6], T[7])
            case 7: result := DllCall("poeapi\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4], params[4], T[5], params[5], T[6], params[6], T[7], params[7], T[8])
            case 8: result := DllCall("poeapi\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4], params[4], T[5], params[5], T[6], params[6], T[7], params[7], T[8], params[8], T[9])
            }
            return (T[params.Count() + 1] == "UPtr") ? Object(result) : result
        }
    }

    __Delete() {
        DllCall("poeapi\ahkpp_delete", "Ptr", &this)
    }
}

DllCall("poeapi\ahkpp_set_callbacks"
    , "Ptr", RegisterCallback("__New")
    , "Ptr", RegisterCallback("__Delete", "F")
    , "Ptr", RegisterCallback("__Get", "F")
    , "Ptr", RegisterCallback("__Set", "F")
    , "Ptr", RegisterCallback("__Call", "F"))

__New(className, baseClassName) {
    className := StrGet(className)
    obj := {}
    if (className != "") {
        baseClassName := StrGet(baseClassName)
        if (baseClassName != "") {
            if (__ahkpp_classes[className]) {
                obj := __ahkpp_classes[className]
                if (obj.base.__Class != baseClassName)
                    MsgBox, % className ": conflict base class: " obj.__Class ", " baseClassName
            } else {
                if (__ahkpp_classes[baseClassName]) {
                    obj.base := __ahkpp_classes[baseClassName]
                    obj.__Class := className
                    __ahkpp_classes[className] := obj
                }
            }
        } else if (__ahkpp_classes[className]) {
            obj := new __ahkpp_classes[className]
        }
    }

    return Object(obj)
}

__Delete(obj) {
    ObjRelease(obj)
}

__Get(obj, key) {
    __ahkpp_value := Object(obj)[StrGet(key)]
    if (IsObject(__ahkpp_value))
        return Object(__ahkpp_value)
    else if (__ahkpp_value)
        return &__ahkpp_value

    return 0
}

__Set(obj, key, params*) {
    obj := Object(obj)
    offset := 0
    params := NumGet(params + offset, "Ptr")

    while (key) {
        key := StrGet(key)
        if (key == "")
            key := obj.Count() + 1

        switch NumGet(params + offset + 8, "Int") {
        case 1: ; Char
            obj[key] := NumGet(params + offset, "Char")
        case 2: ; Short
            obj[key] := NumGet(params + offset, "Short")
        case 3: ; Int
            obj[key] := NumGet(params + offset, "Int")
        case 4: ; Unsigned Char
            obj[key] := NumGet(params + offset, "UChar")
        case 5: ; Unsigned Short
            obj[key] := NumGet(params + offset, "UShort")
        case 6: ; Unsigned Int
            obj[key] := NumGet(params + offset, "UInt")
        case 7: ; Int64
            obj[key] := NumGet(params + offset, "Int64")
        case 8: ; String
            obj[key] := StrGet(NumGet(params + offset, "Ptr"), "utf-8")
        case 9: ; Unicode  String
            obj[key] := StrGet(NumGet(params + offset, "Ptr"))
        case 10: ; Float
            obj[key] := NumGet(params + offset, "Float")
        case 11: ; Double
            obj[key] := NumGet(params + offset, "Double")
        case 12: ; Ptr
            obj[key] := NumGet(params + offset, "Ptr")
        case 13: ; Object
            objPtr := NumGet(params + offset, "Ptr")
            obj[key] := objPtr ? Object(objPtr) : {}
        }

        key := NumGet(params + offset + 16, "Ptr")
        offset += 24
    }
}

__Call(obj, name, params*) {
    offset := 0
    params := NumGet(params + offset, "Ptr")
    args := []

    type := NumGet(params + 0, "Int")
    while (type) {
        offset += 8
        switch type {
        case 1: ; Char
            args.Push(NumGet(params + offset, "Char"))
        case 2: ; Short
            args.Push(NumGet(params + offset, "Short"))
        case 3: ; Int
            args.Push(NumGet(params + offset, "Int"))
        case 4: ; Unsigned Char
            args.Push(NumGet(params + offset, "UChar"))
        case 5: ; Unsigned Short
            args.Push(NumGet(params + offset, "UShort"))
        case 6: ; Unsigned Int
            args.Push(NumGet(params + offset, "UInt"))
        case 7: ; Int64
            args.Push(NumGet(params + offset, "Int64"))
        case 8: ; String
            args.Push(StrGet(NumGet(params + offset, "Ptr"), "utf-8"))
        case 9: ; Unicode  String
            args.Push(StrGet(NumGet(params + offset, "Ptr")))
        case 10: ; Float
            args.Push(NumGet(params + offset, "Double"))
        case 11: ; Double
            args.Push(NumGet(params + offset, "Double"))
        case 12: ; Ptr
            args.Push(NumGet(params + offset, "Ptr"))
        case 13: ; Object
            objPtr := NumGet(params + offset, "Ptr")
            args.Push(objPtr ? Object(objPtr) : "")
        }

        offset += 8
        type := NumGet(params + offset, "Int")
    }

    Object(obj)[StrGet(name)](args*)
}
