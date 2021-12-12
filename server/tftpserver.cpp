#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include "tftp.cpp"
#include <iostream>
#include <string>
#include <strings.h> // bzero

#define MAX_CLIENT_CONNECTIONS 11

//
char *progname;

// Parameters: char (& ackBuffer)[MAXMESG], int sockfd,
//					   struct sockaddr_in pcli_addr
//
// Post: Send initial ACK packet for WRQ operations
void SendAckZeroPacket(char (& ackBuffer)[MAXMESG], int sockfd,
					   struct sockaddr_in pcli_addr) {
	unsigned short ackOpValue = ACK;
	unsigned short* ackOpCodePtr = (unsigned short *) ackBuffer;
	*ackOpCodePtr = htons(ackOpValue);

	unsigned short ackBlockValue = 0;
	unsigned short* ackBlockPtr = (unsigned short *) ackBuffer + 1;
	*ackBlockPtr = htons(ackBlockValue);

	int n = sendto(sockfd, ackBuffer, MAXMESG, 0,
				   (struct sockaddr *) &pcli_addr, sizeof(pcli_addr));
	if (n < 0) {
		printf("%s: sendto error\n",progname);
		exit(4);
	}
}

// Parameters: char buffer[MAXMESG], int sockfd,
//						struct sockaddr_in pcli_addr, int clilen
//
// Post: Perform RRQ or WRQ operations with client
void OperateWithClient(char buffer[MAXMESG], int sockfd,
						struct sockaddr_in pcli_addr, int clilen) {
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
			exit(9);
		}

		// Send initial ACK packet to let the client know the server is ready to receive DATA packets
		char ackBuffer[MAXMESG];
		bzero(ackBuffer, MAXMESG);
		SendAckZeroPacket(ackBuffer, sockfd, pcli_addr);

		//call ReceiveFile and wait for DATA from client
		tftp::ReceiveFile(progname, sockfd, pcli_addr,
						  ackBuffer, fileNameString);
	} else {
		std::cout<< "Error: Unsupported OP Code Received:"<< opNumber <<std::endl;
		exit(10);
	}
}

// Parameters: int serv_udp_port
//
// Post: Set up socket and port connection to the specified serv_udp_port
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

	if (bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		printf("%s: can't bind local address for main\n",progname);
		exit(3);
	}
	return socketfd;
}

// Parameters: int sockfd
//
// Main Loop must be ended manually unless an error or timeout occurs
//
// Post: Main Loop to stay open to client connections and start
// 		 RRQ and WRQ operations
void ClientConnectionsLoop(int sockfd) {
	int n, clilen;
	char buffer[MAXMESG];
	bzero(buffer, MAXMESG);
	struct sockaddr_in pcli_addr;

	for ( ; ; ) {
		std::cout << "Ready To Accept A Client RRQ or WRQ" << std::endl;

		clilen = sizeof(struct sockaddr_in);

		n = recvfrom(sockfd,buffer, MAXMESG, 0, (struct sockaddr *) &pcli_addr,
					(socklen_t*)&clilen);
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
			std::cout<< "Request Operation Complete"<<std::endl;
			exit(7);
		}
	}
}

// Parameters: int argc, char *argv[]
//
// Post: Checks if command-line arguments are legal
void CheckForValidArguments(int argc, char *argv[]) {
	if (argc != 2) {
		printf("%s: invalid number of arguments\n",progname);
		exit(1);
	}

	if (argv[1] == nullptr || std::stoi(argv[1]) == NULL) {
		printf("%s: invalid server port number\n",progname);
		exit(1);
	}

	progname=argv[0];
}

// Parameters: int argc, char *argv[]
//
// Command-Line Arguments MUST BE PASSED IN THIS ORDER:
// ./tftpserver <port number>
//
// The server will timout out of an operation
// if the client loses connection or closes.
//
// Post: Establishes a server for client connections
// 		 to perform Read or Write Requests.
//		 Server will perform in an endless loop
//		 and can only be stopped manually, unless
//		 encountered an error.
int main(int argc, char *argv[]) {
	std::cout << "Starting tftpserver Program" << std::endl;
	CheckForValidArguments(argc, argv);
	ClientConnectionsLoop(SetUpSocket(std::stoi(argv[1])));
	return 0;
}