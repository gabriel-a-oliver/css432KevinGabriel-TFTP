#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <typeinfo>
#include "tftp.cpp"


#define SERV_UDP_PORT 51709
#define SERV_HOST_ADDR "10.158.82.41" //REPLACE WITH SERVER IP ADDRESS; lab11: 10.158.82.41

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
	} else {
        std::cout<< "binded socket"<<std::endl;
    }

	char *bufpoint; // for building packet
	char buffer[MAXMESG]; // packet that will be sent
	bzero(buffer, sizeof(buffer));

	std::cout<< "seeing if OP is r or w" <<std::endl;
	if (op[1] == 'r') {
		std::cout<< "OP is r: " << RRQ <<std::endl;
		unsigned short opValue = RRQ;
		unsigned short* opCodePtr = (unsigned short *) buffer;
		*opCodePtr = htons(opValue);
	} else
	if (op[1] == 'w') {
		std::cout<< "OP is w: " << WRQ <<std::endl;
		unsigned short opValue = WRQ;
		unsigned short* opCodePtr = (unsigned short *) buffer;
		*opCodePtr = htons(opValue);
	} else {
		std::cout<< "neither r or w" <<std::endl;
	}

	std::cout<< "creating RRQ/WRQ request packet" <<std::endl;
	bufpoint = buffer + 2; // move pointer to file name
	strcpy(bufpoint, filename); // add file name to buffer
	bufpoint += strlen(filename) + 1; //move pointer and add null byte
	strcpy(bufpoint, "octet"); // add mode to buffer
	bufpoint += strlen("octet") + 1; // move pointer and add null byte

	tftp::PrintPacket(buffer);

	std::cout<< "sending packet" <<std::endl;
	int n = sendto(sockfd, buffer, bufpoint-buffer, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (n < 0) {
		printf("%s: sendto error\n",progname);
		exit(4);
	} else {
		std::cout<< "no issue sending packet" <<std::endl;
	}

    // save file name before clearing buffer
    std::string fileNameString = tftp::GetFileNameStr(buffer);

    // clear out buffer for reuse
	bzero(buffer, sizeof(buffer));

	if (op[1] == 'r') {
		// if RRQ, call tftp shared receiving function
		tftp::ReceiveMessage(sockfd, (struct sockaddr *) &serv_addr, (struct sockaddr *) &cli_addr, buffer);
		tftp::PrintPacket(buffer);

		char* fileContentBuffer = buffer + 4;

		tftp::WriteToFile(filename, fileContentBuffer);

		// create ACK packet and send to server 
		unsigned short opNumb = tftp::GetPacketOPCode(buffer);
		if (opNumb == DATA) {
			std::cout<< "Received and written Data packet, creating corresponding ack packet"<<std::endl;

			std::cout<< "creating ack packet" <<std::endl;
			bzero(buffer, sizeof(buffer));
			char* ackBufPoint = buffer + 2;

			unsigned short ackOpValue = ACK;
			unsigned short* ackOpCodePtr = (unsigned short *) buffer;
			*ackOpCodePtr = htons(ackOpValue);

			unsigned short ackBlockValue = 1; // temporary value for testing
			unsigned short* ackBlockPtr = (unsigned short *) buffer + 1;
			*ackBlockPtr = htons(ackBlockValue);

			tftp::PrintPacket(buffer);

			std::cout<< "sending packet" <<std::endl;
			int n = sendto(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
			if (n < 0) {
				printf("%s: sendto error\n",progname);
				exit(4);
			} else {
				std::cout<< "no issue sending packet" <<std::endl;
			}
		}
	} else if (op[1] == 'w') {
        std::cout<< "waiting for ACK0 from server" <<std::endl;
        tftp::ReceiveMessage(sockfd, (struct sockaddr *) &serv_addr, (struct sockaddr *) &cli_addr, buffer);

		// check if received packet is the ack
		tftp::PrintPacket(buffer);
		unsigned short ackOpNumb = tftp::GetPacketOPCode(buffer);
		if (ackOpNumb == ACK) {
			std::cout<< "ack received. transaction complete for block:"<< tftp::GetBlockNumber(buffer) <<std::endl;
		} else {
			std::cout<< "no ack received. received:"<<ackOpNumb<<std::endl;
		}

        std::cout<< "creating data packet" <<std::endl;
		char fileBuffer[MAXMESG];
		bzero(fileBuffer, MAXMESG);
        
        tftp::CreateDataPacket(fileNameString,fileBuffer);

		tftp::PrintPacket(fileBuffer);

		// Send the data packet to client 
		std::cout<< "sending data packet" <<std::endl;
		int n = sendto(sockfd, fileBuffer, MAXMESG/*sizeof(fileBuffer)*/, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
		if (n < 0) {
			printf("%s: sendto error\n",progname);
			exit(4);
		} else {
			std::cout<< "no issue sending packet" <<std::endl;
		}

        // waiting for ACK from server
        std::cout<< "waiting for ACK from server" <<std::endl;
        bzero(buffer, sizeof(buffer));
        tftp::ReceiveMessage(sockfd, (struct sockaddr *) &serv_addr, (struct sockaddr *) &cli_addr, buffer);

		// check if received packet is the ack
		tftp::PrintPacket(buffer);
		ackOpNumb = tftp::GetPacketOPCode(buffer);
		if (ackOpNumb == ACK) {
			std::cout<< "ack received. transaction complete for block:"<< tftp::GetBlockNumber(buffer) <<std::endl;
		} else {
			std::cout<< "no ack received. received:"<<ackOpNumb<<std::endl;
		}

	} else {
		std::cout<< "was not RRQ or WRQ" <<std::endl;
	}

	close(sockfd);

	return 0;
}























