/*
* Chat.cpp, 10/9/2020 7:35 PM
*/

class Chat : public Element {
public:

    shared_ptr<Element> messages;
    shared_ptr<Element> last_message;
    unsigned int index;

    Chat(addrtype address) : Element(address) {
        get_childs();
        messages = get_child(std::vector<int>{1, 2, 1});
        index = messages->child_count();

        add_method(L"isOpened", this, (MethodType)&Chat::is_opened, AhkBool);
        add_method(L"hasNext", this, (MethodType)&Chat::has_next, AhkBool);
        add_method(L"nextMessage", this, (MethodType)&Chat::next_message, AhkWStringPtr);
    }

    bool is_opened() {
        return childs[3] ? childs[3]->is_visible() : false;
    }

    bool has_next() {
        int n = messages->child_count();
        if (n < index) {
            if (n > 500) {
                for (index = n - 10; index < n; ++index) {
                    shared_ptr<Element> message = messages->get_child(index);
                    if (message == last_message)
                        break;
                }
            } else {
                index = 0;
            }
        }

        return n > index;
    }

    wstring* next_message() {
        if (messages->child_count() > index) {
            last_message = messages->get_child(index++);
            return &last_message->get_text();
        }

        return nullptr;
    }

    int count() {
        return messages->child_count();
    }
};
