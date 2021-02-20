/*
* Messenger.cpp, 10/23/2020 10:08 PM
*/

class Messenger : public PoEPlugin {
public:

    Messenger() : PoEPlugin(L"Messenger", "0.2") {
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        Chat* chat = poe->in_game_ui->get_chat();
        Notifications *notifications = poe->in_game_ui->get_notifications();

        if (chat->has_next()) {
            wstring* new_message = chat->next_message();
            if (new_message)
                PostThreadMessage(thread_id, WM_NEW_MESSAGE,
                                  (WPARAM)new_message->c_str(), (LPARAM)chat->index);
        }

        if (notifications->has_next()) {
            wstring* new_notification = notifications->next_notification();
            if (new_notification)
                PostThreadMessage(thread_id, WM_NEW_MESSAGE,
                                  (WPARAM)new_notification->c_str(),
                                  (LPARAM)notifications->address);
        }
    }
};
