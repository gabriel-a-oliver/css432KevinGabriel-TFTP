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
#include <typeinfo>
#include "tftp.cpp"


#define SERV_UDP_PORT 51709
#define SERV_HOST_ADDR "10.158.82.41" //REPLACE WITH SERVER IP ADDRESS // lab11: 10.158.82.41

char *progname;

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("%s: invalid number of arguments\n",progname);
		exit(1);
	}
	std::cout<< "correct number of command line arguments"<<std::endl;
	progname = argv[0];
	const char *op = argv[1];
	char *filename = argv[2];
	std::cout<< "assigned command line arguments to variables"<<std::endl;
	int sockfd;

	struct sockaddr_in cli_addr, serv_addr;

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port = htons(SERV_UDP_PORT);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s: can't open datagram socket\n",progname);
		exit(2);
	} else {
		std::cout<< "socket established"<<std::endl;
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
	bzero(buffer, sizeof(buffer));

	std::cout<< "seeing if OP is r or w" <<std::endl;
	if (op[1] == 'r') {
		std::cout<< "OP is r: " << RRQ <<std::endl;
		//*(short *) buffer = htons(RRQ);

		unsigned short RRQ = 1;
		unsigned short* opCodePtr = (unsigned short *) buffer;
		*opCodePtr = htons(RRQ);

		//bufferPointer += 1;  // not needed since you're using bufpoint below to point to filename
		// print out each byte using a for loop. using hex format, or using character

        //std::cout<< "OP code is:" << ntohs(htons(RRQ)) <<std::endl;
	} else
	if (op[1] == 'w') {
		std::cout<< "OP is w:" << WRQ <<std::endl;
		*(short *)buffer = htons(WRQ);
	} else {
		std::cout<< "neither r or w" <<std::endl;
	}

	std::cout<< "creating packet" <<std::endl;
	bufpoint = buffer + 2; // move pointer to file name
	strcpy(bufpoint, filename); // add file name to buffer
	bufpoint += strlen(filename) + 1; //move pointer and add null byte
	strcpy(bufpoint, "octet"); // add mode to buffer
	bufpoint += strlen("octet") + 1; // move pointer and add null byte

	std::cout<< "whole buffer before being sent:";
    unsigned short opNumber = ntohs(buffer[1]);
    std::cout<< opNumber; // This printing is wrong.  cout prints the bytes in ascii format, the value at buffer[1] is 1 , which is a non-printable ascii character, so you won't see anything on screen
	//Instead, print the hex value of first two bytes. should be 0,1 for RRQ, 0,2 for WRQ
	printf("%x,%x", buffer[0], buffer[1]);
	for (int i = 2; i < MAXMESG; ++i) {
		if (buffer[i] == NULL)
		{
			std::cout<< " ";
		}
		std::cout<< buffer[i];
	}
	std::cout<<std::endl;

	std::cout<< "sending packet" <<std::endl;
	int n = sendto(sockfd, buffer, bufpoint-buffer, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (n < 0) {
		printf("%s: sendto error\n",progname);
		exit(4);
	} else {
		std::cout<< "no issue sending packet" <<std::endl;
	}

	if (op[1] == 'r') {
		// if RRQ, call tftp shared receiving function
		tftp::ReceiveMessage(sockfd, (struct sockaddr *) &serv_addr, (struct sockaddr *) &cli_addr);
	} else if (op[1] == 'w') {
		// if WRQ, call tftp shared sending function (may need to receive ACK0 first)
		tftp::SendMessage(sockfd, (struct sockaddr *) &cli_addr, (struct sockaddr *) &serv_addr, filename);
	} else {
		std::cout<< "was not RRQ or WRQ" <<std::endl;
	}




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























