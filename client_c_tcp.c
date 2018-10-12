/* Angel Castro
* cs176A HW1
* client_c_tcp.c
* References:
*   1. TCP C Client Example from CMU
*     link: "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpclient.c"
*     usage: socket setup, setting up address, basic server setup functions
*
*   2. Linux man pages
*     link: "https://linux.die.net/man/"
*     usage: how to use fopen, fwrite, send, recv, gethostbyname
*
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFSIZE 1024

void error(char *msg);

int main(int argc, char **argv) {
  int sockfd, portno, n,bytes_rcvd;
  FILE * fp;
  char * portnos;
  struct sockaddr_in serveraddr;
  struct hostent *server;
  char *hostname;
  char file_name[50];
  char buf[BUFSIZE];
  portnos = malloc(10);
  hostname = malloc(20);
  printf("Enter server name or IP address: ");
  fgets(buf, 30, stdin);
  sscanf(buf, "%s",hostname);
  server = gethostbyname(hostname);
  if (server == NULL)
    error("Could not connect to server.\n");

  printf("Enter port: ");
  bzero(buf, BUFSIZE);
  fgets(buf, 10, stdin);
  sscanf(buf, "%s",portnos);
  portno = atoi(portnos);
  if(portno < 0 || portno > 65535)
    error("Invalid port number.\n");

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd <= 0)
    error("Could not connect to server.\n");

  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
  serveraddr.sin_port = htons(portno);

  if (connect(sockfd,(struct sockaddr *)  &serveraddr, sizeof(serveraddr)) < 0)
    error("Could not connect to server.\n");

  /* get command from the user */
  printf("Enter command: ");
  bzero(buf, BUFSIZE);
  if(fgets(buf, BUFSIZE, stdin) == NULL)
    error("Failed to send command. Terminating\n");
  /* send the command to the server */
  n = send(sockfd, buf, strlen(buf), 0);
  if (n < 0)
    error("Failed to send command. Terminating\n");
  sprintf(file_name, "%s", "tcpout.txt");
  fp = fopen(file_name, "w");
  if(fp == NULL)
    error("Could not fetch file.\n");
  bytes_rcvd = 0;
  n = 1;
  while(n > 0){
    bzero(buf, BUFSIZE);
    n = recv(sockfd, buf, BUFSIZE, 0);
    if(n < 0)
      error("Could not fetch file\n");
    if(n == 0)
    break;
    bytes_rcvd += strlen(buf);
    fwrite(buf, strlen(buf), 1, fp);
  }
  printf("File %s saved.\n", file_name);
  fclose(fp);
  close(sockfd);
  return 0;
}

void error(char *msg) {
  printf("%s", msg);
  exit(EXIT_FAILURE);
}
