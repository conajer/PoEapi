/*
* Render.cpp, 8/8/2020 9:40 AM
*/

#include <iomanip>

struct Vector3 {
    float x;
    float y;
    float z;
};

ostream& operator<<(ostream& os, const Vector3& vec)
{
    cout << fixed << setprecision(0) << vec.x << ", " << vec.y << ", " << vec.z;
    return os;
}

/* Render component offsets */

static FieldOffsets render_component_offsets {
    {"name",     0xb0},
    {"position", 0x90},
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

    Vector3 get_position() {
        return read<Vector3>("position");
    }

    void to_print() {
        Component::to_print();
        wprintf(L"\t\t\t! %S", name().c_str());
    }
};
