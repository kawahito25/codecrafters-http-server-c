#include "http.h"

#include <stdlib.h>
#include <string.h>

void free_http_request(struct HTTPRequest* req) {
  free(req->path);
  free(req);
}

void free_http_response(struct HTTPResponse* res) {
  free(res->body);
  free(res->header_fields);
  free(res);
}

struct HTTPRequest* read_request(FILE* in) {
  struct HTTPRequest* req;
  char* p;

  char buf[2048];
  fgets(buf, sizeof buf, in);

  req = malloc(sizeof(struct HTTPRequest));
  p = strchr(buf, ' ') + 1;  // リクエストパスの先頭へのポインタ
  *strchr(p, ' ') = '\0';    // リクエストパスの末尾の空白を \0 に変更

  req->path = malloc(strlen(p));
  strcpy(req->path, p);

  return req;
}

struct HTTPResponse* init_http_response() {
  struct HTTPResponse* res = malloc(sizeof(struct HTTPResponse));
  res->header_fields = NULL;
  res->body = NULL;
  res->header_count = 0;
  return res;
}

static void append_header_common(struct HTTPHeaderField** fields, int* count,
                                 char* key, char* value) {
  if (*fields == NULL) {
    *fields = malloc(sizeof(struct HTTPHeaderField));
  } else {
    void* tmp = realloc(*fields, sizeof(struct HTTPHeaderField) * (*count + 1));
    if (tmp == NULL) {
      free(*fields);
      exit(1);
    }
    *fields = tmp;
  }

  (*fields)[*count].key = malloc(strlen(key) + 1);
  strcpy((*fields)[*count].key, key);

  (*fields)[*count].value = malloc(strlen(value) + 1);
  strcpy((*fields)[*count].value, value);

  (*count)++;
}

void append_response_header(struct HTTPResponse* res, char* key, char* value) {
  append_header_common(&res->header_fields, &res->header_count, key, value);
}

void append_request_header(struct HTTPRequest* req, char* key, char* value) {
  append_header_common(&req->header_fields, &req->header_count, key, value);
}
