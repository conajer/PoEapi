/*
* ahkpp.cpp, 9/6/2020 1:22 PM
*/

#include <cstdarg>
#include "ahkpp"

template <typename...Ts> void ahkobj_fallback(Ts...args) {}
template <typename T, typename...Ts> T ahkobj_fallback(Ts...args) {return nullptr;}

/* AutoHotkey object manipulating callbacks */
AhkObjRef* (*ahk_new)(const wchar_t*, const wchar_t*) = ahkobj_fallback;
void (*ahk_delete)(AhkObjRef*) = ahkobj_fallback;
void* (*ahk_get)(AhkObjRef*, const wchar_t*) = ahkobj_fallback;
void (*ahk_set)(AhkObjRef*, const wchar_t*, va_list) = ahkobj_fallback;
void* (*ahk_call)(AhkObjRef*, const wchar_t*, va_list) = ahkobj_fallback;

/* Golbal AutoHotkey object index map */
std::unordered_map<AhkObjRef*, AhkObj*> ahkpp_stubs;

template <typename T> using Factory = std::unordered_map<wstring, std::function<T*()>>;
Factory<AhkObj> ahkpp_factory;

DLLEXPORT void ahkpp_set_callbacks(
    AhkObjRef* (*__new)(const wchar_t*, const wchar_t*),
    void (*__delete)(AhkObjRef*),
    void* (*__get)(AhkObjRef*, const wchar_t*),
    void (*__set)(AhkObjRef*, const wchar_t*, va_list),
    void* (*__call)(AhkObjRef*, const wchar_t*, va_list))
{
    ahk_new = __new;
    ahk_delete = __delete;
    ahk_get = __get;
    ahk_set = __set;
    ahk_call = __call;
}

DLLEXPORT void ahkpp_register(const wchar_t* type_name, std::function<AhkObj*()> fn) {
    ahkpp_factory[type_name] = fn;
}

DLLEXPORT void ahkpp_unregister(const wchar_t* type_name) {
    ahkpp_factory.erase(type_name);
}

DLLEXPORT AhkObjRef* ahkpp_new(AhkObjRef* ahkobj_ref, const wchar_t* type_name) {
    if (!ahkobj_ref) {
        if (ahkpp_factory.find(type_name) != ahkpp_factory.end()) {
            AhkObj* obj = ahkpp_factory[type_name]();
            return *obj;
        }
    } else {
        auto i = ahkpp_stubs.find(ahkobj_ref);
        if (i == ahkpp_stubs.end()) {
            if (ahkpp_factory.find(type_name) == ahkpp_factory.end()) {
                /* object -> class -> base class */
                AhkObjRef* base = (AhkObjRef*)ahk_get(ahkobj_ref, L"base");
                if (!base)
                    return nullptr;

                base = (AhkObjRef*)ahk_get(base, L"base");
                if (!base)
                    return nullptr;

                type_name = (const wchar_t*)ahk_get(base, L"__Class");
                if (!type_name || ahkpp_factory.find(type_name) == ahkpp_factory.end())
                    return nullptr;
            }

            AhkObj* obj = ahkpp_factory[L"Hello"]();
            obj->obj_ref = ahkobj_ref;
            obj->__init();
            ahkpp_stubs[ahkobj_ref] = obj;
        }
    }

    return ahkobj_ref;
}

DLLEXPORT void* ahkpp_get(AhkObjRef* ahkobj_ref, const wchar_t* key) {
    auto i = ahkpp_stubs.find(ahkobj_ref);
    if (i != ahkpp_stubs.end())
        return i->second->get(key);

    return 0;
}

DLLEXPORT void ahkpp_set(AhkObjRef* ahkobj_ref, const wchar_t* key, void* value) {
    auto i = ahkpp_stubs.find(ahkobj_ref);
    if (i != ahkpp_stubs.end()) {
        i->second->set(key, value);
    }
}

/* Uses fixed size parameters, for performace reason, only supported up to 8 parameters. */
using MethodType = void* (AhkObj::*)(void*, void*, void*, void*, void*, void*, void*, void*);

DLLEXPORT void* ahkpp_call(AhkObjRef* ahkobj_ref, const wchar_t* method_name, ...) {
    void* result = nullptr;

    auto i = ahkpp_stubs.find(ahkobj_ref);
    if (i != ahkpp_stubs.end()) {
        auto m = i->second->methods.find(method_name);
        if (m != i->second->methods.end()) {
            va_list ap;

            va_start(ap, method_name);
            void** args = (void**)ap;
            result = (i->second->*(MethodType)std::get<0>(m->second))(
                args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
            va_end(ap);
        }
    }

    return result;
}
