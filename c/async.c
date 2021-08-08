#include <stdio.h>       //printf
#include <stdlib.h>      //atoi
#include <string.h>      //strlen
#include <sys/socket.h>  //socket
#include <arpa/inet.h>   //inet_addr
#include <unistd.h>
#include <sys/select.h>  //select

int main(int argc, char *argv[]) {
  if (argc != 2) {
    puts("Usage: ./sync <n> \n");
    return 0;
  }

  int N = atoi(argv[1]);
  int socks[200];
  int max_fd = 0;
  struct sockaddr_in server;
  char server_reply[2000];

  // populate address information for server
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons(8888);

  // Setup the fd_set
  fd_set socket_fd_set;
  int retval;
  int fds_remaining;
  FD_ZERO(&socket_fd_set);

  char *request_string =
      "GET /foo.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";

  // Loop to create sockets and connect to remote server,
  // issue request then add to FD_SET
  for (int i = 0; i < N; i++) {
    // create socket and establish connection with server
    socks[i] = socket(AF_INET, SOCK_STREAM, 0);
    max_fd = max_fd > socks[i] ? max_fd : socks[i];

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
  }

  while (retval = select(max_fd + 1, &socket_fd_set, NULL, NULL, NULL)) {
    puts("Select gave some new events!\n");
    for (int i = 0; i < N; i++) {
      // Sockets that are ready to be read are removed 
      // from the fd_set
      if (FD_ISSET(socks[i], &socket_fd_set)) {
        recv(socks[i], server_reply, 2000, 0);
        puts(server_reply);
        close(socks[i]);
      } else {
        // FD_SET(socks[i], &socket_fd_set);
      }
    }
    // Now need to put back the ones that still need to be read from
    FD_ZERO(&socket_fd_set);

  }


  return 0;
}