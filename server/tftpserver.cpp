#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include "tftp.cpp"
#include <iostream>
#include <string>
#include <strings.h> // bzero
#include <pthread.h>
#include <valarray>
#include <sys/wait.h>

//#define SERV_UDP_PORT 51709
#define MAX_CLIENT_CONNECTIONS 11

char *progname;
int serv_udp_port;

<<<<<<< Updated upstream
=======
struct sockaddr_in myClientAddress;
int mySocket;

bool permissionToReceive = true;
>>>>>>> Stashed changes



// Small struct to pass multiple variables in the threaded
// function "GettingClientInput(void* threadArguments)"
struct ThreadArguments {
<<<<<<< Updated upstream
	int myThreadSocket;
=======
>>>>>>> Stashed changes
	struct sockaddr_in* myThreadClientAddress;
	int dummyInt;
	int myThreadClientLength;
	char* myThreadBuffer;
<<<<<<< Updated upstream
};
void* OperateWithClient(void* threadArguments) {
	std::cout<< "In OperateWithClient()"<<std::endl;
	ThreadArguments* passedThreadArgs;
	passedThreadArgs = (ThreadArguments*)threadArguments;

	char* buffer;
	//bzero(buffer, MAXMESG);
	buffer = passedThreadArgs->myThreadBuffer;
	std::cout<< "buffer from thread arguments set:"<<std::endl;
	tftp::PrintPacket(buffer);
	int sockfd = passedThreadArgs->myThreadSocket;
	std::cout<< "socket from thread arguments:" << sockfd <<std::endl;

	sockaddr_in* pcli_addr_pointer = nullptr;
	std::cout<< "assigning local pcli_addr_pointer with threaded arguments"<<std::endl;
	pcli_addr_pointer = passedThreadArgs->myThreadClientAddress;
	if (pcli_addr_pointer == nullptr) {
		std::cout<< "it was passed a nullPtr"<<std::endl;
	} else {
		std::cout<< "it was not passed a nullPtr"<<std::endl;
	}
	struct sockaddr_in pcli_addr = *pcli_addr_pointer;//passedThreadArgs->myThreadClientAddress;
	int clilen = passedThreadArgs->myThreadClientLength;

=======
	int myThreadSocket;
};
void* OperateWithClient(void* threadArguments) {
	std::cout<< "In OperateWithClient()"<<std::endl;
	char* buffer;
	buffer = (char*)threadArguments;

	/*ThreadArguments* passedThreadArgs;
	passedThreadArgs = (ThreadArguments*)threadArguments;

	int myDummyInt = passedThreadArgs->dummyInt;
	std::cout<< "dummy int from thread arguments:"<<myDummyInt<<std::endl;



	int sockfd = passedThreadArgs->myThreadSocket;
	std::cout<< "socket from thread arguments:"<<sockfd<<std::endl;

	*//*sockaddr_in* pcli_addr_pointer = nullptr;
	pcli_addr_pointer = passedThreadArgs->myThreadClientAddress;
	std::cout<< "client address from thread arguments";
	if (pcli_addr_pointer == nullptr) {
		std::cout<< " is null"<<std::endl;
	} else {
		std::cout<< " is not null"<<std::endl;
	}*//*
	struct sockaddr_in pcli_addr = *passedThreadArgs->myThreadClientAddress;//*pcli_addr_pointer;

	//int clilen = passedThreadArgs->myThreadClientLength;
	int clilen = sizeof(pcli_addr);
	std::cout<< "client addres length from threaded arguments:"<<clilen<<std::endl;*/

	//buffer = passedThreadArgs->myThreadBuffer;
	std::cout<< "buffer from thread arguments:"<<std::endl;
	tftp::PrintPacket(buffer);


	int sockfd = mySocket;
	std::cout<< "socket from thread arguments:"<<sockfd<<std::endl;

	struct sockaddr_in pcli_addr = myClientAddress;

	int clilen = sizeof(pcli_addr);
	std::cout<< "client address length from threaded arguments:"<<clilen<<std::endl;


>>>>>>> Stashed changes
	unsigned short opNumber = tftp::GetPacketOPCode(buffer);
	std::cout << "checking if op is RRQ or WRQ" << std::endl;
	if (opNumber == RRQ) {
		std::cout<< "op is RRQ" <<std::endl;
		/*char fileBuffer[MAXMESG];
		bzero(fileBuffer, MAXMESG);*/

		std::string fileName = tftp::GetFileNameStr(buffer);

		//call tftp SendFile to send DATA
		tftp::SendFile(progname, sockfd, pcli_addr, clilen, buffer, /*fileBuffer,*/ fileName);

	} else if (opNumber == WRQ) {
		std::cout<< "op is WRQ"<<std::endl;

		// save file name before clearing buffer
		std::string fileNameString = tftp::GetFileNameStr(buffer);

		// clear out buffer for reuse
		bzero(buffer, sizeof(buffer));

		std::ifstream infile(fileNameString);
		if (infile.good()){
			std::cout<< "file exists, delete data before writing to it"<<std::endl;

			// create ERROR packet
			char errBuff[MAXMESG];
			bzero(errBuff, sizeof(errBuff));
			std::cout<< "creating ERROR packet" <<std::endl;
			unsigned short opValue = ERROR;
			unsigned short* buffPtr = (unsigned short *) errBuff;
			*buffPtr = htons(opValue);
			std::cout<< "OP is: " << opValue <<std::endl;

			buffPtr++;
			unsigned short errorCode = OVERWRITE;
			*buffPtr = htons(errorCode);
			std::cout<< "Error Code is : " << errorCode <<std::endl;

			buffPtr++;
			std::string errormessage = "File already exists.";
			strcpy((char*)buffPtr, errormessage.c_str());

			// Send the ERROR packet
			std::cout<< "sending ERROR packet" <<std::endl;
			int n = sendto(sockfd, errBuff, MAXMESG/*sizeof(fileBuffer)*/, 0, (struct sockaddr *) &pcli_addr, sizeof(pcli_addr));
			if (n < 0) {
				printf("%s: sendto error\n",progname);
				exit(4);
			} else {
				std::cout<< "no issue sending packet" <<std::endl;
			}

			exit(99); // temporary, should just be sending back error instead of exiting
		}

		std::cout<< "creating ack0 packet" <<std::endl;
		char ackBuffer[MAXMESG];
		bzero(ackBuffer, sizeof(ackBuffer));
		char* ackBufPoint = ackBuffer + 2;

		unsigned short ackOpValue = ACK;
		unsigned short* ackOpCodePtr = (unsigned short *) ackBuffer;
		*ackOpCodePtr = htons(ackOpValue);

		unsigned short ackBlockValue = 0;
		unsigned short* ackBlockPtr = (unsigned short *) ackBuffer + 1;
		*ackBlockPtr = htons(ackBlockValue);

		tftp::PrintPacket(ackBuffer);

		std::cout<< "sending ack0 packet" <<std::endl;
		int n = sendto(sockfd, ackBuffer, MAXMESG, 0, (struct sockaddr *) &pcli_addr, sizeof(pcli_addr));
		if (n < 0) {
			printf("%s: sendto error\n",progname);
			exit(4);
		} else {
			std::cout<< "no issue sending packet" <<std::endl;
		}

		//call ReceiveFile and wait for DATA from client
		tftp::ReceiveFile(progname, sockfd, pcli_addr, fileNameString);
	} else {
		std::cout<< "op was neither"<<std::endl;
	}
	permissionToReceive = true;
	return nullptr;
}




