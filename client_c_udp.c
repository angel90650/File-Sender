/* Angel Castro
* cs176A HW1
* client_c_udp.c
* References:
*   1. UDP C Client Example from CMU
*     link: "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpclient.c"
*     usage: socket setup, setting up address, basic server setup functions
*
*   2. Socket Set Timeout
*     link: "https://stackoverflow.com/questions/13547721/udp-socket-set-timeout?
*             utm_medium=organic&utm_source=google_rich_qa&utm_campaign=google_rich_qa"
*     usage: setting up timeout
*
*   3. Length of file from stackoverflow
*     link: "https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c"
*     usage: finding length of output file
*
*   4. Linux man pages
*     link: "https://linux.die.net/man/"
*     usage: how to use fopen, fwrite, sendto, recvfrom, setsockopt
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>

#define BUFSIZE 1024

int sockfd, server_len;
struct sockaddr_in serveraddr;

void getfilename(char * buf, char * output_file);
int calculatepackets(int size);
void sendack();
bool recvack();
void sendreliable(char * buf);
void recvreliable(char * buf);
int recvudp(char * buf);
void sendudp(char * buf);
void getcommand(char * buf);
void error(char * msg);

int main(int argc, char **argv) {
  int portno, n, msg_len, file_size, packets_left;
  int bytes_rcvd = 0;
  char * portnos;
  struct hostent * server;
  FILE * fp;
  char * hostname;
  char buf[BUFSIZE], pbuf[BUFSIZE];
  char len[BUFSIZE], command_buf[BUFSIZE];
  char output_file[50];
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  server_len = sizeof(struct sockaddr_in);
  portnos = malloc(10);
  hostname = malloc(20);
  printf("Enter server name or IP address: ");
  fgets(buf, 30, stdin);
  sscanf(buf, "%s",hostname);

  server = gethostbyname(hostname);
  if(server == NULL)
    error("Could not connect to server.\n");

  printf("Enter port: ");
  bzero(buf, BUFSIZE);
  fgets(buf, 10, stdin);
  sscanf(buf, "%s",portnos);
  portno = atoi(portnos);
  if(portno < 0 || portno > 65535)
    error("Invalid port number.\n");

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd <= 0)
    error("Could not connect to server.\n");


  // setup timeout
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(char *)  &timeout, sizeof (timeout));

  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
  serveraddr.sin_port = htons(portno);

  bzero(command_buf, BUFSIZE);
  getcommand(command_buf);
  msg_len = strlen(command_buf);
  bzero(buf, BUFSIZE);
  sprintf(buf, "%d", msg_len);
  /* Send length to server */
  sendreliable(buf);
  /* Send command to server */
  sendreliable(command_buf);
  getfilename(command_buf, output_file);
  sprintf(output_file, "%s", "outfile.txt");
  /* Get file size */
  bzero(buf, BUFSIZE);
  timeout.tv_sec = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(char *)  &timeout, sizeof (timeout));
  recvudp(buf);
  sendack();
  file_size = atoi(buf);
  packets_left = calculatepackets(file_size);
  if((fp = fopen(output_file, "w")) == NULL)
    error("Could not fetch file.\n");

  while(packets_left > 0){
    bzero(buf, BUFSIZE);
    recvreliable(buf);
    if(strcmp(pbuf, buf) == 0){} /* Same message resent */
    else{
      memcpy(pbuf, buf, BUFSIZE);
      fwrite(buf, strlen(buf), 1, fp);
      bytes_rcvd += strlen(buf);
      packets_left--;
    }
  }
  printf("File %s saved.\n",output_file);

  fclose(fp);
  close(sockfd);
  return 0;
}// end main



void error(char * msg) {
  printf("%s", msg);
  exit(EXIT_FAILURE);
}

void getfilename(char * buf, char * output_file){
  char * a;
  a = strtok(buf, " ");
  while((a = strtok(NULL, " ")) != NULL){
    sscanf(a, "%s", output_file);
  }

  if(output_file == NULL)
    error("Could not fetch file.\n");

}
int calculatepackets(int size){
  int packets;
  packets = size/(BUFSIZE - 1);
  if(size % (BUFSIZE - 1) != 0)
  packets++;
  return packets;
}
void getcommand(char * buf){
  printf("Enter command: ");
  bzero(buf, BUFSIZE);
  if(fgets(buf, BUFSIZE,stdin) == NULL)
    error("Failed to send command. Terminating\n");
}
void sendudp(char * buf){
  int rtn;
  rtn = sendto(sockfd, buf, strlen(buf), 0,(struct sockaddr *) &serveraddr, server_len);
  if(rtn <= 0)
    error("Failed to send command. Terminating\n");

}

void sendreliable(char * buf){
  int counter = 0;
  while(counter < 3){
    sendudp(buf);
    if(recvack() == true)
    break;
    counter++;
  }
  if(counter == 3)
    error("Failed to send command. Terminating\n");
}

bool recvack(){
  int rtn = 0;
  char ack[10];
  while(rtn == 0){
    rtn =  recvfrom(sockfd, ack, 10, 0,(struct sockaddr *) &serveraddr, &server_len);
  }
  if(rtn < 0){
    if(errno == EAGAIN || errno == EWOULDBLOCK)  /* recvack timed out */
    return false;
    else
      error("recvack failed");
  }
  if(strcmp("ack", ack) == 0) /* recieved ack */
  return true;
  else
  return false;
}
int recvudp(char * buf){
  int n;
  n =  recvfrom(sockfd, buf, BUFSIZE, 0,(struct sockaddr *) &serveraddr, &server_len);
  if(n < 0){
    if(errno == EAGAIN){} /* recvfrom timed out */
    else
      error("Could not fetch file.\n");
    return -1;
  }
  return 0;
}

void sendack(){
  char ack[] = "ack";
  int rtn;
  rtn = sendto(sockfd, ack, strlen(ack) + 1, 0,(struct sockaddr *) &serveraddr, server_len);
  if (rtn < 0)
    error("Could not fetch file.\n");
}

void recvreliable(char * buf){
  int counter = 3;
  while(counter > 0){
    if(recvudp(buf) == 0){
      sendack();
      break;
    }
    counter--;
  }
  if(counter == 0)
    error("Could not fetch file.\n");
}
