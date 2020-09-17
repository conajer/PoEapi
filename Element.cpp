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
    wstring text;
    
    Element(addrtype address) : PoEObject(address, &element_offsets), parent(nullptr) {
    }

    shared_ptr<Element> get_parent() {
        if (!parent) {
            if (addrtype addr = read<addrtype>("parent"))
                parent = shared_ptr<Element>(new Element(addr));
        }
        return parent;
    }

    wstring& get_text() {
        if (text.empty())
            text = read<wstring>("text");
        return text;
   }

    void getChilds() {

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

    Rect get_rect() {
        Vec2 pos = position();
        Vec2 size = read<Vec2>("size");
        float s = scale();

        Rect r;
        r.x = pos.x * s + 9;
        r.y = pos.y * s + 38;
        r.x = size.x * s;
        r.x = size.y * s;

        return r;
    }

    Point get_pos(int& x, int& y) {
        Vec2 pos = position();
        Vec2 size = read<Vec2>("size");
        float s = scale();

        x = (pos.x + size.x / 2) * s + 9;
        y = (pos.y + size.y / 2) * s + 38;

        return {x, y};
    }
};