void ThreadClient(char buffer[MAXMESG], int sockfd, struct sockaddr_in pcli_addr, int clilen) {
	std::cout<< "In ThreadClient()"<<std::endl;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Initializing threading variables
	std::cout<< "Initializing threading"<<std::endl;
	pthread_t clientThread;
	pthread_attr_t clientThreadAttr;
	pthread_attr_init(&clientThreadAttr);
	ThreadArguments* clientThreadArguments;
	std::cout<< "allocating memory for thread arguments"<<std::endl;
	clientThreadArguments = (ThreadArguments*)malloc(sizeof(ThreadArguments));


	// Assigning variables to pass as arguments
<<<<<<< Updated upstream
	std::cout<< "assigning thread arguments"<<std::endl;
=======
	/*std::cout<< "assigning thread arguments"<<std::endl;

	clientThreadArguments->dummyInt = 999;
	std::cout<< "dummyInt:"<<clientThreadArguments->dummyInt<<std::endl;

>>>>>>> Stashed changes
	clientThreadArguments->myThreadSocket = sockfd;
	std::cout<< "assigned thread socket:" << clientThreadArguments->myThreadSocket<<std::endl;

	clientThreadArguments->myThreadClientAddress = (sockaddr_in*)malloc(sizeof(sockaddr_in));
	sockaddr_in* pcli_addr_pointer;
	*pcli_addr_pointer = pcli_addr;
	clientThreadArguments->myThreadClientAddress = pcli_addr_pointer;
	std::cout<< "assigned thread client address"<<std::endl;

	clientThreadArguments->myThreadClientLength = clilen;
	std::cout<< "assigned thread client address length:" << clientThreadArguments->myThreadClientLength<<std::endl;

	clientThreadArguments->myThreadBuffer = buffer;
	std::cout<< "packet assigned to thread arguments"<<std::endl;
	tftp::PrintPacket(clientThreadArguments->myThreadBuffer);*/


	mySocket = sockfd;
	myClientAddress = pcli_addr;



	std::cout<< "starting to thread"<<std::endl;
	// Created new thread
	pthread_create(&clientThread, &clientThreadAttr, OperateWithClient,
				   (void*)buffer);

	// Close Thread and Free threaded arguments memory
	int result;
	result = pthread_join(clientThread, NULL);
	//delete clientThreadArguments->myThreadClientAddress;
	//delete[] clientThreadArguments->myThreadBuffer;
	delete clientThreadArguments;
	clientThreadArguments = nullptr;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



}
<<<<<<< Updated upstream
=======

