#include "http.h"

#include <unistd.h>

#include "core/core.h"
#include "router/router.h"
#include <string.h>
#include <sys/socket.h>

void do_http_service(int sock) {
  // 5秒でタイムアウトさせる
  struct timeval tv = {5, 0};
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  FILE *inf = fdopen(sock, "r");
  FILE *outf = fdopen(sock, "w");

  while (1) {
    struct HTTPRequest *req = init_http_request();
    struct HTTPResponse *res = init_http_response();

    if (read_request(req, inf) == -1) {
      free_http_request(req);
      free_http_response(res);
      printf("failed to read request\n");
      break;
    }
    handle_request(req, res);
    append_common_response_headers(req, res);
    write_response(res, outf);
    fflush(outf);

    // when Connection: close, break the loop
    int location = find_header_location(req->header_fields, req->header_count,
                                        CONNECTION_KEY);
    if (location >= 0 &&
        strcmp(req->header_fields[location].value, "close") == 0) {
      free_http_request(req);
      free_http_response(res);
      break;
    }

    free_http_request(req);
    free_http_response(res);
  }

  close(sock);
}
