## Minimal Web Server in C

To compile source files run `make` or `gcc main.c -o main server.c`  
Then run `./main 8080` to start the server on port `8080`  
Test with `localhost:8080/client/home.html` or `localhost:8080`  

## Some Context

The program listens to connections using the socket syscalls  
It creates a new thread for each request to the server

## References
 - https://beej.us/guide/bgnet/html/split/what-is-a-socket.html
 - https://www.reddit.com/r/C_Programming/comments/kbfa6t/building_a_http_server_in_c/
