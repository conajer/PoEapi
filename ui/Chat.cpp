/*
* Chat.cpp, 10/9/2020 7:35 PM
*/

class Chat : public Element {
public:

    std::vector<wstring> messages;
    std::wregex ignored_message;

    Chat(addrtype address) : Element(address), ignored_message(L"^[#$]") {
    }

    wstring* get_new_message() {
        return nullptr;
    }

    int size() {
        return messages.size();
    }
};