>>>>>>> Stashed changes


void ClientConnectionsLoop(int sockfd) {

	int n, clilen;
	char buffer[MAXMESG];
	bzero(buffer, MAXMESG);
	struct sockaddr_in pcli_addr;
	clilen = sizeof(struct sockaddr_in);

	for ( ; ; ) {
<<<<<<< Updated upstream
		std::cout << "am in loop" << std::endl;
		clilen = sizeof(struct sockaddr_in);

		n = recvfrom(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &pcli_addr, (socklen_t*)&clilen);
		if (n < 0) {
			printf("%s: recvfrom error\n",progname);
			exit(4);
		}

		std::cout << "received something" << std::endl;

		tftp::PrintPacket(buffer);
		ThreadClient(buffer, sockfd, pcli_addr, clilen);

=======
		std::cout << "Start of main server loop" << std::endl;
		if (permissionToReceive) {
			n = recvfrom(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &pcli_addr, (socklen_t *) &clilen);
			if (n < 0) {
				printf("%s: recvfrom error in main loop\n", progname);
				exit(4);
			}
			permissionToReceive = false;
			std::cout << "Received something in main server loop. Printing Packet:" << std::endl;
			tftp::PrintPacket(buffer);

			unsigned short opReceived = tftp::GetPacketOPCode(buffer);
			if (opReceived == RRQ || opReceived == WRQ) {
				std::cout << "clilen value before ThreadClient():" << clilen << std::endl;
				ThreadClient(buffer, sockfd, pcli_addr, clilen);
				pcli_addr = {0, 0, 0};
			} else {
				std::cout << "Received a RRQ or WRQ in main loop" << std::endl;
			}
			pcli_addr = {0,0,0};
		}
>>>>>>> Stashed changes
	}

}




int SetUpServer() {
	struct sockaddr_in serv_addr;
	int result;

	if ( (result = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s: can't open datagram socket\n",progname);
		exit(2);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(serv_udp_port);

	if (bind(result, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("%s: can't bind local address\n",progname);
		exit(3);
	}
	return result;
}


int main(int argc, char *argv[]) {
	std::cout << "in main" << std::endl;
	if (argc != 2) {
		printf("%s: invalid number of arguments\n",progname);
		exit(1);
	}

	progname=argv[0];
	if (argv[1] == nullptr || std::stoi(argv[1]) == NULL) {
		printf("%s: invalid server port number\n",progname);
		exit(1);
	}
	serv_udp_port = std::stoi(argv[1]);
	int sockfd = SetUpServer();



	ClientConnectionsLoop(sockfd);




	return 0;
}
