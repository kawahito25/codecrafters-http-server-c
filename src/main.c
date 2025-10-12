#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "http/http.h"
#include "servers/server.h"

struct {
  const char* name;
  ServerFunc server_func;
} server_func_table[] = {
    {"single_process_server_loop", serve_with_single_process_loop},
    {"serve_with_multi_process_loop", serve_with_multi_process_loop},
    {NULL, NULL}  // 終端用
};

int main(int argc, char* argv[]) {
  int opt;
  ServerFunc server_func = serve_with_multi_process_loop;

  while ((opt = getopt(argc, argv, "m:")) != -1) {
    switch (opt) {
      case 'm':
        for (int i = 0; server_func_table[i].name != NULL; i++) {
          if (strcmp(optarg, server_func_table[i].name) == 0) {
            server_func = server_func_table[i].server_func;
          }
        }
        break;
      case '?':
        break;
    }
  }

  // Disable output buffering
  setbuf(stdout, NULL);
  setbuf(stderr, NULL);

  int server_fd;

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

  server_func(server_fd, do_http_service);
  close(server_fd);

  return 0;
}
