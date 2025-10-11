#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http.h"
#include "routing.h"

void output_response(struct HTTPResponse* res, FILE* out);

int main() {
  // Disable output buffering
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  printf("Logs from your program will appear here!\n");

  int server_fd, client_addr_len;
  struct sockaddr_in client_addr;

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    printf("Socket creation failed: %s...\n", strerror(errno));
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
      0) {
    printf("SO_REUSEADDR failed: %s \n", strerror(errno));
    return 1;
  }

  struct sockaddr_in serv_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(4221),
      .sin_addr = {htonl(INADDR_ANY)},
  };

  if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) {
    printf("Bind failed: %s \n", strerror(errno));
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    printf("Listen failed: %s \n", strerror(errno));
    return 1;
  }

  printf("Waiting for a client to connect...\n");
  client_addr_len = sizeof(client_addr);

  int sock;  // 通信用ソケット（server_fd は待ち受け用ソケット）
  sock = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
  printf("Client connected\n");

  FILE* inf = fdopen(sock, "r");
  FILE* outf = fdopen(sock, "w");

  struct HTTPRequest* req = read_request(inf);
  struct HTTPResponse* res = init_http_response();

  handle_request(req, res);
  output_response(res, outf);

  free_http_request(req);
  free_http_response(res);
  fclose(outf);

  close(server_fd);

  return 0;
}

void output_response(struct HTTPResponse* res, FILE* outf) {
  fprintf(outf, "HTTP/1.1 %d %s", res->status_code, res->reason_phrase);
  fputs("\r\n", outf);  // CRLF that marks the end of the status line

  for (int i = 0; i < res->header_count; i++) {
    fputs(res->header_fields[i].key, outf);
    fputs(": ", outf);
    fputs(res->header_fields[i].value, outf);
    fputs("\r\n", outf);
  }
  fputs("\r\n", outf);  // CRLF that marks the end of the headers

  if (res->body != NULL) {
    fputs(res->body, outf);  // TODO: \0 がない場合に対応できない
  }
}
