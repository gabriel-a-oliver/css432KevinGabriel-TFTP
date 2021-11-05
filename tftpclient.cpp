#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define SERV_UDP_PORT 51709
#define SERV_HOST_ADDR "172.18.0.31"
#define MAXLINE 512

char *progname;

int main(int argc, char *argv[]) {
    int sockfd;
	
	struct sockaddr_in cli_addr, serv_addr;
	progname = argv[0];

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	
	serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port = htons(SERV_UDP_PORT);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s: can't open datagram socket\n",progname);
		exit(1);
	}

	bzero((char *) &cli_addr, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port = htons(0);
	
	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
		printf("%s: can't bind local address\n",progname);
		exit(2);
	}

    char sendline[] = "test"; 
    int n = strlen(sendline);

    if (sendto(sockfd, sendline, n, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != n) {
		printf("%s: sendto error on socket\n",progname);
		exit(3);
	}

	close(sockfd);
	exit(0);
}