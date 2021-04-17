/*
* Notifications.cpp, 10/24/2020 11:13 AM
*/

class Notifications : public Element {
public:

    std::vector<shared_ptr<Element>> notifications;
    shared_ptr<Element> last_notification;
    int index = 0;

    Notifications(addrtype address) : Element(address) {
        add_method(L"hasNext", this, (MethodType)&Notifications::has_next, AhkBool);
        add_method(L"nextNotification", this, (MethodType)&Notifications::next_notification, AhkWStringPtr);
    }

    bool has_next() {
        int n = child_count();
        if (n == 0) {
            index = 0;
            last_notification.reset();
            notifications.clear();

            return false;
        }

        if (index > 0) {
            index = 0;
            for (int i = n - 1; i >= 0; --i) {
                if (last_notification->address == get_child(i)->address) {
                    index = i + 1;
                    break;
                }
            }
        }

        return n > index;
    }

    wstring* next_notification() {
        if (child_count() > index) {
            last_notification = get_child(index++);
            if (last_notification) {
                notifications.push_back(last_notification);

                int n = last_notification->child_count();
                if (n > 2) {
                    shared_ptr<Element> player = last_notification->get_child(std::vector<int>{0, 0, 1});
                    shared_ptr<Element> notification_text = last_notification->get_child(std::vector<int>{0, 1});
                    if (notification_text)
                        last_notification->text = player->get_text() + L" " + notification_text->get_text();
                } else {
                    shared_ptr<Element> notification_text = last_notification->get_child(0);
                    if (notification_text)
                        last_notification->text = notification_text->get_text();
                }

                return &last_notification->get_text();
            }
        }

        return nullptr;
    }

    int count() {
        return child_count();
    }
};
