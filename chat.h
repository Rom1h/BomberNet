#ifndef CHAT_H
#define CHAT_H
#include "type.h"
#include <string.h>
void init_chat(Chat *chat);
void add_message(Chat *chat, const char *message);
#endif