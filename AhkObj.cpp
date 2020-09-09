/*
* AhkObj.cpp, 9/1/2020 10:37 AM
*/

#include <cstdarg>
#include "ahkpp"

using PropertyType = std::tuple<void*, int>;
using MethodType = void* (AhkObj::*)();

static const wchar_t* ahk_types[] = {
    L"Unknown",
    L"Char",
    L"Short",
    L"Int",
    L"UChar",
    L"UShort",
    L"UInt",
    L"Int64",
    L"AStr",
    L"WStr",
    L"Float",
    L"Double",
    L"Ptr",
    L"UPtr",
    0,
};

DLLEXPORT AhkObj::AhkObj() : obj_ref(0) {
}

DLLEXPORT AhkObj::AhkObj(const string& type_name, const wstring& basetype_name, ...)
    : basetype_name(basetype_name), obj_ref(0)
{
    /* type_name comes from typeid(*).name(), a typeid's name has format 'N<typename>'
       for reference when using GCC as compiler. N is length of the typename string. */
    char *actural_type_name;
    int len = strtol(type_name.c_str(), &actural_type_name, 0);
    this->type_name.resize(len);
    std::mbstowcs(&this->type_name[0], actural_type_name, len);
}

DLLEXPORT AhkObj::AhkObj(AhkObjRef* obj_ref) : obj_ref(obj_ref) {
    __init();
    ahkpp_stubs[obj_ref] = this;
}

DLLEXPORT AhkObj::~AhkObj() {
    if (obj_ref) {
        ahkpp_stubs.erase(obj_ref);
        ahk_delete(obj_ref);
    }
}

DLLEXPORT AhkObj::operator AhkObjRef*() {
    if (!obj_ref) {
        if (obj_ref = ahk_new(type_name.c_str(), basetype_name.c_str())) {
            __init();
            ahkpp_stubs[obj_ref] = this;
        }
    }

    return obj_ref;
}

void AhkObj::__new() {
    if (obj_ref && !type_name.empty()) {
        AhkObj* __properties = new AhkObj(ahk_new(L"", L""));
        __set(L"__properties", __properties->obj_ref, AhkObject, 0);
        for (auto p : properties)
            __properties->__set(p.first.c_str(), ahk_types[std::get<1>(p.second)], AhkWString, 0);
        delete __properties;

        AhkObj* __methods = new AhkObj(ahk_new(L"", L""));
        __set(L"__methods", __methods->obj_ref, AhkObject, 0);
        for (auto m : methods) {
            AhkObj* method = new AhkObj(ahk_new(L"", L""));
            __methods->__set(m.first.c_str(), method->obj_ref, AhkObject, 0);
            for (auto t : std::get<2>(m.second))
                method->__set(L"", ahk_types[t], AhkWString, 0);
            method->__set(L"", ahk_types[std::get<1>(m.second)], AhkWString, 0);
            delete method;
        }
        delete __methods;
    }
}

DLLEXPORT void AhkObj::__init() {
}

DLLEXPORT bool AhkObj::add_property(const wstring& key, void* value, AhkType type) {
    properties[key] = std::make_tuple(value, type);
    return true;
}

DLLEXPORT bool AhkObj::remove_property(const wstring& key) {
    return properties.erase(key);
}

DLLEXPORT bool AhkObj::add_method(const wstring& name, MethodType fn, AhkType ret_type, std::vector<AhkType> args) {
    methods[name] = std::make_tuple(fn, ret_type, args);
    return true;
}

DLLEXPORT bool AhkObj::remove_method(const wstring& name) {
    return methods.erase(name);
}

DLLEXPORT void* AhkObj::get(const wchar_t* key) {
    auto i = properties.find(key);
    if (i != properties.end()) {
        void* value_ptr = std::get<0>(i->second);
        AhkType type = std::get<1>(i->second);

        switch (type) {
        case AhkString:
            return (void*)((string*)value_ptr)->c_str();

        case AhkWString:
            return (void*)((wstring*)value_ptr)->c_str();

        case AhkFloat:
        case AhkDouble:
            {
                /* Must do some floating opertions here, otherwise AutoHotkey can't
                   correctly receive the return value. */
                double f;
                f = *(double*)value_ptr;
                return (void*)(*(__int64*)&f);
            }

        default:
            return (void*)(*(__int64*)value_ptr);
        }
    }

    return nullptr;
}

DLLEXPORT void AhkObj::set(const wchar_t* key, void* value, AhkType type) {
    auto i = properties.find(key);
    if (i != properties.end()) {
        void* value_ptr = std::get<0>(i->second);
        type = std::get<1>(i->second);

        switch (type) {
        case AhkChar:
        case AhkUChar:
            *(char*)value_ptr = *(char*)value;
            break;

        case AhkShort:
        case AhkUShort:
            *(short*)value_ptr = *(short*)value;
            break;

        case AhkInt:
        case AhkUInt:
            *(int*)value_ptr = *(int*)&value;
            break;

        case AhkInt64:
        case AhkPointer:
        case AhkObject:
            *(__int64*)value_ptr = *(__int64*)&value;
            break;

        case AhkFloat:
            *(float*)value_ptr = *(float*)&value;
            break;

        case AhkDouble:
            *(double*)value_ptr = *(double*)&value;
            break;

        case AhkString:
            *(string*)value_ptr = (char*)value;
            break;

        case AhkWString:
            *(wstring*)value_ptr = (wchar_t*)value;
            break;

        default:
            ;
        }
    }
}

DLLEXPORT void AhkObj::__get(const wchar_t* key, void* value, AhkType type) {
    if (obj_ref) {
        void* value_ptr = ahk_get(obj_ref, key);

        switch (type) {
        case AhkChar:
        case AhkUChar:
            *(char*)value = wcstol((wchar_t*)value_ptr, 0, 0);
            break;

        case AhkShort:
        case AhkUShort:
            *(short*)value = wcstol((wchar_t*)value_ptr, 0, 0);
            break;

        case AhkInt:
        case AhkUInt:
            *(int*)value = wcstol((wchar_t*)value_ptr, 0, 0);
            break;

        case AhkInt64:
        case AhkPointer:
        case AhkObject:
            *(__int64*)value = wcstol((wchar_t*)value_ptr, 0, 0);
            break;

        case AhkFloat:
            *(float*)value = wcstof((wchar_t*)value_ptr, 0);
            break;

        case AhkDouble:
            *(double*)value = wcstof((wchar_t*)value_ptr, 0);
            break;

        case AhkString:
            *(char**)value = (char*)value_ptr;
            break;

        case AhkWString:
            *(wchar_t**)value = (wchar_t*)value_ptr;
            break;
        }
    }
}

DLLEXPORT void AhkObj::__set(const wchar_t* key, ...) {
    if (obj_ref) {
        std::va_list ap;
        va_start(ap, key);
        ahk_set(obj_ref, key, ap);
        va_end(ap);
    }
}

DLLEXPORT void* AhkObj::__call(const wchar_t* method_name, ...) {
    void* result = nullptr;
    if (obj_ref) {
        std::va_list ap;
        va_start(ap, method_name);
        void* result = ahk_call(obj_ref, method_name, ap);
        va_end(ap);
    }

    return result;
}
