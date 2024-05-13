#include "server.h"
#include <pthread.h>

void main(int argc, char **argv)
{
  system("clear");
  if (argc < 2)
  {
    fprintf(stderr, "usage: main <port>\n");
    exit(1);
  }

  int socketfd, accept_fd;

  struct sockaddr_storage client_addr;
  socklen_t socket_addr_len = sizeof client_addr;

  server_start(&socketfd, argv[1]);

  while (1)
  {
    accept_fd = accept(socketfd, (struct sockaddr *)&client_addr, &socket_addr_len);
    if (accept_fd == -1)
    {
      perror("accept: ");
      continue;
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_request, (void *)&accept_fd);
    pthread_detach(thread_id);
  }
}