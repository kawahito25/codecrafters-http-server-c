// controllers.h
#ifndef HANDLER_H
#define HANDLER_H

#include "../http.h"

// 関数宣言
void handle_echo(struct HTTPRequest* req, struct HTTPResponse* res);
void handle_user_agent(struct HTTPRequest* req, struct HTTPResponse* res);
#endif
