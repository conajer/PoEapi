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
    {"childs",	      0x38},
    {"parent",	      0x90},
    {"position",	  0x98},
    {"scale",	     0x108},
    {"is_visible",   0x111},
    {"size",	     0x130},
    {"is_highlight", 0x178},
    {"label",	     0x238},
};

class Element : public PoEObject {
public:

    shared_ptr<Element> parent;
    std::vector<shared_ptr<Element>> childs;
    Rect r;
    wstring text;
    
    Element(addrtype address, FieldOffsets* offsets = &element_offsets)
        : PoEObject(address, offsets)
    {
        if (offsets != &element_offsets)
            offsets->insert(element_offsets.begin(), element_offsets.end());
        add_method(L"getChilds", this, (MethodType)&Element::get_childs);
        add_method(L"getRect", this, (MethodType)&Element::get_rect, AhkPointer);
        add_method(L"getText", this, (MethodType)&Element::get_text, AhkWStringPtr);
        add_method(L"isHighlight", this, (MethodType)&Element::is_highlight, AhkBool);
        add_method(L"isVisible", this, (MethodType)&Element::is_visible, AhkBool);
    }

    wstring& get_text() {
        if (text.empty())
            text = read<wstring>("text");
        return text;
   }

    shared_ptr<Element> get_parent() {
        if (!parent) {
            if (addrtype addr = read<addrtype>("parent"))
                parent = shared_ptr<Element>(new Element(addr));
        }
        return parent;
    }

    Element* get_child(int index) {
        addrtype addr = read<addrtype>("child", index * 8);
        return new Element(addr);
    }

    std::vector<shared_ptr<Element>>& get_childs() {
        childs.clear();
        for (auto addr : read_array<addrtype>("childs", 0x0, 0x8)) {
            if (addr)
                childs.push_back(shared_ptr<Element>(new Element(addr)));
            else
                childs.push_back(shared_ptr<Element>());
        }

        if (obj_ref) {
            AhkObjRef* ahkobj_ref;

            __set(L"Childs", nullptr, AhkObject, nullptr);
            __get(L"Childs", &ahkobj_ref, AhkObject);
            AhkObj elements(ahkobj_ref);
            for (auto& i : childs) {
                if (i.get() != nullptr)
                    elements.__set(L"", (AhkObjRef*)*i, AhkObject, nullptr);
            }
        }

        return childs;
    }

    shared_ptr<Element>& operator[](int index) {
        if (childs.empty())
            get_childs();
        return childs[index];
    }

    bool is_visible() {
        return read<byte>("is_visible") & 0x4;
    }

    bool is_highlight() {
        return read<byte>("is_highlight");
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

    Rect& get_rect() {
        Vec2 pos = position();
        Vec2 size = read<Vec2>("size");
        float s = scale();

        r.x = pos.x * s;
        r.y = pos.y * s;
        r.w = size.x * s;
        r.h = size.y * s;

        __set(L"x", r.x, AhkInt,
              L"y", r.y, AhkInt,
              L"w", r.w, AhkInt,
              L"h", r.h, AhkInt,
              nullptr);

        return r;
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
