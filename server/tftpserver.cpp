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


void CreateDataPacket(char buffer[MAXMESG], char fileBuffer[MAXMESG]) {
	std::cout<< "in CreateDataPacket()"<<std::endl;
	//char buffer[MAXMESG];
	bzero(fileBuffer, (MAXMESG));
	char* bufpoint = fileBuffer;

	std::cout<< "creating data packet" <<std::endl;

	unsigned short opValue = DATA;
	unsigned short* opCodePtr = (unsigned short *) fileBuffer;
	*opCodePtr = htons(opValue);
	std::cout<< "OP:";
	std::cout << opValue <<std::endl;


	opCodePtr++; // move pointer to block number
	std::cout<< "Block#:";
	unsigned short blockNumber = 1; // temporary for testing
	*opCodePtr = htons(blockNumber);
	std::cout << blockNumber <<std::endl;

	bufpoint = fileBuffer + 4; // move pointer to file name
	//FileData//////////////////////////////////////////////////////////////////
	//open and reading Linux commands:
	std::cout << "Get File Name:" << tftp::GetFileNameStr(buffer) <<std::endl;
	int fd = open(const_cast<char*>(tftp::GetFileNameStr(buffer).c_str()), O_RDONLY); // open text file
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
		if (n < 0) {
			printf("%s: recvfrom error\n",progname);
			exit(4);
		}
		std::cout << "received something" << std::endl;
		tftp::PrintPacket(buffer);

		unsigned short opNumber = tftp::GetPacketOPCode(buffer);

		std::cout << "checking if op is RRQ or WRQ" << std::endl;
        if (opNumber == RRQ) {
			std::cout<< "op is RRQ" <<std::endl;
			char fileBuffer[MAXMESG];
			bzero(fileBuffer, MAXMESG);
			CreateDataPacket(buffer,fileBuffer);
			tftp::PrintPacket(fileBuffer);


			// Send the data packet to client /////////////////////////////////////////////////
			std::cout<< "sending data packet" <<std::endl;
			int n = sendto(sockfd, fileBuffer, MAXMESG/*sizeof(fileBuffer)*/, 0, (struct sockaddr *) &pcli_addr, sizeof(pcli_addr));
			if (n < 0) {
				printf("%s: sendto error\n",progname);
				exit(4);
			} else {
				std::cout<< "no issue sending packet" <<std::endl;
			}
			// ////////////////////////////////////////////////////////////////////////////////

			// Wait to receive ACK from client /////////////////////////////////////////////////////////////////////////
			std::cout<< "Waiting to receive ack from client"<<std::endl;
			char ackBuffer[MAXMESG];
			bzero(ackBuffer, MAXMESG);
			n = recvfrom(sockfd, ackBuffer, MAXMESG, 0, &pcli_addr, (socklen_t*)&clilen);
			if (n < 0) {
				printf("%s: recvfrom error\n",progname);
				exit(4);
			}
			std::cout << "received something" << std::endl;

			// check if received packet is the ack
			tftp::PrintPacket(ackBuffer);
			unsigned short ackOpNumb = tftp::GetPacketOPCode(ackBuffer);
			if (ackOpNumb == ACK) {
				std::cout<< "ack received. transaction complete for block:"<< tftp::GetBlockNumber(ackBuffer) <<std::endl;
			} else {
				std::cout<< "no ack received. received:"<<ackOpNumb<<std::endl;
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////////

		} else
        if (opNumber == WRQ) {
			/*std::cout<< "op is WRQ"<<std::endl;
            // if WRQ, send ACK0 and call tftp shared receiving function
            // BuildAckMessage()
            // sendto()
            tftp::ReceiveMessage(sockfd, &pcli_addr, (struct sockaddr *) &serv_addr, filename);*/
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