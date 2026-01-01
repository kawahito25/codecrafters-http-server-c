
#include "core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

void write_response(struct HTTPResponse *res, FILE *outf) {
  fprintf(outf, "HTTP/1.1 %d %s", res->status_code, res->reason_phrase);
  fputs("\r\n", outf); // CRLF that marks the end of the status line

  int compress_with_gzip = 0;
  for (int i = 0; i < res->header_count; i++) {
    fputs(res->header_fields[i].key, outf);
    fputs(": ", outf);
    fputs(res->header_fields[i].value, outf);
    fputs("\r\n", outf);

    if (strcmp(res->header_fields[i].key, CONTENT_ENCODING_KEY) == 0 &&
        strcmp(res->header_fields[i].value, "gzip") == 0) {
      compress_with_gzip = 1;
    }
  }
  fputs("\r\n", outf); // CRLF that marks the end of the headers

  if (res->body != NULL) {
    if (compress_with_gzip) {
      size_t srcLen = strlen(res->body) + 1;

      // 1. 圧縮後の最大サイズを計算し、バッファを確保
      uLong destLen = compressBound(srcLen) + 18; // gzipヘッダー分を考慮
      unsigned char *dest = (unsigned char *)malloc(destLen);

      // 2. z_stream 構造体の初期化
      z_stream strm;
      strm.zalloc = Z_NULL;
      strm.zfree = Z_NULL;
      strm.opaque = Z_NULL;
      strm.next_in = (Bytef *)res->body;
      strm.avail_in = (uInt)srcLen;
      strm.next_out = dest;
      strm.avail_out = (uInt)destLen;

      // 3. gzip形式を指定して初期化 (16 + 15 が gzip のマジックナンバー)
      if (deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + 15, 8,
                       Z_DEFAULT_STRATEGY) != Z_OK) {
        return;
      }

      // 4. 圧縮実行
      deflate(&strm, Z_FINISH);
      uLong actualGzipLen = strm.total_out;

      // 5. 後処理
      deflateEnd(&strm);

      fwrite(dest, 1, actualGzipLen, outf);
    } else {
      fputs(res->body, outf); // TODO: \0 がない場合に対応できない
    }
  }
}

#define FREE_HTTP_HEADER_FIELDS(fields, count)                                 \
  do {                                                                         \
    for (int i = 0; i < count; i++) {                                          \
      free(fields[i].key);                                                     \
      free(fields[i].value);                                                   \
    }                                                                          \
  } while (0);

void free_http_request(struct HTTPRequest *req) {
  free(req->path);
  FREE_HTTP_HEADER_FIELDS(req->header_fields, req->header_count);
  free(req->header_fields);
  free(req->body);
  free(req);
}

void free_http_response(struct HTTPResponse *res) {
  free(res->body);
  FREE_HTTP_HEADER_FIELDS(res->header_fields, res->header_count);
  free(res->header_fields);
  free(res);
}

#undef FREE_HTTP_HEADER_FIELDS

#define MAX_REQUEST_HEADER_LENGTH 4096

void read_request(struct HTTPRequest *req, FILE *in) {
  char *p;

  char buf[MAX_REQUEST_HEADER_LENGTH];

  /* ステータスライン */
  fgets(buf, sizeof buf, in);
  p = strchr(buf, ' ');
  *p++ = '\0';

  if (strcmp(buf, "GET") == 0) {
    req->http_method = HTTP_METHOD_GET;
  } else if (strcmp(buf, "POST") == 0) {
    req->http_method = HTTP_METHOD_POST;
  }

  *strchr(p, ' ') = '\0'; // リクエストパスの末尾の空白を \0 に変更
  req->path = malloc(strlen(p));
  strcpy(req->path, p);

  int content_length = 0;

  /* リクエストヘッダ */
  int i = 0;
  while (1) {
    fgets(buf, sizeof buf, in);
    if (strcmp(buf, "\r\n") == 0) {
      break;
    }

    char *key = buf;
    *strchr(key, ':') = '\0';

    char *value = key + strlen(key) + 2;
    value[strcspn(value, "\r\n")] = '\0'; // 注意: fgets は改行も読み込む

    append_request_header(req, key, value);
    if (strcmp(key, CONTENT_LENGTH_KEY) == 0) {
      content_length = atoi(value);
    }
  }

  /* リクエストボディ */
  if (content_length) {
    req->body = malloc(content_length);
    // TODO:  content_length と 実際のリクエストボディの長さに差がある場合
    size_t s = fread(req->body, 1, content_length, in);
    if (ferror(in)) {
      perror("read request body");
      return;
    }
  }
}

struct HTTPRequest *init_http_request() {
  struct HTTPRequest *req = malloc(sizeof(struct HTTPRequest));
  req->header_fields = NULL;
  req->header_count = 0;
  return req;
}

struct HTTPResponse *init_http_response() {
  struct HTTPResponse *res = malloc(sizeof(struct HTTPResponse));
  res->header_fields = NULL;
  res->body = NULL;
  res->header_count = 0;
  return res;
}

static void append_header_common(struct HTTPHeaderField **fields, int *count,
                                 char *key, char *value) {
  if (*fields == NULL) {
    *fields = malloc(sizeof(struct HTTPHeaderField));
  } else {
    void *tmp = realloc(*fields, sizeof(struct HTTPHeaderField) * (*count + 1));
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

void append_response_header(struct HTTPResponse *res, char *key, char *value) {
  append_header_common(&res->header_fields, &res->header_count, key, value);
}

void append_request_header(struct HTTPRequest *req, char *key, char *value) {
  append_header_common(&req->header_fields, &req->header_count, key, value);
}

int find_header_location(struct HTTPHeaderField *fields, int count, char *key) {
  int location = -1;
  for (int i = 0; i < count; i++) {
    if (strcmp(fields[i].key, key) == 0) {
      location = i;
      break;
    }
  }

  return location;
}

// static char accept_encodings[1][5] = {"gzip"};

void append_common_response_headers(struct HTTPRequest *req,
                                    struct HTTPResponse *res) {
  int location = -1;

  // Content-Encoding
  location = find_header_location(req->header_fields, req->header_count,
                                  ACCEPT_ENCODING_KEY);
  if (location < 0) {
    return;
  }

  char *start = req->header_fields[location].value;
  char *end;

  do {
    end = strchr(start, ',');
    if (end != NULL) {
      *end = '\0';
    }
    if (strcmp(start, "gzip") == 0) {
      append_response_header(res, CONTENT_ENCODING_KEY, "gzip");
      break;
    }
    if (end != NULL) {
      start = end + 2;
    }
  } while (end != NULL);

  return;
}
