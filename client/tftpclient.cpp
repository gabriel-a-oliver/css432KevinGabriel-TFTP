#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>          // for retrieving the error number.
#include <string.h>         // for strerror function.
#include <signal.h>         // for the signal handler registration.
#include <unistd.h>

// to be moved to shared tftp file
#define RRQ	1
#define WRQ 2
#define DATA 3
#define ACK 4
#define	ERROR 5
#define MAXMESG 512

#define SERV_UDP_PORT 51709 //REPLACE WITH YOUR PORT NUMBER
#define SERV_HOST_ADDR "10.158.82.38" //REPLACE WITH SERVER IP ADDRESS

char *progname;

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("%s: invalid number of arguments\n",progname);
		exit(1);
    }
    progname = argv[0];
    const char *op = argv[1];
    const char *filename = argv[2];
    
    int sockfd;
	
	struct sockaddr_in cli_addr, serv_addr;

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port = htons(SERV_UDP_PORT);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s: can't open datagram socket\n",progname);
		exit(2);
	}

	bzero((char *) &cli_addr, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port = htons(0);

	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
		 printf("%s: can't bind local address\n",progname);
		 exit(3);
	}

    char *bufpoint; // for building packet
    char buffer[MAXMESG]; // packet that will be sent
    if (op[1] == 'r') {
        *(short *)buffer = htons(RRQ);
    }
    if (op[1] == 'w') {
        *(short *)buffer = htons(WRQ);
    }
    bufpoint = buffer + 2; // move pointer to file name
    strcpy(bufpoint, filename); // add file name to buffer
    bufpoint += strlen(filename) + 1; //move pointer and add null byte
    strcpy(bufpoint, "octet"); // add mode to buffer
    bufpoint += strlen("octet") + 1; // move pointer and add null byte

    int n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (n < 0) {
		printf("%s: sendto error\n",progname);
		exit(4);
	}

    // if RRQ, call tftp shared receiving function 
    // if WRQ, call tftp shared sending function (may need to receive ACK0 first)

	close(sockfd);

	return 0;
}

// dg_cli(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

/*
void dg_cli(int sockfd, struct sockaddr *pserv_addr, int servlen) {
	int n;
	char sendline[MAXLINE], recvline[MAXLINE + 1];

	while (fgets(sendline, MAXLINE, stdin) != NULL) {
		n = strlen(sendline); 
		if (sendto(sockfd, sendline, n, 0, pserv_addr, servlen) != n) {
			 printf("%s: sendto error on socket\n",progname);
			 exit(3);
		}

		n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
		if (n < 0) {
			 printf("%s: recvfrom error\n",progname);
			 exit(4);
		}
		
		recvline[n] = 0;
		fputs(recvline, stdout);
	}
}
*/























