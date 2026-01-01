#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "handler.h"

extern char dir_path[PATH_MAX + 1];

void handle_get_file(struct HTTPRequest *req, struct HTTPResponse *res) {
  struct dirent *ent;

  char file_path[PATH_MAX + 1];
  strcpy(file_path, dir_path);
  char *filename = req->path + strlen("/files/");
  // Validate filename to prevent directory traversal
  if (strstr(filename, "/") != NULL || strstr(filename, "..") != NULL) {
    res->status_code = 400;
    strcpy(res->reason_phrase, "Bad Request");
    puts("prevent directory traversal");
    return;
  }
  strncat(file_path, filename, PATH_MAX - strlen(file_path));

  int fd = open(file_path, O_RDONLY);
  if (fd < 0) {
    res->status_code = 404;
    strcpy(res->reason_phrase, "Not Found");
    return;
  }

  struct stat file_stat;
  if (stat(file_path, &file_stat) == -1) {
    close(fd);
    res->status_code = 500;
    strcpy(res->reason_phrase, "Internal Server Error");
    puts("file stat error");
    return;
  };

  res->body = malloc(file_stat.st_size);
  if (!res->body) {
    close(fd);
    res->status_code = 500;
    strcpy(res->reason_phrase, "Internal Server Error");
    puts("Error: malloc for res->body");
    return;
  }

  ssize_t size = read(fd, res->body, file_stat.st_size);
  if (size == -1 || size != file_stat.st_size) {
    close(fd);
    res->status_code = 500;
    strcpy(res->reason_phrase, "Internal Server Error");
    puts("Error: read file");
    return;
  }

  close(fd);

  res->status_code = 200;
  strcpy(res->reason_phrase, "OK");
  char buf[256];
  sprintf(buf, "%zd", size);
  append_response_header(res, CONTENT_LENGTH_KEY, buf);
  append_response_header(res, CONTENT_TYPE_KEY, CONTENT_TYPE_BINARY);
}

void handle_post_file(struct HTTPRequest *req, struct HTTPResponse *res) {
  struct dirent *ent;

  char file_path[PATH_MAX + 1];
  strcpy(file_path, dir_path);
  char *filename = req->path + strlen("/files/");
  // Validate filename to prevent directory traversal
  if (strstr(filename, "/") != NULL || strstr(filename, "..") != NULL) {
    res->status_code = 400;
    strcpy(res->reason_phrase, "Bad Request");
    puts("prevent directory traversal");
    return;
  }
  strncat(file_path, filename, PATH_MAX - strlen(file_path));

  if (access(file_path, F_OK) == 0) {
    res->status_code = 409;
    strcpy(res->reason_phrase, "Conflict");
    append_response_header(res, CONTENT_LENGTH_KEY, "0");
    return;
  }

  FILE *fp = fopen(file_path, "w");
  if (fp == NULL) {
    perror(file_path);
    res->status_code = 500;
    strcpy(res->reason_phrase, "Internal Server Error");
    append_response_header(res, CONTENT_LENGTH_KEY, "0");
    return;
  }

  int content_length = 0;
  int location = find_header_location(req->header_fields, req->header_count,
                                      CONTENT_LENGTH_KEY);
  if (location < 0) {
    res->status_code = 400;
    strcpy(res->reason_phrase, "Bad Request");
    return;
  }
  size_t s = fwrite(req->body, 1, atoi(req->header_fields[location].value), fp);

  res->status_code = 201;
  strcpy(res->reason_phrase, "Created");
  append_response_header(res, CONTENT_LENGTH_KEY, "0");
}
