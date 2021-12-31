/*
* Element.cpp, 9/14/2020 7:13 PM
*/

struct Vec2 {
    float x, y;
};

struct Rect {
    int x, y, w, h;
};

float g_scale_x = 1.0f, g_scale_y = 1.0f;

std::map<string, int> element_offsets {
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
    {"item",         0x428},
    {"item_pos",     0x430},
};

class Element : public PoEObject {
private:

    bool __has_child() {
        return child_count();
    }

    AhkObjRef* __get_child(int* path) {
        std::vector<int> indices;
        for (int* p = path; *p >= 0; ++p)
            indices.push_back(*p);
        shared_ptr<Element> e = get_child(indices);

        return e ? (AhkObjRef*)*e : nullptr;
    }

    AhkObjRef* __get_childs() {
        AhkObj temp_childs;
        for (auto& i : get_childs())
            temp_childs.__set(L"", (i ? (AhkObjRef*)*i : nullptr), AhkObject, nullptr);
        __set(L"childs", (AhkObjRef*)temp_childs, AhkObject, nullptr);

        return temp_childs;
    }

    AhkObjRef* __find_child(const wchar_t* text) {
        Element* e = find_child(wstring(text));
        return e ? (AhkObjRef*)*e : nullptr;
    }

    AhkObjRef* __get_parent() {
        if (get_parent())
            return *parent;
        return nullptr;
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
    }

    void __init() {
        PoEObject::__init();
        add_method(L"hasChild", this, (MethodType)&Element::__has_child, AhkBool);
        add_method(L"getChild", this, (MethodType)&Element::__get_child, AhkObject, ParamList{AhkPointer});
        add_method(L"getChilds", this, (MethodType)&Element::__get_childs, AhkObject);
        add_method(L"findChild", this, (MethodType)&Element::__find_child, AhkObject, ParamList{AhkWString});
        add_method(L"getIndex", this, (MethodType)&Element::get_index, AhkInt, ParamList{AhkInt});
        add_method(L"getParent", this, (MethodType)&Element::__get_parent, AhkObject);
        add_method(L"getRect", this, (MethodType)&Element::__get_rect, AhkObject);
        add_method(L"getText", this, (MethodType)&Element::get_text, AhkWStringPtr);
        add_method(L"isHighlighted", this, (MethodType)&Element::is_highlighted, AhkBool);
        add_method(L"isVisible", this, (MethodType)&Element::is_visible, AhkBool);
        add_method(L"enabled", this, (MethodType)&Element::is_enabled, AhkBool);
    }

    bool is_valid() {
        return this->address == read<addrtype>("self");
    }

    wstring& get_text() {
        text = read<wstring>("text");
        return text;
   }

    shared_ptr<Element> get_parent() {
        addrtype addr = read<addrtype>("parent");
        if (addr > (addrtype)0x10000000 && addr < (addrtype)0x7F0000000000)
            parent = shared_ptr<Element>(new Element(addr));

        return parent;
    }

    int child_count() {
        addrtype addr = address + (*offsets)["childs"];
        addrtype child_begin = PoEMemory::read<addrtype>(addr);
        addrtype child_end = PoEMemory::read<addrtype>(addr + 0x8);

        return (child_end - child_begin) / 8;
    }

    shared_ptr<Element> get_child(int index) {
        int n = child_count();
        if (index >= n)
            return nullptr;

        if (childs.size() < n) {
            childs.clear();
            for (int i = 0; i < n; ++i)
                childs.push_back(shared_ptr<Element>());
        }

        addrtype addr = read<addrtype>("childs", index * 8);
        if (!childs[index] || childs[index]->address != addr)
            childs[index] = shared_ptr<Element>(new Element(addr));
        return childs[index];
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
        std::vector<addrtype> vec = read_array<addrtype>("childs", 0x0, 0x8);
        if (childs.size() == vec.size()) {
            for (int i = 0; i < childs.size(); ++i) {
                if (!vec[i])
                    childs[i].reset();
                else if (!childs[i] || vec[i] != childs[i]->address)
                    childs[i] = shared_ptr<Element>(new Element(vec[i]));
            }
        } else {
            childs.clear();
            for (auto addr : vec) {
                if (addr)
                    childs.push_back(shared_ptr<Element>(new Element(addr)));
                else
                    childs.push_back(shared_ptr<Element>());
            }
        }

        return childs;
    }

    shared_ptr<Element> operator[](int index) {
        return get_child(index);
    }

    Element* find_child(const wstring& text) {
        if (text == get_text())
            return this;

        for (auto& i : get_childs()) {
            Element* e = i->find_child(text);
            if (e != nullptr)
                return e;
        }

        return nullptr;
    }

    bool is_enabled() {
        if (!get_parent() || parent->is_visible())
            return read<byte>("is_visible") & 0x20;
        return false;
    }

    bool is_visible() {
        if (!get_parent() || parent->is_visible())
            return read<byte>("is_visible") & 0x8;
        return false;
    }

    bool is_highlighted() {
        return read<byte>("highlighted") && PoEMemory::read<byte>(address + 0x11d);
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

    int get_index(int rows) {
        Point pos = read<Point>("item_pos");
        return pos.x * rows + pos.y + 1;
    }

    Rect get_rect() {
        Vec2 pos = position();
        Vec2 size = read<Vec2>("size");
        float local_scale = scale();

        int x = pos.x * local_scale * g_scale_x;
        int y = pos.y * local_scale * g_scale_y;
        int w = size.x * local_scale * g_scale_x;
        int h = size.y * local_scale * g_scale_y;

        return {x, y, w, h};
    }

    Point get_pos() {
        Vec2 pos = position();
        Vec2 size = read<Vec2>("size");
        float local_scale = scale();

        int x = (pos.x + size.x / 2) * local_scale * g_scale_x;
        int y = (pos.y + size.y / 2) * local_scale * g_scale_y;

        return {x, y};
    }
};
