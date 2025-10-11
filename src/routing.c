#include "routing.h"

#include <string.h>

#include "controllers/echo_controller.h"

#define ECHO_PREFIX "/echo/"

void handle_request(struct HTTPRequest* req, struct HTTPResponse* res) {
  if (strcmp(req->path, "/") == 0) {
    res->status_code = 200;
    strcpy(res->reason_phrase, "OK");
  } else if (strncmp(req->path, ECHO_PREFIX, strlen(ECHO_PREFIX)) == 0) {
    handle_echo(req, res);
  } else {
    res->status_code = 404;
    strcpy(res->reason_phrase, "Not Found");
  }
}
