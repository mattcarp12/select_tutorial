#include <stdio.h>       //printf
#include <stdlib.h>      //atoi
#include <string.h>      //strlen
#include <sys/socket.h>  //socket
#include <arpa/inet.h>   //inet_addr
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    puts("Usage: ./sync <n> \n");
    return 0;
  }

  int N = atoi(argv[1]);
  int sock;
  struct sockaddr_in server;
  char server_reply[2000];

  // populate address information for server
  server.sin_addr.s_addr = inet_addr("127.0.0.1");
  server.sin_family = AF_INET;
  server.sin_port = htons(8888);

  // create socket and establish connection with server
  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock == -1)
    puts("Could not create socket");
  else
    puts("Socket created");

  // Connect to remote server
  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
    perror("connect failed. Error");
    return 1;
  }

  // construct http request
  char *request_string =
      "GET /foo.txt HTTP/1.1\r\nHost: localhost\r\nConnection: close\r\n\r\n";

  // Write request and read response
  for (int i = 0; i < N; i++) {
    if (send(sock, request_string, strlen(request_string), 0) < 0) {
      puts("Send failed");
      return 1;
    }

    // Receive a reply from the server
    if (recv(sock, server_reply, 2000, 0) < 0) {
      puts("recv failed");
    }

    puts("Server reply :");
    puts(server_reply);
  }

  close(sock);
  return 0;
}