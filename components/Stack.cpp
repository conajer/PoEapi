/*
* Stack.cpp, 8/11/2020 10:41 AM
*/

/* Stack component offsets */

static std::map<string, int> stack_component_offsets {
    {"base",           0x10},
        {"stack_size", 0x28},
    {"stack_count",    0x18},
};

class Stack : public Component {
public:

    Stack(addrtype address) : Component(address, "Stack", &stack_component_offsets) {
    }

    int stack_count() {
        return read<int>("stack_count");
    }

    int stack_size() {
        return read<int>("base", "stack_size");
    }

    void to_print() {
        Component::to_print();
        printf("\t\t\t! %d/%d", stack_count(), stack_size());
    }
};
