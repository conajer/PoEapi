/*
* HeistRewardDisplay.cpp, 5/31/2021 5:07 PM
*/

/* HeistRewardDisplay component offsets */

static std::map<string, int> heist_reward_display_component_offsets {
    {"item", 0x20},
};

class HeistRewardDisplay : public Component {
public:

    HeistRewardDisplay(addrtype address)
        : Component(address, "HeistRewardDisplay", &heist_reward_display_component_offsets)
    {
    }

    addrtype item() {
        return read<addrtype>("item");
    }
};
