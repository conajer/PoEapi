/*
* AhkObj.cpp, 9/1/2020 10:37 AM
*/

#include <cstdarg>
#include <algorithm>
#include <unordered_map>
#include <string>

class AhkObjRef;    /* AutoHotkey object reference */
class AhkObj;       /* AutoHotkey object wrapper class */
class AhkObjStub;   /* AutoHotkey object stub class */

enum AhkTypes {
    AHK_INT,
    AHK_ASTR,
    AHK_WSTR,
    AHK_FLOAT,
    AHK_OBJECT,
    AHK_PTR,
};

/* AutoHotkey object related callbacks */
AhkObjRef* (*ahk_new)(const char* class_name, va_list args);
void* (*ahk_delete)(AhkObjRef* obj);
void* (*ahk_get)(AhkObjRef* obj, const char* key);
void* (*ahk_set)(AhkObjRef* obj, const char* key, va_list args);
void* (*ahk_call)(AhkObjRef* obj, const char* method, va_list args);

class AhkObj {
public:

    AhkObjRef* ahkobj_ref;

    AhkObj(const string& type_name, ...) : ahkobj_ref(0) {
        if (ahk_new) {
            va_list args;

            /* if typename is a typeid's name, which has format 'N<typename>', 
               N is length of the typename string. */
            char* ahk_typename;
            strtol(type_name.c_str(), &ahk_typename, 0);

            va_start(args, type_name);
            ahkobj_ref = ahk_new(ahk_typename, args);
            va_end(args);
        }
    }

    AhkObj(AhkObjRef* obj_ref) : ahkobj_ref(obj_ref) {
    }

    void* get(const char* key) {
        if (!ahk_get || !ahkobj_ref)
            return nullptr;

        return ahk_get(ahkobj_ref, key);
    }

    void* set(const char* key, ...) {
        if (!ahk_set || !ahkobj_ref)
            return nullptr;

        std::va_list args;
        va_start(args, key);
        void* result = ahk_set(ahkobj_ref, key, args);
        va_end(args);

        return result;
    }

    void* call(const char* method, ...) {
        if (!ahk_call || !ahkobj_ref)
            return nullptr;

        std::va_list args;
        va_start(args, method);
        void* result = ahk_call(ahkobj_ref, method, args);
        va_end(args);

        return result;
    }

    virtual ~AhkObj() {
        if (ahk_delete && ahkobj_ref)
            ahk_delete(ahkobj_ref);
    }
};

class AhkObjStub : public AhkObj, public PoEMemory {
private:

    static std::unordered_map<AhkObjRef*, AhkObjStub*> ahkobj_stubs;

    /* AutoHotkey does not actually support multithreading, 
       so one buffer makes some sense. */
    static byte* g_buffer;
    static size_t g_buffer_size;

public:

    AhkObjStub(PoEMemory& obj, addrtype address)
        : AhkObj(typeid(obj).name(), address)
    {
        if (ahkobj_ref) {
            ahkobj_stubs[ahkobj_ref] = this;
        }
    }

    static byte* read(addrtype address, size_t size) {
        if (!g_buffer || size > g_buffer_size) {
            delete[] g_buffer;
            g_buffer_size = std::max(g_buffer_size, size);
            g_buffer = new byte[g_buffer_size];
        }

        return ::read<byte>(process_handle, address, g_buffer, size);
    }

    ~AhkObjStub() {
        ahkobj_stubs.erase(ahkobj_ref);
    }
};

DLLEXPORT void ahkobj_set_callbacks(
    AhkObjRef* (*cb1)(const char* class_name, va_list args),
    void* (*cb2)(AhkObjRef* obj),
    void* (*cb3)(AhkObjRef* obj, const char* key),
    void* (*cb4)(AhkObjRef* obj, const char* key, va_list args),
    void* (*cb5)(AhkObjRef* obj, const char* method, va_list args))
{
    ahk_new = cb1;
    ahk_delete = cb2;
    ahk_get = cb3;
    ahk_set = cb4;
    ahk_call = cb5;
}

std::unordered_map<AhkObjRef*, AhkObjStub*> AhkObjStub::ahkobj_stubs;
byte* AhkObjStub::g_buffer;
size_t AhkObjStub::g_buffer_size = 0x100;

DLLEXPORT byte* ahkobj_read(addrtype address, int size) {
    return AhkObjStub::read(address, size);
}
