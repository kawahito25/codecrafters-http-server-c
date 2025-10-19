#ifndef HTTP_CORE
#define HTTP_CORE

#include <stdio.h>

#define CONTENT_LENGTH_KEY "Content-Length"
#define CONTENT_TYPE_KEY "Content-Type"
#define CONTENT_TYPE_TEXT_PLAIN "text/plain"
#define CONTENT_TYPE_BINARY "application/octet-stream"

struct HTTPHeaderField {
  char* key;
  char* value;
};

enum HTTPMethod {
  HTTP_METHOD_GET,
  HTTP_METHOD_POST,
};

struct HTTPRequest {
  char* path;
  enum HTTPMethod http_method;
  struct HTTPHeaderField* header_fields;
  int header_count;
  char* body;
};

struct HTTPResponse {
  int status_code;
  char reason_phrase[256];
  struct HTTPHeaderField* header_fields;
  int header_count;
  char* body;
};

// 初期化
struct HTTPRequest* init_http_request();
struct HTTPResponse* init_http_response();

// free
void free_http_request(struct HTTPRequest* req);
void free_http_response(struct HTTPResponse* res);

// HTTPヘッダー
void append_request_header(struct HTTPRequest* req, char* key, char* value);
void append_response_header(struct HTTPResponse* res, char* key, char* value);

// I/O
void read_request(struct HTTPRequest* req, FILE* in);
void write_response(struct HTTPResponse* res, FILE* outf);

#endif
