/*
* Element.cpp, 9/14/2020 7:13 PM
*/

struct Vec2 {
    float x, y;
};

struct Rect {
    int x, y, w, h;
};

static std::map<string, int> element_offsets {
    {"self",          0x18},
    {"childs",        0x38},
    {"root",          0x88},
    {"parent",        0x90},
    {"position",      0x98},
    {"scale",        0x108},
    {"is_visible",   0x111},
    {"size",         0x130},
    {"highlighted",  0x178},
    {"text",         0x2e8},
};

class Element : public PoEObject {
private:

    bool __has_child() {
        return !get_childs().empty();
    }

    AhkObjRef* __get_childs() {
        AhkObj temp_childs;
        for (auto& i : get_childs())
            temp_childs.__set(L"", (i ? (AhkObjRef*)*i : nullptr), AhkObject, nullptr);
        __set(L"childs", (AhkObjRef*)temp_childs, AhkObject, nullptr);

        return temp_childs;
    }

    AhkObjRef* __get_rect() {
        AhkTempObj rect(L"Rect");
        Rect r = get_rect();
        rect.__set(L"l", r.x, AhkInt,
                   L"t", r.y, AhkInt,
                   L"r", r.x + r.w, AhkInt,
                   L"b", r.y + r.h, AhkInt,
                   L"w", r.w, AhkInt,
                   L"h", r.h, AhkInt,
                   nullptr);

        return rect;
    }

public:

    shared_ptr<Element> parent;
    std::vector<shared_ptr<Element>> childs;
    wstring text;
    
    Element(addrtype address, FieldOffsets* offsets = &element_offsets)
        : PoEObject(address, offsets)
    {
        if (offsets != &element_offsets) {
            offsets->insert(element_offsets.begin(), element_offsets.end());
        }

        add_method(L"hasChild", this, (MethodType)&Element::__has_child, AhkBool);
        add_method(L"getChilds", this, (MethodType)&Element::__get_childs, AhkObject);
        add_method(L"getRect", this, (MethodType)&Element::__get_rect, AhkObject);
        add_method(L"getText", this, (MethodType)&Element::get_text, AhkWStringPtr);
        add_method(L"isHighlighted", this, (MethodType)&Element::is_highlighted, AhkBool);
        add_method(L"isVisible", this, (MethodType)&Element::is_visible, AhkBool);
    }

    void __new() {
        PoEObject::__new();
        if (!childs.empty()) {
            AhkObj temp_childs;
            for (auto& i : childs)
                temp_childs.__set(L"", (i ? (AhkObjRef*)*i : nullptr), AhkObject, nullptr);
            __set(L"childs", (AhkObjRef*)temp_childs, AhkObject, nullptr);
        } else {
            __set(L"childs", nullptr, AhkObject, nullptr);
        }
    }

    bool is_valid() {
        return this->address == read<addrtype>("self");
    }

    wstring& get_text() {
        if (text.empty())
            text = read<wstring>("text");
        return text;
   }

    shared_ptr<Element> get_parent() {
        if (!parent) {
            addrtype addr = read<addrtype>("parent");
            if (addr > (addrtype)0x10000000 && addr < (addrtype)0x7F0000000000)
                parent = shared_ptr<Element>(new Element(addr));
        }

        return parent;
    }

    int child_count() {
        addrtype addr = address + (*offsets)["childs"];
        addrtype child_begin = PoEMemory::read<addrtype>(addr);
        addrtype child_end = PoEMemory::read<addrtype>(addr + 0x8);

        return (child_end - child_begin) / 8;
    }

    shared_ptr<Element> get_child(int index) {
        if (child_count() <= index)
            return nullptr;

        addrtype addr = read<addrtype>("childs", index * 8);
        return shared_ptr<Element>(new Element(addr));
    }

    shared_ptr<Element> get_child(std::vector<int> indices) {
        if (indices.size() == 0)
            return nullptr;

        shared_ptr<Element> child = get_child(indices[0]);
        for (int i = 1; i < indices.size(); ++i) {
            if (!child)
                break;
            child = child->get_child(indices[i]);
        }

        return child;
    }

    std::vector<shared_ptr<Element>>& get_childs() {
        childs.clear();
        for (auto addr : read_array<addrtype>("childs", 0x0, 0x8)) {
            if (addr)
                childs.push_back(shared_ptr<Element>(new Element(addr)));
            else
                childs.push_back(shared_ptr<Element>());
        }

        return childs;
    }

    shared_ptr<Element> operator[](int index) {
        return get_child(index);
    }

    bool is_visible() {
        if (!get_parent() || parent->is_visible())
            return read<byte>("is_visible") & 0x4;
        return false;
    }

    bool is_highlighted() {
        return read<byte>("highlighted");
    }

    float scale() {
        return read<float>("scale");
    }

    Vec2 position() {
        Vec2 pos = read<Vec2>("position");
        Vec2 parentPos = {.0, .0};
        
        if (get_parent())
            parentPos = parent->position();
        pos.x += parentPos.x;
        pos.y += parentPos.y;

        return pos;
    }

    Vec2 size() {
        return read<Vec2>("size");
    }

    Rect get_rect() {
        Vec2 pos = position();
        Vec2 size = read<Vec2>("size");
        float s = scale();

        int x = pos.x * s;
        int y = pos.y * s;
        int w = size.x * s;
        int h = size.y * s;

        return {x, y, w, h};
    }

    Point get_pos() {
        Vec2 pos = position();
        Vec2 size = read<Vec2>("size");
        float s = scale();

        int x = (pos.x + size.x / 2) * s;
        int y = (pos.y + size.y / 2) * s;

        return {x, y};
    }
};
