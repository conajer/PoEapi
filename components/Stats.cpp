/*
* Stats.cpp, 5/21/2022 4:12 AM
*/

/* Stats component offsets */

static std::map<string, int> stats_component_offsets {
    {"data",        0x20},
        {"stats",   0xf0},
};

class Stats : public Component {
public:

    std::map<int, int> stats;

    Stats(addrtype address) : Component(address, "Stats", &stats_component_offsets) {
    }

    std::map<int, int>& get_stats() {
        auto local_stats = read_array<__int64>("data", "stats", 0, 0x8);

        stats.clear();
        for (auto& i : local_stats)
            stats[i & 0xffff] = i >> 32;

        return stats;
    }

    int get_stat(int key) {
        auto local_stats = read_array<__int64>("data", "stats", 0, 0x8);

        for (auto& i : local_stats) {
            if ((i & 0xffff) == key)
                return i >> 32;
        }

        return -1;
    }
};
