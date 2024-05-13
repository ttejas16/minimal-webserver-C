#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <regex.h>
#include <sys/time.h>
#include <sys/sendfile.h>
#include <fcntl.h>
// #include <pthread.h>

#define MAX_REQ_SIZE 2048
#define MAX_RES_SIZE 2048
#define MAX_REGEX_LEN 2048

void server_start(int *socketfd, char *port);

void *handle_request(void *arg);

#endif