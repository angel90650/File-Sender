/* Angel Castro
* cs176A HW1
* server_c_udp.c
* References:
*   1. UDP C Server example from CMU
*     link: "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/udpserver.c"
*     usage: socket setup, setting up address, basic server setup functions
*
*   2. UDP Socket Set Timeout from stackoverflow
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
*     usage: how to use fopen, fread, sendto, recvfrom, setsockopt
*/


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>

#define BUFSIZE 1024

int sockfd;
FILE *fp;
int clientlen;
int counter = 0;
struct sockaddr_in clientaddr;
struct sockaddr_in serveraddr;
struct timeval timeout;

void sendack();
void recvudp(char * buf, int size);
void settimeout(int sec, int usec);
void getfilename(char * buf, char * output_file);
bool recvack();
void sendudp(char * buf);
void sendreliable(char * buf);
void calculatepackets(int * packets, int * size);
void error(char * msg);

int main(int argc, char **argv) {
  int portno;
  struct hostent *hostp;
  char buf[BUFSIZE];
  char len[BUFSIZE];
  char output_file[50];
  char *hostaddrp;
  int optval, rtn, num_packets, size;

  if(argc != 2)
    error("Invalid port number.\n");

  portno = atoi(argv[1]);
  if(portno < 0 || portno > 65535)
    error("Invalid port number.\n");

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sockfd < 0)
    error("Failed getting instructions from the client.\n");

  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  if(bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
    error("Failed getting instructions from the client.\n");

  clientlen = sizeof(clientaddr);
  while(1){
    while(1){
      settimeout(0, 0);
      bzero(buf, BUFSIZE);
      bzero(len, BUFSIZE);
      recvudp(len, BUFSIZE);
      sendack();
      settimeout(0, 500000);
      recvudp(buf, BUFSIZE);
      if(strcmp(buf, len) == 0){} /* Length was resent, discard and repeat */
      else{
        sendack();
        break;
      }

      if(atoi(len) == strlen(buf)){} /* Recieved all bytes */
      break;
    }

    if(atoi(len) == strlen(buf)){} /* Recieved all bytes */
    else
      error("Failed getting instructions from the client.\n");

    /* command recieved correctly */

    if((system(buf)) < 0)
      error("Failed getting instructions from the client.\n");

    getfilename(buf, output_file);

    if((fp = fopen(output_file, "r")) == NULL)
      error("Failed getting instructions from the client.\n");

    calculatepackets(&num_packets, &size);

    /* Send file size */
    settimeout(1, 0);
    bzero(buf, BUFSIZE);
    sprintf(buf, "%d", size);
    sendreliable(buf);

    while(num_packets > 0){
      bzero(buf, BUFSIZE);
      rtn = fread(buf, BUFSIZE - 1, 1, fp);
      if(rtn < 0)
        error("File transmission failed\n");
      sendreliable(buf);
      num_packets--;
    }

    fclose(fp);
  }
  close(sockfd);
  return 0;
}/* end main */


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
    error("Failed getting instructions from the client.\n");
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
    error("File transmission failed\n");
}

void sendudp(char * buf){
  int n;
  n = sendto(sockfd, buf, strlen(buf), 0,  (struct sockaddr *) &clientaddr, clientlen);
  if (n < 0)
    error("File transmission failed\n");
}


void sendack(){
  char ack[] = "ack";
  int rtn;
  rtn = sendto(sockfd, ack, strlen(ack) + 1, 0,(struct sockaddr *) &clientaddr, clientlen);
  if (rtn < 0)
    error("Failed getting instructions from the client.\n");
}

void calculatepackets(int * packets, int * size){
  if((fseek(fp, 0L, SEEK_END)) < 0)
    error("File transmission failed\n");
  *size = ftell(fp);
  rewind(fp);
  *packets = *size/(BUFSIZE - 1);
  if(*size != (*packets)*(BUFSIZE - 1))
  *packets += 1;
}

bool recvack(){
  int rtn = 0;
  char ack[10];
  while(rtn == 0){
    rtn = recvfrom(sockfd, ack, 10, 0, (struct sockaddr *) &clientaddr, &clientlen);
  }
  if(rtn < 0){
    if(errno == EAGAIN || errno == EWOULDBLOCK) /* recvfrom timed out */
    return false;
    else
      error("File transmission failed\n");
  }
  if(strcmp("ack", ack) == 0) /* Recieved ack */
  return true;
  else
  return false;
}

void recvudp(char * buf, int size){
  int n;
  n = recvfrom(sockfd, buf, size, 0, (struct sockaddr *) &clientaddr, &clientlen);
  if(n < 0){
    if(errno == EAGAIN){} /* recvfrom timed out */
    else
      error("Failed getting instructions from the client.\n");
  }
}

void settimeout(int sec, int usec){
  timeout.tv_sec = sec;
  timeout.tv_usec = usec;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
}
