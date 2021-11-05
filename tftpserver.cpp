#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>


#define SERV_UDP_PORT 51709
#define MAXMESG 2048

char *progname;

int main(int argc, char *argv[]) {
	int sockfd;
	
	struct sockaddr_in serv_addr;

	progname = argv[0];

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		 printf("%s: can't open datagram socket\n",progname);
		 exit(1); 
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_UDP_PORT);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	    printf("%s: can't bind local address\n",progname);
		exit(2);
	}

    struct sockaddr pcli_addr;
    int n; 
    socklen_t clilen;
	char mesg[MAXMESG];

    while (true) {
        clilen = sizeof(struct sockaddr);
        n = recvfrom(sockfd, mesg, MAXMESG, 0, &pcli_addr, &clilen);
        if (n < 0) {
			printf("%s: recvfrom error\n",progname);
			exit(3);
		}

        fputs(mesg, stdout);
    }

    exit(0);
}