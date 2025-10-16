#include "router.h"

#include <string.h>

#include "../core/core.h"
#include "../handlers/handler.h"

void handle_request(struct HTTPRequest* req, struct HTTPResponse* res) {
  if (req->http_method == HTTP_METHOD_GET && strcmp(req->path, "/") == 0) {
    res->status_code = 200;
    strcpy(res->reason_phrase, "OK");
  } else if (req->http_method == HTTP_METHOD_GET &&
             strncmp(req->path, "/echo", strlen("/echo")) == 0) {
    handle_echo(req, res);
  } else if (req->http_method == HTTP_METHOD_GET &&
             strcmp(req->path, "/user-agent") == 0) {
    handle_user_agent(req, res);
  } else if (req->http_method == HTTP_METHOD_GET &&
             strcmp(req->path, "/files")) {
    handle_file(req, res);
  } else {
    res->status_code = 404;
    strcpy(res->reason_phrase, "Not Found");
  }
}
