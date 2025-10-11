#include <stdio.h>

#define CONTENT_LENGTH_KEY "Content-Length"
#define CONTENT_TYPE_KEY "Content-Type"
#define CONTENT_TYPE_TEXT_PLAIN "text/plain"

#ifndef HTTP_H
#define HTTP_H
struct HTTPHeaderField {
  char* key;
  char* value;
};

struct HTTPRequest {
  char* path;
  struct HTTPHeaderField* header_fields;
  int header_count;
};

struct HTTPResponse {
  int status_code;
  char reason_phrase[256];
  struct HTTPHeaderField* header_fields;
  int header_count;
  char* body;
};

void free_http_request(struct HTTPRequest* req);
void free_http_response(struct HTTPResponse* req);

struct HTTPRequest* read_request(FILE* in);
struct HTTPResponse* init_http_response();
void append_request_header(struct HTTPRequest* res, char* key, char* value);
void append_response_header(struct HTTPResponse* res, char* key, char* value);
#endif
