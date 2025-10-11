// controllers.h
#ifndef HANDLER_H
#define HANDLER_H

#include "../http.h"

void (*HandlerFunc)(struct HTTPRequest* req, struct HTTPResponse* res);

// 関数宣言
void handle_echo(struct HTTPRequest* req, struct HTTPResponse* res);
#endif
