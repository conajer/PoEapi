/*
* Render.cpp, 8/8/2020 9:40 AM
*/

#include <iomanip>

ostream& operator<<(ostream& os, const Vector3& vec)
{
    cout << fixed << setprecision(0) << vec.x << ", " << vec.y << ", " << vec.z;
    return os;
}

/* Render component offsets */

static FieldOffsets render_component_offsets {
    {"name",     0xa0},
    {"position", 0x80},
    {"bounds",   0x8c},
};

class Render : public Component {
protected:

    wstring render_name;

public:

    Render(addrtype address) : Component(address, "Render", &render_component_offsets) {
    }

    wstring& name() {
        if (render_name.empty())
            render_name = read<wstring>("name");
        return render_name;
    }

    Vector3 position() {
        return read<Vector3>("position");
    }

    Vector3 bounds() {
        return read<Vector3>("bounds");
    }

    void to_print() {
        Component::to_print();
        wprintf(L"\t\t\t! %S", name().c_str());
    }
};
