/*
* Messenger.cpp, 10/23/2020 10:08 PM
*/

class Messenger : public PoEPlugin {
public:

    Messenger() : PoEPlugin(L"Chatter", "0.1") {
    }

    void on_player(LocalPlayer* local_player, InGameState* in_game_state) {
        InGameUI* in_game_ui = in_game_state->in_game_ui();
        Chat* chat = in_game_ui->get_chat();
        NotificationArea *notification_area = in_game_ui->get_notification_area();

        if (chat->has_next()) {
            wstring* new_message = chat->next_message();
            PostThreadMessage(thread_id, WM_NEW_MESSAGE, (WPARAM)new_message->c_str(), (LPARAM)chat->index);
        }

        if (notification_area->has_next()) {
            wstring* new_notification = notification_area->next_notification();
            PostThreadMessage(thread_id, WM_NEW_MESSAGE,
                              (WPARAM)new_notification->c_str(),
                              (LPARAM)(AhkObjRef*)*notification_area->last_notification);
        }
    }
};
