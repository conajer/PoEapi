/*
* Sockets.cpp, 8/10/2020 11:41 PM
*/

/* Sockets component offsets */

static std::map<string, int> sockets_component_offsets {
    {"sockets", 0x18},
    {"links",   0x60},
};

class Sockets : public Component {
protected:

    int number_of_sockets;
    int number_of_links;
    bool linked_rgb;
    int socket_types[6];

public:

    Sockets(addrtype address) : Component(address, "Sockets", &sockets_component_offsets) {
        number_of_sockets = number_of_links = 0;
        linked_rgb = false;
    }

    int sockets() {
        if (number_of_sockets == 0) {
            PoEMemory::read(address + (*offsets)["sockets"], (byte*)socket_types, 24);
            for (int t : socket_types) {
                if (t > 0 && t <= 6)
                    number_of_sockets += 1;
            }
        }

        return number_of_sockets;
    }

    int links() {
        if (number_of_links == 0) {
            int rgb = 0, i = 0;

            for (auto l : read_array<byte>("links", 0x0, 1)) {
                if (l > number_of_links)
                    number_of_links = l;

                if (l >= 3) {
                    for (int k = 0; k < l; ++k)
                        rgb |= (1 << socket_types[i + k]) & 0xf;
                }

                i += l;
            }
            linked_rgb = !(rgb ^ 0xe);
        }

        return number_of_links;
    }

    bool is_rgb() {
        links();
        return linked_rgb;
    }

    void to_print() {
        const char* socket_colors[] = {"", "R", "G", "B", "W", "A", "D"};
        int i = 0;

        Component::to_print();
        sockets();
        printf("\t\t\t! ");
        for (auto l : read_array<byte>("links", 0x0, 1)) {
            for (int k = 0; k < l; k++)
                printf("%s%s", k > 0 ? "-" : "", socket_colors[socket_types[i++]]);
            printf(" ");
        }
    }
};
