/*
* Positioned.cpp, 8/8/2020 11:22 AM
*/

/* Positioned component offsets */

struct Point {
    int x;
    int y;

    Point() : x(0), y(0) {
    }

    Point(int x, int y) : x(x), y(y) {
    }
};

static std::map<string, int> positioned_component_offsets {
    {"is_neutral",      0x158},
    {"grid_position",   0x1e8},
    {"position",        0x218},
};

class Positioned : public Component {
public:

    Positioned(addrtype address)
        : Component(address, "Positioned", &positioned_component_offsets)
    {
    }

    bool is_neutral() {
        return read<byte>("is_neutral") & 0x7f;
    }

    Point get_position() {
        return read<Point>("grid_position");
    }

    void to_print() {
        Component::to_print();
        Point pos = get_position();
        printf("\t\t\t! %d, %d", pos.x, pos.y);
    }
};
