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

//#define SERV_UDP_PORT 51709
#define MAX_CLIENT_CONNECTIONS 11

char *progname;
int serv_udp_port;
//int mySocket;



// Small struct to pass multiple variables in the threaded
// function "GettingClientInput(void* threadArguments)"
/*struct ThreadArguments {
	int myThreadSocket;
	struct sockaddr_in* myThreadClientAddress;
	int myThreadClientLength;
	char* myThreadBuffer;
};*/
void* OperateWithClient(char buffer[MAXMESG], int sockfd, struct sockaddr_in pcli_addr, int clilen) {
	std::cout<< "In OperateWithClient()"<<std::endl;
	//ThreadArguments* passedThreadArgs;
	//passedThreadArgs = (ThreadArguments*)threadArguments;

	/*char* buffer;
	//bzero(buffer, MAXMESG);
	//buffer = passedThreadArgs->myThreadBuffer;
	std::cout<< "buffer from thread arguments set:"<<std::endl;
	tftp::PrintPacket(buffer);
	//int sockfd = passedThreadArgs->myThreadSocket;
	int sockfd = mySocket;
	//std::cout<< "socket value:"<<sockfd<<std::endl<<"socket value from thread args:"<<passedThreadArgs->myThreadSocket<<std::endl;

	sockaddr_in* pcli_addr_pointer = nullptr;
	std::cout<< "assigning local pcli_addr_pointer with threaded arguments"<<std::endl;*/
	//pcli_addr_pointer = passedThreadArgs->myThreadClientAddress;
	/*if (pcli_addr_pointer == nullptr) {
		std::cout<< "it was passed a nullPtr"<<std::endl;
	} else {
		std::cout<< "it was not passed a nullPtr"<<std::endl;
	}*/
	//struct sockaddr_in pcli_addr = *pcli_addr_pointer;//passedThreadArgs->myThreadClientAddress;
	//int clilen = passedThreadArgs->myThreadClientLength;
	//std::cout<< "client length:"<<clilen<<std::endl<<"client length from thread args:"<<passedThreadArgs->myThreadClientLength<<std::endl;
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
		tftp::ReceiveFile(progname, sockfd, pcli_addr, ackBuffer, fileNameString);
	} else {
		std::cout<< "op was neither"<<std::endl;
	}
	return nullptr;
}




/*void ThreadClient(char buffer[MAXMESG], int sockfd, struct sockaddr_in pcli_addr, int clilen) {
	std::cout<< "In ThreadClient()"<<std::endl;
	std::cout<< "socket value:" <<sockfd<<std::endl;
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
	std::cout<< "assigning thread arguments"<<std::endl;
	mySocket = sockfd;
	clientThreadArguments->myThreadSocket = mySocket;
	std::cout<< "assigned thread socket:" << clientThreadArguments->myThreadSocket<<std::endl;
	sockaddr_in* pcli_addr_pointer;
	*pcli_addr_pointer = pcli_addr;
	clientThreadArguments->myThreadClientAddress = pcli_addr_pointer;
	std::cout<< "assigned thread client address"<<std::endl;
	clientThreadArguments->myThreadClientLength = clilen;
	std::cout<< "assigned thread client address length:" << clientThreadArguments->myThreadClientLength<<std::endl;

	clientThreadArguments->myThreadBuffer = buffer;
	std::cout<< "packet assigned to thread arguments"<<std::endl;
	tftp::PrintPacket(clientThreadArguments->myThreadBuffer);


	std::cout<< "starting to thread"<<std::endl;
	// Created new thread
	pthread_create(&clientThread, &clientThreadAttr, OperateWithClient,
				   (void*)clientThreadArguments);

	// Close Thread and Free threaded arguments memory
	//int result;
	//result = pthread_join(clientThread, NULL);
	//delete clientThreadArguments->myThreadClientAddress;
	//delete[] clientThreadArguments->myThreadBuffer;
	delete clientThreadArguments;
	clientThreadArguments = nullptr;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



}*/


void ClientConnectionsLoop(int sockfd) {

	int n, clilen;
	char buffer[MAXMESG];
	bzero(buffer, MAXMESG);
	struct sockaddr_in pcli_addr;

	for ( ; ; ) {
		std::cout << "am in loop" << std::endl;

		clilen = sizeof(struct sockaddr_in);

		n = recvfrom(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &pcli_addr, (socklen_t*)&clilen);
		if (n < 0) {
			printf("%s: recvfrom error\n",progname);
			exit(4);
		}

		std::cout << "received something" << std::endl;
		tftp::PrintPacket(buffer);
		unsigned short myOpCode = tftp::GetPacketOPCode(buffer);
		std::cout<< "Received OP code:"<<myOpCode<<std::endl;
		if (myOpCode == RRQ || myOpCode == WRQ) {
			std::cout<< "its a RRQ or a WRQ, forking"<<std::endl;
			//ThreadClient(buffer, sockfd, pcli_addr, clilen);
			int pid = fork();
			if (pid < 0) {
				std::cout<< "error in making fork"<<std::endl;
				exit(99);
			}
			if (pid == 0) {

				OperateWithClient(buffer, sockfd, pcli_addr, clilen);
				//close(sockfd);
				exit(7);
			}
		} //else {
		//std::cout<< "not an RRQ or a WRQ, dont fork"<<std::endl;
		//}
		//close(sockfd);
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
	std::cout<< "socket Value:" << sockfd <<std::endl;

	ClientConnectionsLoop(sockfd);

	return 0;
}
