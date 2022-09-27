/*
*  PoEMemory.cpp, 8/4/2020 8:51 PM
*
*  Path of Exile memory access interface.
*/

template <typename T> T* read(HANDLE handle, addrtype address, T* buffer, int n) {
        if (ReadProcessMemory(handle, (LPVOID)address, buffer, n * sizeof(T), 0))
            return buffer;
        return nullptr;
    }

template <> void* read(HANDLE handle, addrtype address, void* buffer, int size) {
        if (ReadProcessMemory(handle, (LPVOID)address, buffer, size, 0))
            return buffer;
        return nullptr;
    }

template <typename T> T read(HANDLE handle, addrtype address, int size) {
    T t = {};
    if (ReadProcessMemory(handle, (LPVOID)address, &address, sizeof(addrtype), 0))
        ReadProcessMemory(handle, (LPVOID)address, &t, size, 0);
    return t;
}

template <> string read(HANDLE handle, addrtype address, int len) {
    char buffer[len + 1];
    if (ReadProcessMemory(handle, (LPVOID)address, &address, sizeof(addrtype), 0))
        if (ReadProcessMemory(handle, (LPVOID)address, buffer, len, 0)) {
            buffer[len] = '\0';
            return buffer;
        }

    return "";
}

template <> wstring read(HANDLE handle, addrtype address, int len) {
    wchar_t buffer[len + 1];
    if (ReadProcessMemory(handle, (LPVOID)address, &address, sizeof(addrtype), 0))
        if (ReadProcessMemory(handle, (LPVOID)address, buffer, 2 * len, 0)) {
            buffer[len] = L'\0';
            return buffer;
        }

    return L"";
}

template <typename T> T read(HANDLE handle, addrtype address) {
    T t = {};
    ReadProcessMemory(handle, (LPVOID)address, &t, sizeof(T), 0);
    return t;
}

template <> string read<string>(HANDLE handle, addrtype address) {
    unsigned int len = read<int>(handle, address + 0x10);
    unsigned int max_len = read<int>(handle, address + 0x18);
    string str;

    if (max_len < 256) {
        wchar_t buffer[len];
        if (max_len >= 8)
            address = read<addrtype>(handle, address);
        if (ReadProcessMemory(handle, (LPVOID)address, buffer, len * 2, 0))
            for (wchar_t c : buffer)
                str += c;
    }

    return str;
}

template <> wstring read<wstring>(HANDLE handle, addrtype address) {
    unsigned int len = read<int>(handle, address + 0x10);
    unsigned int max_len = read<int>(handle, address + 0x18);

    if (max_len != 8) {
        if (len <= max_len && len < 2048 && max_len < 2048) {
            wchar_t buffer[len + 1];
            if (max_len >= 8)
                address = read<addrtype>(handle, address);
            if (ReadProcessMemory(handle, (LPVOID)address, buffer, (len + 1) * 2, 0))
                return wstring(buffer, len);
        }
    }

    return L"";
}

template <typename T> std::vector<T> read_array(HANDLE handle, addrtype address, int element_size) {
    std::vector<T> vec;
    addrtype ptrs[2];
    if (address && ReadProcessMemory(handle, (LPVOID)address, ptrs, 16, 0)) {
        size_t size = ptrs[1] - ptrs[0];
        if (size > 0 && size / element_size < 1024) {
            for (address = ptrs[0]; address < ptrs[1]; address += element_size)
                vec.push_back(T(address));
        }
    }

    return vec;
}

template <> std::vector<wstring> read_array(HANDLE handle, addrtype address, int element_size) {
    std::vector<wstring> vec;
    addrtype ptrs[2];
    if (address && ReadProcessMemory(handle, (LPVOID)address, ptrs, 16, 0)) {
        size_t size = ptrs[1] - ptrs[0];
        if (size > 0 && size / element_size < 1024) {
            for (address = ptrs[0]; address < ptrs[1]; address += element_size)
                vec.push_back(read<wstring>(handle, address));
        }
    }

    return vec;
}

template <typename T> std::vector<T> read_array(HANDLE handle, addrtype address, int offset, int element_size) {
    std::vector<T> vec;
    addrtype ptrs[2];
    if (address && ReadProcessMemory(handle, (LPVOID)address, ptrs, 16, 0)) {
        size_t size = ptrs[1] - ptrs[0];
        if (size > 0 && size / element_size < 1024) {
            byte buffer[size];
            if (ReadProcessMemory(handle, (LPVOID)ptrs[0], buffer, size, 0)) {
                for (int i = 0; i < size; i += element_size)
                    vec.push_back(T(*(addrtype *)&buffer[i + offset]));
            }
        }
    }

    return vec;
}

template <> std::vector<wstring> read_array(HANDLE handle, addrtype address, int offset, int element_size) {
    std::vector<wstring> vec;
    addrtype ptrs[2];
    if (address && ReadProcessMemory(handle, (LPVOID)address, ptrs, 16, 0)) {
        size_t size = ptrs[1] - ptrs[0];
        if (size > 0 && size / element_size < 1024) {
            byte buffer[size];
            if (ReadProcessMemory(handle, (LPVOID)ptrs[0], buffer, size, 0)) {
                for (int i = 0; i < size; i += element_size)
                    vec.push_back(read<wstring>(handle, *(addrtype *)&buffer[i + offset]));
            }
        }
    }


    return vec;
}

template <typename T> bool write(HANDLE handle, addrtype address, T* buffer, int n) {
    DWORD old_protect;
    
    int size = n * sizeof(T);
    if (VirtualProtectEx(handle, (LPVOID)address, size, PAGE_EXECUTE_READWRITE, &old_protect)) {
        WriteProcessMemory(handle, (LPVOID)address, buffer, size, 0);
        VirtualProtectEx(handle, (LPVOID)address, size, old_protect, 0);
        return true;
    }

    return false;
}

class PoEMemory {
protected:

    static HANDLE process_handle;

public:

    template <typename T> T* read(addrtype address, T* buffer, int n) {
        return ::read<T>(process_handle, address, buffer, n);
    }

    template <typename T> T read(addrtype address) {
        return ::read<T>(process_handle, address);
    }

    template <typename T> T read(addrtype address, vector<int> offsets) {
        int n = offsets.size();
        for (int i = 0; i < n - 1; ++i)
            ReadProcessMemory(process_handle, (LPVOID)(address + offsets[i]), &address, 8, 0);
        return read<T>(address + offsets[n - 1]);
    }

    template <typename T> T read(addrtype address, int size) {
        return ::read<T>(process_handle, address, size);
    }

    template <typename T> std::vector<T> read_array(addrtype address, int element_size) {
        return ::read_array<T>(process_handle, address, element_size);
    }

    template <typename T> std::vector<T> read_array(addrtype address, int offset, int element_size) {
        return ::read_array<T>(process_handle, address, offset, element_size);
    }

    template <typename T> bool write(addrtype address, T* buffer, int n) {
        return ::write(process_handle, address, buffer, n);
    }
};

HANDLE PoEMemory::process_handle;
