#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <sys/event.h> // kqueue 用のヘッダ
#endif

#include "server.h"

#define MAX_EVENTS 32
#define BUF_SIZE 8192

// クライアントごとの状態管理
struct connection_state {
  int fd;
  char buffer[BUF_SIZE];
  size_t bytes_read;
};

#if defined(__APPLE__)
void serve_with_event_loop(int server_fd, void (*do_service)(int sock)) {
  int kq = kqueue();
  if (kq == -1) {
    perror("kqueue");
    return;
  }

  // 1. Listenソケットを監視対象に登録
  struct kevent sev;
  EV_SET(&sev, server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
  if (kevent(kq, &sev, 1, NULL, 0, NULL) == -1) {
    perror("kevent register listen_fd");
    return;
  }

  struct kevent event_list[MAX_EVENTS];

  while (1) {
    // イベントの発生を待機（第2, 3引数はNULL, 0）
    int n = kevent(kq, NULL, 0, event_list, MAX_EVENTS, NULL);
    if (n == -1) {
      perror("kevent wait");
      break;
    }

    for (int i = 0; i < n; i++) {
      int curr_fd = event_list[i].ident;
      struct connection_state *state =
          (struct connection_state *)event_list[i].udata;

      // 新規接続
      if (curr_fd == server_fd) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1)
          continue;

        printf("Accepted new connection: fd=%d\n", client_fd);

        // ノンブロッキングに設定
        fcntl(client_fd, F_SETFL, O_NONBLOCK);

        struct connection_state *new_state =
            calloc(1, sizeof(struct connection_state));
        new_state->fd = client_fd;

        // その場ですぐにクライアントを登録
        struct kevent client_ev;
        EV_SET(&client_ev, client_fd, EVFILT_READ, EV_ADD, 0, 0, new_state);
        kevent(kq, &client_ev, 1, NULL, 0, NULL);
      }
      // データ受信
      else if (event_list[i].filter == EVFILT_READ) {
        ssize_t bytes = recv(curr_fd, state->buffer + state->bytes_read,
                             sizeof(state->buffer) - state->bytes_read - 1, 0);

        printf("Received %zd bytes on fd=%d\n", bytes, curr_fd);
        if (bytes > 0) {
          state->bytes_read += bytes;
          state->buffer[state->bytes_read] = '\0';

          // HTTPリクエストの終端を確認
          if (strstr(state->buffer, "\r\n\r\n") != NULL) {
            // 文字列が完成したので、サービスロジックを呼び出す
            printf("HTTP request complete on fd=%d\n", curr_fd);

            // do_service が、内部で curr_fd からの読み取りを前提としている。
            // ソケットは lseek で位置を読み取位置を変更できないため、
            // do_service に文字列を渡せるようにリファクタリングする必要がある。
            // do_service(curr_fd);

            // 動作確認のために、print するに留める。
            printf("Request Data:\n%s\n", state->buffer);
            close(curr_fd);
            free(state);
          }
        } else if (bytes <= 0) {
          // エラーまたは切断
          close(curr_fd);
          free(state);
        }
      }
    }
  }
}
#else
void serve_with_event_loop(int server_fd, void (*do_service)(int sock)) {
  return;
}
#endif
