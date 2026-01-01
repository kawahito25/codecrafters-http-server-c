#include "http.h"

#include <unistd.h>

#include "core/core.h"
#include "router/router.h"

void do_http_service(int sock) {
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
    free_http_request(req);
    free_http_response(res);
  }

  close(sock);
}
