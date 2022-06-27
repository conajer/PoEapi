/*
* Chat.cpp, 10/9/2020 7:35 PM
*/

class Chat : public Element {
public:

    shared_ptr<Element> messages;
    shared_ptr<Element> last_message;

    Chat(addrtype address) : Element(address) {
        get_childs();
        messages = get_child(std::vector<int>{1, 2, 1});
        last_message = messages->get_child(messages->child_count() - 1);

        add_method(L"isOpened", this, (MethodType)&Chat::is_opened, AhkBool);
        add_method(L"count", this, (MethodType)&Chat::count, AhkInt);
        add_method(L"getMessages", this, (MethodType)&Chat::get_messages, AhkObject, ParamList{AhkInt, AhkInt});
        add_method(L"hasNext", this, (MethodType)&Chat::has_next, AhkBool);
        add_method(L"nextMessage", this, (MethodType)&Chat::next_message, AhkWStringPtr);
    }

    bool is_opened() {
        return (childs.size() >= 4) ? childs[3]->is_visible() : false;
    }

    int count() {
        return messages ? messages->child_count() : 0;
    }

    AhkObjRef* get_messages(int index, int count) {
        if (messages) {
            int n = messages->child_count();
            AhkTempObj msgs;

            index = (index >= 1) ? index - 1 : max(1, n + index - 1);
            for (int i = index; i < min(index + count, n); ++i) {
                shared_ptr<Element> message = messages->get_child(i);
                msgs.__set(L"", message->get_text().c_str(), AhkWString, nullptr);
            }

            return msgs;
        }

        return nullptr;
    }

    bool has_next() {
        if (messages) {
            int n = messages->child_count();
            if (n > 0) {
                shared_ptr<Element> message = messages->get_child(n - 1);
                if (message != last_message)
                    return true;
            }
        }

        return false;
    }

    wstring* next_message() {
        if (messages) {
            shared_ptr<Element> new_message;
            int n = messages->child_count();

            for (int i = n - 1; i >= max(0, n - 16); --i ) {
                shared_ptr<Element> message = messages->get_child(i);

                if (!message || !last_message) {
                    new_message = message;
                    break;
                } else if (message->address == last_message->address) {
                    break;
                }
                new_message = message;
            }

            if (new_message) {
                last_message = new_message;
                return &last_message->get_text();
            }
        }

        return nullptr;
    }
};
