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
    int len = read<int>(handle, address + 0x10);
    string str;

    if (len > 0 && len < 256) {
        wchar_t buffer[len];
        if (len >= 8)
            address = read<addrtype>(handle, address);
        if (ReadProcessMemory(handle, (LPVOID)address, buffer, len * 2, 0))
            for (wchar_t c : buffer)
                str += c;
    }

    return str;
}

template <> wstring read<wstring>(HANDLE handle, addrtype address) {
    int len = read<int>(handle, address + 0x10);
    wstring str;

    if (len > 0 && len < 256) {
        wchar_t buffer[len + 1];
        if (len >= 8)
            address = read<addrtype>(handle, address);
        if (ReadProcessMemory(handle, (LPVOID)address, buffer, (len + 1) * 2, 0))
            str = buffer;
    }

    return str;
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
        ReadProcessMemory(process_handle, (LPVOID)address, &address, 8, 0);
        for (int i = 0; i < n - 1; ++i)
            ReadProcessMemory(process_handle, (LPVOID)(address + offsets[i]), &address, 8, 0);
        return read<T>(address + offsets[n - 1]);
    }

    template <typename T> T read(addrtype address, int size) {
        return ::read<T>(process_handle, address, size);
    }

    template <typename T> std::vector<T> read_array(addrtype address, int element_size) {
        addrtype begin = read<addrtype>(address);
        addrtype end = read<addrtype>(address + 0x8);

        std::vector<T> vec;
        for (address = begin; address < end; address += element_size) {
            vec.push_back(T(address));
        }

        return vec;
    }

    template <typename T> std::vector<T> read_array(addrtype address, int offset, int element_size) {
        addrtype begin = read<addrtype>(address);
        addrtype end = read<addrtype>(address + 0x8);

        std::vector<T> vec;
        for (address = begin; address < end; address += element_size) {
            vec.push_back(T(read<addrtype>(address + offset)));
        }

        return vec;
    }
};

HANDLE PoEMemory::process_handle;
