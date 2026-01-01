#include <stdio.h>

#include "server.h"

void serve_with_single_process_loop(int server_fd,
                                    void (*do_service)(int sock)) {
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  while (1) {
    int sock; // 通信用ソケット（server_fd は待ち受け用ソケット）
    sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    printf("Client connected\n");

    do_service(sock);
  }
}
