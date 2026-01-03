
#include <netinet/in.h>

typedef void (*ServerFunc)(int server_fd, void (*do_http_service)(int sock));

void serve_with_single_process_loop(int server_fd,
                                    void (*do_service)(int sock));
void serve_with_multi_process_loop(int server_fd, void (*do_service)(int sock));
void serve_with_event_loop(int server_fd, void (*do_service)(int sock));
