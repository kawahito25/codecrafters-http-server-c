#include "echo_controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_echo(struct HTTPRequest* req, struct HTTPResponse* res) {
  res->status_code = 200;
  strcpy(res->reason_phrase, "OK");
  char* p = req->path + strlen("/echo/");
  res->body = malloc(strlen(p) + 1);
  strcpy(res->body, p);
  char buf[256];
  sprintf(buf, "%ld", strlen(res->body));
  append_response_header(res, CONTENT_LENGTH_KEY, buf);
  append_response_header(res, CONTENT_TYPE_KEY, CONTENT_TYPE_TEXT_PLAIN);
}
