#include "server.h"

int check_method(char *method, char *status_line, int line_len)
{
  char pattern[MAX_REGEX_LEN];
  regex_t expression;

  snprintf(pattern, MAX_REGEX_LEN, "%s /[a-zA-z]*", method);

  int err = regcomp(&expression, pattern, 0);
  if (err)
  {
    fprintf(stderr, "regex compilation failed\n");
    exit(1);
  }

  int match = regexec(&expression, status_line, 0, NULL, 0);
  regfree(&expression);

  if (!match)
  {
    return 1;
  }
  else if (match == REG_NOMATCH)
  {
    return 0;
  }

  return -1;
}

int check_file(const char *path)
{
  struct stat st;
  if (stat(path, &st) < 0)
  {
    return -1;
  }

  return S_ISREG(st.st_mode);
}

int send_file_response(int accept_fd, char *file_path)
{
  char *header = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";

  int bytes_sent = send(accept_fd, header, strlen(header), 0);
  if (bytes_sent == -1)
  {
    perror("send: ");
  }

  int fd = open(file_path, O_RDONLY);

  struct stat st;
  memset(&st, 0, sizeof st);

  if (fstat(fd, &st))
  {
    perror("fstat: ");
    exit(1);
  }

  ssize_t n = 0;

  for (size_t i = st.st_size; i != 0; i = i - n)
  {
    n = sendfile(accept_fd, fd, NULL, i);
    if (n == -1)
    {
      perror("fstat: ");
      exit(1);
    }
  }

  close(fd);
  return 1;
}

int send_response(int accept_fd, char *status_code, char *reason_phrase, char *data)
{
  char response[MAX_RES_SIZE];
  memset(response, '\0', sizeof response);

  const char *http_v = "HTTP/1.0";

  snprintf(response, MAX_RES_SIZE, "%s %s %s\r\n\r\n%s\r\n", http_v, status_code, reason_phrase, data);

  int bytes_sent = send(accept_fd, response, strlen(response), 0);
  if (bytes_sent == -1)
  {
    perror("send: ");
  }

  return 1;
}

void *handle_request(void *arg)
{
  struct timeval start, end;
  double processing_time;
  gettimeofday(&start, NULL);

  char request[MAX_REQ_SIZE];
  memset(request, '\0', sizeof request);

  int accept_fd = *((int *)arg);

  int bytes_received = recv(accept_fd, request, MAX_REQ_SIZE, 0);
  if (bytes_received == -1)
  {
    perror("recv: ");
  }

  char *method, *URI;
  char *status_line = strtok(request, "\r\n");

  int regex_result = check_method("GET", status_line, strlen(status_line));
  if (regex_result == -1)
  {
    fprintf(stderr, "failed executing regular expression\n");
    exit(1);
  }

  method = strtok(status_line, " ");
  URI = strtok(NULL, " ");

  if (!regex_result)
  {
    // 405
    send_response(accept_fd, "405", "Method Not Allowed", "Method Not Allowed");
  }
  else
  {
    if (strlen(URI) == 1)
    {
      // respond with index.html
      send_file_response(accept_fd, "./app/index.html");
    }
    else
    {
      // search the file recursively if needed
      char file_path[1024];
      memset(file_path, '\0', sizeof file_path);

      strncpy(file_path, "./app", 1024);
      strncat(file_path, URI, 1000);

      if (check_file(file_path) == 1)
      {
        send_file_response(accept_fd, file_path);
      }
      else
      {
        // 404
        send_response(accept_fd, "404", "Not Found", "<h1>Resource Not Found!</h1>");
      }
    }
  }

  gettimeofday(&end, NULL);
  processing_time = (end.tv_sec - start.tv_sec) * 1000;
  processing_time += (end.tv_usec - start.tv_usec) / 1000.0;

  printf("%s %s %.4f ms\n", method, URI, processing_time);

  // fflush(stdout);
  close(accept_fd);
}

void server_start(int *socketfd, char *port)
{
  int yes = 1;
  struct addrinfo hints, *server_info, *temp;

  // load basic server info in the addrinfo struct
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  // translate to socket addresses
  if (getaddrinfo(NULL, port, &hints, &server_info) != 0)
  {
    perror("getaddrinfo: ");
    exit(1);
  }

  int temp_socket_fd;
  // loop through each result and bind on the first one
  for (temp = server_info; temp != NULL; temp = temp->ai_next)
  {
    if ((temp_socket_fd = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol)) == -1)
    {
      perror("socket: ");
      continue;
    }

    if (setsockopt(temp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
      perror("setsockopt: ");
      exit(1);
    }

    if (bind(temp_socket_fd, temp->ai_addr, temp->ai_addrlen) == -1)
    {
      perror("bind: ");
      close(temp_socket_fd);
      continue;
    }

    break;
  }

  freeaddrinfo(server_info);
  // check if bind was successfull
  if (temp == NULL)
  {
    fprintf(stderr, "failed to bind on port %s\n", port);
    exit(1);
  }

  // listen on the specified port with 10 as backlog
  if (listen(temp_socket_fd, 10) == -1)
  {
    perror("listen: ");
    exit(1);
  }

  *socketfd = temp_socket_fd;
  printf("server listening on port %s\n", port);
}
