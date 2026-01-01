#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "server.h"

// 子プロセスが終了すると OS が SIGCHLD を親に送信
void sigchld_handler(int sig) {
  while (waitpid(-1, NULL, WNOHANG) > 0) {
  }
}

void serve_with_multi_process_loop(int server_fd,
                                   void (*do_service)(int sock)) {
  signal(SIGCHLD, sigchld_handler);

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  while (1) {
    int sock; // 通信用ソケット（server_fd は待ち受け用ソケット）
    sock = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

    pid_t pid = fork();
    if (pid < 0)
      exit(1);
    if (pid == 0) { /* child process */
      do_service(sock);
      close(sock);
      exit(0);
    }
    printf("fork process %d\n", pid);
    close(sock);
  }
}
