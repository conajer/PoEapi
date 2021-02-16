/*
* Positioned.cpp, 8/8/2020 11:22 AM
*/

/* Positioned component offsets */

struct Point {
    int x;
    int y;
};

struct Vector3 {
    float x;
    float y;
    float z;
};

static std::map<string, int> positioned_component_offsets {
    {"is_neutral",      0x15c},
    {"grid_position",   0x1ec},
    {"position",        0x21c},
};

class Positioned : public Component {
public:

    Positioned(addrtype address)
        : Component(address, "Positioned", &positioned_component_offsets)
    {
    }

    bool is_neutral() {
        return read<byte>("is_neutral") == 0x81;
    }

    Point grid_position() {
        return read<Point>("grid_position");
    }

    Vector3 position() {
        Vector3 vec = read<Vector3>("position");
        vec.z = 0;
        return vec;
    }

    void to_print() {
        Component::to_print();
        Point pos = grid_position();
        printf("\t\t\t! %d, %d", pos.x, pos.y);
    }
};
