#include <stdio.h>       //printf
#include <stdlib.h>      //atoi
#include <string.h>      //strlen
#include <sys/socket.h>  //socket
#include <arpa/inet.h>   //inet_addr
#include <unistd.h>
#include <sys/select.h>  //select

int main(int argc, char *argv[]) {
  if (argc != 3) {
    puts("Usage: ./sync <n-requests> <server port> \n");
    return 0;
  }

  int N = atoi(argv[1]);
  int port = atoi(argv[2]);
  int socks[N];
  struct sockaddr_in server;
  char server_reply[BUFSIZ];

  // populate address information for server
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  // Setup the fd_set
  // fd_set is the set of file descripters that select()
  // will watch to see if they can be read() from
  fd_set socket_fd_set;
  FD_ZERO(&socket_fd_set);
  fd_set temp_fd_set;
  FD_ZERO(&temp_fd_set);


  char *request_string =
      "GET /foo.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";

  // Loop to create sockets and connect to remote server,
  // issue request then add to FD_SET
  for (int i = 0; i < N; i++) {
    // create socket and establish connection with server
    socks[i] = socket(AF_INET, SOCK_STREAM, 0);

    if (socks[i] == -1)
      puts("Could not create socket");
    else
      puts("Socket created");

    // Connect to remote server
    if (connect(socks[i], (struct sockaddr *)&server, sizeof(server)) < 0) {
      perror("connect failed. Error");
      return 1;
    }
    // write request to server
    if (send(socks[i], request_string, strlen(request_string), 0) < 0) {
      puts("Send failed");
      return 1;
    }
    FD_SET(socks[i], &socket_fd_set);
    FD_SET(socks[i], &temp_fd_set);
  }

  int loop = 0;
  while (select(FD_SETSIZE, &socket_fd_set, NULL, NULL, NULL)) {
    printf("Select gave some new events! Loop: %d\n", loop++);

    fd_set temp_fd_set2;
    FD_ZERO(&temp_fd_set2);

    for (int i = 0; i < N; i++) {
      // Sockets that are ready to be read are removed 
      // from the fd_set
      if (FD_ISSET(socks[i], &socket_fd_set)) {
        printf("READING FROM SOCKET: %d\n", socks[i]);
        recv(socks[i], server_reply, BUFSIZ, 0);
        // puts(server_reply);
        close(socks[i]);
      } else if (FD_ISSET(socks[i], &temp_fd_set)){
        printf("FOUND SOCKET NOT READY FOR READING: %d\n", socks[i]);
        FD_SET(socks[i], &temp_fd_set2);
      }
    }

    // Put back file descriptors that still need
    // to be waited on
    FD_ZERO(&socket_fd_set);
    FD_ZERO(&temp_fd_set);
    int nset = 0;
    for (int i = 0; i < N; i++) 
      if (FD_ISSET(socks[i], &temp_fd_set2)){
        FD_SET(socks[i], &socket_fd_set);
        FD_SET(socks[i], &temp_fd_set);
        nset++;
      }
    if (nset == 0) break;
        
  }

  return 0;
}