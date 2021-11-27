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


char* CreateDataPacket(char* filename) {
	std::cout<< "in CreateDataPacket()"<<std::endl;
	char buffer[MAXMESG];
	bzero(buffer, sizeof(buffer));
	char* bufpoint = buffer;

	std::cout<< "OP:";
	std::cout << DATA <<std::endl;
	unsigned short htonsNum = 3;
	*buffer = htons(htonsNum);

	bufpoint = buffer + 2; // move pointer to block number
	std::cout<< "Block#:";
	unsigned short blockNumber = 1; // temporary for testing
	std::cout << blockNumber <<std::endl;
	*bufpoint = htons(blockNumber);

	std::cout<< "creating data packet" <<std::endl;
	bufpoint = buffer + 2; // move pointer to file name

	//FileData//////////////////////////////////////////////////////////////////
	//open and reading Linux commands:
	int fd = open(filename, O_RDONLY); // open text file
	std::cout<< "fd value:" << fd <<std::endl;
	if (fd < 0) {
		std::cout<< "linux open file error"<<std::endl;
		// error opening fileName
	} else {
		std::cout<< "no issue opening file"<<std::endl;
	}

	//char data[MAXDATA];
	//bzero(bufpoint, MAXDATA);
	int n = read(fd, bufpoint, MAXDATA); // read up to MAXDATA bytes
	if (n < 0) {
		std::cout<< "read error:" << n <<std::endl;
	} else {
		std::cout << "read successful:" << n << std::endl;
	}
	// if result == 0: end of file; if result < 0: error

	// create DATA packet and call sendto

	close(fd); // once finish reading whole file, close text file
	///////////////////////////////////////////////////////////////////////////

	std::cout<< "whole data buffer after being created:";
	std::cout << *buffer << *(buffer + 1);
	std::cout << *(buffer + 2) << *(buffer + 3);
	for (int i = 4; i < MAXMESG; ++i) {
		std::cout<< buffer[i];
	}
	std::cout<<std::endl << "END OF FILE DATA" << std::endl;

	return buffer;
}


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
    bzero(buffer, sizeof(buffer));
    struct sockaddr pcli_addr;

    for ( ; ; ) {
		std::cout << "am in loop" << std::endl;
        clilen = sizeof(struct sockaddr);

        n = recvfrom(sockfd, buffer, MAXMESG, 0, &pcli_addr, (socklen_t*)&clilen);
		std::cout << "received something" << std::endl;
		if (n < 0) {
			printf("%s: recvfrom error\n",progname);
			exit(4);
		}

        // print out received buffer
		std::cout<< "whole buffer after being received:";
		printf("%x,%x", buffer[0], buffer[1]);
        for (int i = 2; i < MAXMESG; ++i) {
			std::cout<< buffer[i];
		}
		std::cout<<std::endl;


		unsigned short* bufferPointer = nullptr;
		bufferPointer = reinterpret_cast<unsigned short *>(buffer);
		unsigned short opNumber = ntohs(*bufferPointer);
        std::cout << "converted ntohs op: " << opNumber << std::endl;
        

        char *bufpoint = buffer + 2;
		std::cout<< "bufpoint: " << *bufpoint <<std::endl;

		int fileNameLength = 0;
		std::cout<< "checking for file name length"<<std::endl;
		for (int i = 2; i < MAXMESG; i++) {
			if (buffer[i] != NULL) {
				fileNameLength++;
			}
			else {
				break;
			}
		}
		char filename[fileNameLength];
		std::cout<< "fileNameLength:" << fileNameLength<<std::endl;
		bcopy(bufpoint, filename, fileNameLength);
        //strcpy(filename, bufpoint);
		std::cout<< "filename:";
        for (int i = 0; i < fileNameLength; i++) {
            std::cout << filename[i];
        }
        std::cout << std::endl;

		std::cout << "checking if op is RRQ or WRQ" << std::endl;
        if (opNumber == RRQ) {
			std::cout<< "op is RRQ" <<std::endl;
            // if RRQ, call tftp shared sending function
            //tftp::SendMessage(sockfd, (struct sockaddr *) &serv_addr, &pcli_addr, filename);
			char *fileBuffer = nullptr;
			//bzero(fileBuffer, MAXMESG);
			fileBuffer = CreateDataPacket(filename);
		} else
        if (opNumber == WRQ) {
			std::cout<< "op is WRQ"<<std::endl;
            // if WRQ, send ACK0 and call tftp shared receiving function
            // BuildAckMessage()
            // sendto()
            tftp::ReceiveMessage(sockfd, &pcli_addr, (struct sockaddr *) &serv_addr);
        } else {
			std::cout<< "op was neither"<<std::endl;
		}
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