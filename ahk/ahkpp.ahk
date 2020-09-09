; ahkpp.ahk, 9/6/2020 5:28 PM

global __ahkpp_classes := {"AhkObj" : AhkObj}
global __ahkpp_objects := {}
global __ahkpp_value := ""

class AhkObj {
    __New() {
        DllCall("ahkpp\ahkpp_new", "Ptr", &this, "Str", this.__Class)
    }

    __Get(key) {
        if (key != "__properties" && this.__properties.HasKey(key)) {
            valType := this.__properties[key]
            value := DllCall("ahkpp\ahkpp_get", "Ptr", &this, "Str", key, valType)
            if (valType == "UPtr") {
                if (value)
                    return Object(value)

                value := {}
                DllCall("ahkpp\ahkpp_set", "Ptr", &this, "Str", key, "UPtr", Object(value))
            }

            return value
        }
    }

    __Set(key, value) {
        if (this.__properties.HasKey(key)) {
            valType := this.__properties[key]
            if (valType == "UPtr") {
                value := Object(value)
            }

            return DllCall("ahkpp\ahkpp_set", "Ptr", &this, "Str", key, valType, value)
        }
    }
    
    __Call(name, params*) {
        if (this.__methods.HasKey(name)) {
            T := this.__methods[name]
            switch params.Count() {
            case 0: return DllCall("ahkpp\ahkpp_call", "Ptr", &this, "Str", name, T[1])
            case 1: return DllCall("ahkpp\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2])
            case 2: return DllCall("ahkpp\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3])
            case 3: return DllCall("ahkpp\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4])
            case 4: return DllCall("ahkpp\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4], params[4], T[5])
            case 5: return DllCall("ahkpp\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4], params[4], T[5], params[5], T[6])
            case 6: return DllCall("ahkpp\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4], params[4], T[5], params[5], T[6], params[6], T[7])
            case 7: return DllCall("ahkpp\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4], params[4], T[5], params[5], T[6], params[6], T[7], params[7], T[8])
            case 8: return DllCall("ahkpp\ahkpp_call", "Ptr", &this, "Str", name, T[1], params[1], T[2], params[2], T[3], params[3], T[4], params[4], T[5], params[5], T[6], params[6], T[7], params[7], T[8], params[8], T[9])
            }
        }
    }
}

DllCall("ahkpp\ahkpp_set_callbacks"
    , "Ptr", RegisterCallback("__New")
    , "Ptr", RegisterCallback("__Delete", "F")
    , "Ptr", RegisterCallback("__Get", "F")
    , "Ptr", RegisterCallback("__Set", "F")
    , "Ptr", RegisterCallback("__Call", "F"))

__New(className, baseName) {
    className := StrGet(className)
    if (Not __ahkpp_classes[className]) {
        baseName := StrGet(baseName)
        if (Not __ahkpp_classes[baseName])
            obj := {}
        else
            obj := new __ahkpp_classes[baseName]
    } else {
        obj := new __ahkpp_classes[className]
    }
    obj.__Init()
    obj.__New()

    return Object(obj)
}

__Delete(obj) {
    ObjRelease(obj)
}

__Get(obj, key) {
    __ahkpp_value := Object(obj)[StrGet(key)]
    return &__ahkpp_value
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
