#include "chat.h"

//On initialise tout le chat
void init_chat(Chat *chat) {
    chat->message_count = 0;
    for (int i = 0; i < MAX_MESSAGES; i++) {
        memset(chat->messages[i], 0, MAX_MESSAGE_LENGTH);
    }
}

// ajoute un message au chat
void add_message(Chat *chat, const char *message) {
    if (chat->message_count < MAX_MESSAGES) {
        strncpy(chat->messages[chat->message_count], message, MAX_MESSAGE_LENGTH - 1);
        chat->message_count++;
    } else {
        for (int i = 1; i < MAX_MESSAGES; i++) {
            strncpy(chat->messages[i - 1], chat->messages[i], MAX_MESSAGE_LENGTH - 1);
        }
        strncpy(chat->messages[MAX_MESSAGES - 1], message, MAX_MESSAGE_LENGTH - 1);
    }
}