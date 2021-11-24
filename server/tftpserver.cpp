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
#include "tftp.cpp"

#define SERV_UDP_PORT   51709 // REPLACE WITH YOUR PORT NUMBER

char *progname;

int main(int argc, char *argv[]) {
	std::cout << "in main" << std::endl;
	if (argc != 1) {
		printf("%s: invalid number of arguments\n",progname);
		exit(1);
    }
    
    int sockfd;

	struct sockaddr_in serv_addr;

	progname=argv[0];

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s: can't open datagram socket\n",progname);
		exit(2); 
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_UDP_PORT);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
		printf("%s: can't bind local address\n",progname);
		exit(3);
	}

	int n, clilen;
    char buffer[MAXMESG];
    struct sockaddr pcli_addr;

    for ( ; ; ) {
		std::cout << "am in loop" << std::endl;
        clilen = sizeof(struct sockaddr);

        n = recvfrom(sockfd, buffer, MAXMESG, 0, &pcli_addr, (socklen_t*)&clilen);
		std::cout << "received something" << std::endl;
		if (n < 0) {
			printf("%s: recvfrom error\n",progname);
			exit(4);
		} else {
			std::cout << "no errors in recvfrom" << std::endl;
		}

		//char tempMesg[MAXMESG] = nullptr;
		for (int i = 0; i < MAXMESG; ++i) {
			char currentValue = static_cast<char>(ntohs(buffer[i]));
			std::cout<< "currentValue: " << currentValue <<std::endl;

		}


		std::cout<< "whole buffer: ";
		for (int i = 0; i < MAXMESG; ++i) {
			std::cout<< buffer[i];
		}
		std::cout<<std::endl;

        unsigned short op = buffer[1];
		std::cout<< "op: " << op <<std::endl;
        char *bufpoint = buffer + 2;
		std::cout<< "bufpoint: " << *bufpoint <<std::endl;
        char *filename;
        strcpy(filename, bufpoint);
		std::cout<< "filename: " << *filename <<std::endl;

		std::cout << "checking if op is RRQ or WRQ" << std::endl;
        if (op == RRQ) {
			std::cout<< "op is RRQ" <<std::endl;
            // if RRQ, call tftp shared sending function
            tftp::SendMessage(sockfd, (struct sockaddr *) &serv_addr, &pcli_addr, filename);
        }
        if (op == WRQ) {
			std::cout<< "op is WRQ"<<std::endl;
            // if WRQ, send ACK0 and call tftp shared receiving function
            // BuildAckMessage()
            // sendto()
            tftp::ReceiveMessage(sockfd, &pcli_addr, (struct sockaddr *) &serv_addr);
        }
		std::cout<< "op was neither"<<std::endl;
    }

    return 0;
}



// dg_echo(sockfd);

/*
void dg_echo(int sockfd) {
	struct sockaddr pcli_addr;
	int n, clilen;
	char mesg[MAXMESG];

	for ( ; ; ) {
		clilen = sizeof(struct sockaddr);

		n = recvfrom(sockfd, mesg, MAXMESG, 0, &pcli_addr, (socklen_t*)&clilen);

		if (n < 0) {
			printf("%s: recvfrom error\n",progname);
			exit(3);
		}

		// while looping for any OPs, receives some OP
		// checks OP type and sees its a read request
		// use sendTo to send a file to the client
		// once packet is sent wait for acknowledgement
		// after acknowledgement, return to loop


if (n.OPCode == "RRQ") {
		    if (sendto(sockfd, mesg, n, 0, &pcli_addr, clilen) != n) {
		   		printf("%s: sendto error\n",progname);
		   		exit(4);
		   	}
		   	// bool receivedAck
		 	// set timer, approx 3 RTT
		 	while(!recievedAck) {
		 		m = recvfrom(sockfd, mesg, MAXMESG, 0, &pcli_addr, (socklen_t*)&clilen);
				if (m < 0) {
					printf("%s: recvfrom error\n",progname);
					exit(5);
				}
				else
				if (m did recieve an ack) {
					receivedAck = true;
				}
				if (timer is done) {
					reset timer
					if (sendto(sockfd, mesg, n, 0, &pcli_addr, clilen) != n) {
		   				printf("%s: sendto error\n",progname);
		   				exit(4);
		   			}
				}
		 	}

		 }




		if (sendto(sockfd, mesg, n, 0, &pcli_addr, clilen) != n) {
			printf("%s: sendto error\n",progname);
			exit(4);
		}
	}
}
*/