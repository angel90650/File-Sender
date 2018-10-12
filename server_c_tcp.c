/* Angel Castro
* cs176A HW1
* server_c_tcp.c
* References:
*   1. TCP C Server example from CMU
*     link: "https://www.cs.cmu.edu/afs/cs/academic/class/15213-f99/www/class26/tcpserver.c"
*     usage: socket setup, setting up address, basic server setup functions
*
*   2. Length of file from stackoverflow
*     link: "https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c"
*     usage: finding length of output file
*
*   3. Linux man pages
*     link: "https://linux.die.net/man/"
*     usage: how to use fopen, fread, send, recv, fseek
*
*
*/


#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 1024

FILE * fp;

void calculatepackets(int * packets, int * size);
void error(char * msg);

int main(int argc, char const *argv[]){
	int server_fd, new_socket, PORT;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024];
	char output_file[50];
	char * a;
	int packets, size, return_val;

	a = (char *) malloc(50);
	PORT = atoi(argv[1]);
	if(PORT < 0 || PORT > 65535)
	  error("Invalid port number.\n");

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	  error("Failed getting instructions from the client.\n");

	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	  error("Failed getting instructions from the client.\n");

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	  error("Failed getting instructions from the client.\n");

	if (listen(server_fd, 3) < 0)
	  error("Failed getting instructions from the client.\n");
	while(1){
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
		  error("Failed getting instructions from the client.\n");

		bzero(buffer, BUFSIZE);
		/* recieve command */
		return_val = recv(new_socket, buffer, 1024, 0);
		if(return_val < 0)
		  error("Failed getting instructions from the client.\n");

		return_val = system(buffer);
		if(return_val < 0)
		  error("Failed getting instructions from the client.\n");

		a = strtok(buffer, " ");
		while((a = strtok(NULL, " ")) != NULL){
			sscanf(a, "%s", output_file);
		}

		if(output_file == NULL)
		  error("Failed getting instructions from the client.\n");

		if((fp = fopen(output_file, "r")) == NULL)
		  error("File transmission failed.\n");

		calculatepackets(&packets, &size);

		while(packets > 0){
			bzero(buffer, BUFSIZE);
			return_val = fread(buffer, BUFSIZE - 1, 1, fp);
			if(return_val < 0)
			  error("File transmission failed.\n");
			else{
				return_val = send(new_socket, buffer, BUFSIZE, 0);
				if(return_val < 0)
				  error("File transmission failed.\n");
				packets--;
			}
		}
		fclose(fp);
		close(new_socket);
	}
	close(server_fd);
	return 0;
}


void calculatepackets(int * packets, int * size){
	if((fseek(fp, 0L, SEEK_END)) < 0)
	  error("File transmission failed.\n");
	*size = ftell(fp);
	rewind(fp);
	*packets = *size/(BUFSIZE - 1);
	if(*size != (*packets)*(BUFSIZE - 1))
	*packets += 1;
}

void error(char * msg) {
	printf("%s", msg);
	exit(EXIT_FAILURE);
}
