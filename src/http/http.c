#include "http.h"

#include <unistd.h>

#include "core/core.h"
#include "router/router.h"

void do_http_service(int sock) {
  FILE *inf = fdopen(sock, "r");
  FILE *outf = fdopen(sock, "w");

  struct HTTPRequest *req = init_http_request();
  struct HTTPResponse *res = init_http_response();

  read_request(req, inf);
  handle_request(req, res);
  append_common_response_headers(req, res);
  write_response(res, outf);
  fclose(outf);

  free_http_request(req);
  free_http_response(res);

  close(sock);
}
