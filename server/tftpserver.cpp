#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include "tftp.cpp"
#include <iostream>
#include <string>
#include <strings.h> // bzero

#define MAX_CLIENT_CONNECTIONS 11

char *progname;

void SendAckZeroPacket(char (& ackBuffer)[MAXMESG], int sockfd, struct sockaddr_in pcli_addr) {
	unsigned short ackOpValue = ACK;
	unsigned short* ackOpCodePtr = (unsigned short *) ackBuffer;
	*ackOpCodePtr = htons(ackOpValue);

	unsigned short ackBlockValue = 0;
	unsigned short* ackBlockPtr = (unsigned short *) ackBuffer + 1;
	*ackBlockPtr = htons(ackBlockValue);

	tftp::PrintPacket(ackBuffer);

	int n = sendto(sockfd, ackBuffer, MAXMESG, 0, (struct sockaddr *) &pcli_addr, sizeof(pcli_addr));
	if (n < 0) {
		printf("%s: sendto error\n",progname);
		exit(4);
	}
}

void* OperateWithClient(char buffer[MAXMESG], int sockfd, struct sockaddr_in pcli_addr, int clilen) {
	unsigned short opNumber = tftp::GetPacketOPCode(buffer);
	if (opNumber == RRQ) {
		std::string fileName = tftp::GetFileNameStr(buffer);

		//call tftp SendFile to send DATA
		tftp::SendFile(progname, sockfd, pcli_addr, clilen, buffer, /*fileBuffer,*/ fileName);
	} else if (opNumber == WRQ) {
		// save file name before clearing buffer
		std::string fileNameString = tftp::GetFileNameStr(buffer);

		// clear out buffer for reuse
		bzero(buffer, sizeof(buffer));

		// Send file already exists error if file already exists in server
		std::ifstream infile(fileNameString);
		if (infile.good()) {
			tftp::SendFileAlreadyExistsError(progname, sockfd, pcli_addr);
		}

		// Send initial ACK packet to let the client know the server is ready to receive DATA packets
		char ackBuffer[MAXMESG];
		bzero(ackBuffer, MAXMESG);
		SendAckZeroPacket(ackBuffer, sockfd, pcli_addr);

		//call ReceiveFile and wait for DATA from client
		tftp::ReceiveFile(progname, sockfd, pcli_addr, ackBuffer, fileNameString);
	} else {
		std::cout<< "Error: Unsupported OP Code Received:"<< opNumber <<std::endl;
		exit(10);
	}
	return nullptr;
}

int SetUpSocket(int serv_udp_port) {
	struct sockaddr_in serv_addr;
	int socketfd;

	if ( (socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s: can't open datagram socket\n",progname);
		exit(2);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(serv_udp_port);

	if (bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("%s: can't bind local address for main\n",progname);
		exit(3);
	}
	return socketfd;
}

void ClientConnectionsLoop(int sockfd) {
	int n, clilen;
	char buffer[MAXMESG];
	bzero(buffer, MAXMESG);
	struct sockaddr_in pcli_addr;

	for ( ; ; ) {
		std::cout << "Ready To Accept A Client RRQ or WRQ" << std::endl;

		clilen = sizeof(struct sockaddr_in);

		n = recvfrom(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &pcli_addr, (socklen_t*)&clilen);
		if (n < 0) {
			printf("%s: recvfrom error\n",progname);
			exit(4);
		}

		std::cout << "Received Packet:" << tftp::PacketToString(buffer) << std::endl;

		int pid = fork();
		if (pid < 0) {
			std::cout<< "error in making fork"<<std::endl;
			exit(8);
		}
		if (pid == 0) {
			int forkedSockfd = SetUpSocket(0);
			OperateWithClient(buffer, forkedSockfd, pcli_addr, clilen);
			exit(7);
		}
	}
}

void CheckForValidArguments(int argc, char *argv[]) {
	if (argc != 2) {
		printf("%s: invalid number of arguments\n",progname);
		exit(1);
	}

	progname=argv[0];
	if (argv[1] == nullptr || std::stoi(argv[1]) == NULL) {
		printf("%s: invalid server port number\n",progname);
		exit(1);
	}
}

int main(int argc, char *argv[]) {
	std::cout << "Starting tftpserver Program" << std::endl;
	CheckForValidArguments(argc, argv);
	ClientConnectionsLoop(SetUpSocket(std::stoi(argv[1])));
	return 0;
}