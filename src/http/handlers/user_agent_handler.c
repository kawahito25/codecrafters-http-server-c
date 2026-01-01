#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "handler.h"

void handle_user_agent(struct HTTPRequest *req, struct HTTPResponse *res) {
  int location =
      find_header_location(req->header_fields, req->header_count, "User-Agent");

  if (location < 0) {
    res->status_code = 400;
    strcpy(res->reason_phrase, "Bad Request");
  } else {
    res->status_code = 200;
    strcpy(res->reason_phrase, "OK");

    res->body = malloc(strlen(req->header_fields[location].value) + 1);
    strcpy(res->body, req->header_fields[location].value);

    append_response_header(res, CONTENT_TYPE_KEY, CONTENT_TYPE_TEXT_PLAIN);
  }
}
