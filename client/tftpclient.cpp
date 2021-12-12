#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "tftp.cpp"

#define SERV_HOST_ADDR "10.158.82.41" //REPLACE WITH SERVER IP ADDRESS; lab11: 10.158.82.41

char *progname;

// Parameters: const char *op, int sockfd, struct sockaddr_in serv_addr,
// 				char buffer[MAXMESG], std::string fileNameString
//
// Guaranteed, const char *op carries either an RRQ or WRQ value
//
// Post: Performs either:
// 		 1. A Read Request
//				- Client receives error if the file does not exist in the server
//				= Client sends error if the file already exists in the client
//		 2. A Write Request
//				- Client receives error if file already exists in server
//				- Client sends error if the file does not exist in the client
void PerformRequest(const char *op, int sockfd, struct sockaddr_in serv_addr,
					char buffer[MAXMESG], std::string fileNameString) {
	if (op[1] == 'r') { // if RRQ, call tftp shared receiving function
		tftp::ReceiveFile(progname, sockfd, serv_addr, buffer, fileNameString);
	} else if (op[1] == 'w') {
		// if WRQ, perform preliminary server acknowledgment before
		// writing to server

		// timeout implementation
		timeoutCount = 1;
		signal(SIGALRM,sig_handler); // Register signal handler
		siginterrupt(SIGALRM, 1);

		// initialize buffer and socket length variables
		char ackBuffer[MAXMESG];
		int servlen = sizeof(struct sockaddr);

		while (true) {
			// Wait until client receives the ACK0, an Error,
			// or until the client times out
			bzero(ackBuffer, sizeof(ackBuffer));

			alarm(TIMEOUT_TIME); // set timer
			int n;
			n = recvfrom(sockfd, ackBuffer, MAXMESG, 0,
						 (struct sockaddr *) &serv_addr, (socklen_t*)&servlen);
			if (n < 0) {
				std::cout<< "recvfrom value is -1"<<std::endl;
				if (errno == EINTR && timeoutCount <= 10) {
					std::cout<< "errno == EINTR"<<std::endl;
					std::cout<< "timeout count: " << timeoutCount <<std::endl;
					std::cout<< "resending last data packet"<<std::endl;
					n = sendto(sockfd, buffer, MAXMESG, 0,
							   (struct sockaddr *) &serv_addr, servlen);
					if (n < 0) {
						printf("%s: sendto error\n",progname);
						exit(4);
					}
					continue;
				} else {
					printf("%s: recvfrom error\n",progname);
					exit(4);
				}
			}
			alarm(0); // turn off alarm
			timeoutCount = 1;

			// check if received packet is the ack
			unsigned short ackOpNumb = tftp::GetPacketOPCode(ackBuffer);
			if (ackOpNumb == ACK && tftp::GetBlockNumber(ackBuffer) == 0) { // Got the ACK0
				std::cout<< "ACK received. transaction complete for block:"
						 << tftp::GetBlockNumber(ackBuffer) <<std::endl;
				break;
			} else if (ackOpNumb == ERROR) { // Some error occurred
				unsigned short errorCode = tftp::GetBlockNumber(ackBuffer);
				char* ackpoint = ackBuffer + 4;
				int errMsgLength = 0;
				for (int i = 4; i < MAXMESG; i++) {
					std::cout << ackBuffer[i];
					if (ackBuffer[i] == NULL) {
						break;
					}
					errMsgLength++;
				}
				char errMsg[errMsgLength];
				bcopy(ackpoint, errMsg, errMsgLength + 1);
				std::string result = std::string(errMsg);
				printf("%s: Error Code %d - %s\n",progname, errorCode, errMsg);
				exit(8);
			}
		}
		// call tftp shared sending function
		tftp::SendFile(progname, sockfd, serv_addr, sizeof(struct sockaddr_in),
				       buffer, /*fileBuffer,*/ fileNameString);
	}
}


// Post: Sets up the socket that the client sends and receives packets from.
//		 Returns int value of the socket
int SetUpSocket() {
	int sockfd;
	struct sockaddr_in cli_addr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s: can't open datagram socket\n",progname);
		exit(2);
	}
	bzero((char *) &cli_addr, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port = htons(0);

	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
		printf("%s: can't bind local address\n",progname);
		exit(3);
	}
	return sockfd;
}

// Parameters: int argc, char *argv[]
//
// Post: Checks that the values of the command line arguments are legal
void CheckForValidArguments(int argc, char *argv[]) {
	if (argc != 4) {
		printf("%s: invalid number of arguments\n",progname);
		exit(1);
	}

	if (argv[3] == nullptr || std::stoi(argv[3]) == NULL) {
		printf("%s: invalid server port number\n",progname);
		exit(1);
	}

	progname=argv[0];
}

// Parameters: int argc, char* argv[]
//
// Command-Line Arguments MUST BE PASSED IN THIS ORDER:
// ./tftpclient <request type> <file name> <port number>
//
// Request type must be either "-r" or "-w".
// File name must include the .txt extension.
// If the port number does not match the server port number,
// the client will time out.
//
// Client will timeout if server loses connection or closes.
//
// Post: Either make a:
// 		 1. Read Request for a file in the server
// 		    and download its data to the client.
// 		 2. Write Request to write a file
// 	   	    from the client to the server.
int main(int argc, char *argv[])
{
	CheckForValidArguments(argc, argv);

	// Initializing Arguments ///////////////
	const char *op = argv[1];			   //
	char *filename = argv[2];			   //
	/////////////////////////////////////////

	// Initializing socket and connection variables
	struct sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port = htons(std::stoi(argv[3]));

	// SetUp Socket and get its value
	int sockfd = SetUpSocket();

	// Start building a packet
	char *bufpoint; // for building packet
	char buffer[MAXMESG]; // packet that will be sent
	bzero(buffer, sizeof(buffer));
	if (op[1] == 'r') { // Create an RRQ packet
		std::cout<< "Creating RRQ packet" <<std::endl;
		unsigned short opValue = RRQ;
		unsigned short* opCodePtr = (unsigned short *) buffer;
		*opCodePtr = htons(opValue);
	} else if (op[1] == 'w') { // Create an WRQ packet
		std::cout<< "Creating WRQ packet" <<std::endl;
		unsigned short opValue = WRQ;
		unsigned short* opCodePtr = (unsigned short *) buffer;
		*opCodePtr = htons(opValue);
	} else {
		// Some unexpected argument. Reduntant check in case it was
		// not caught sooner.
		std::cout<< "Invalid argument, should be either r or w" <<std::endl;
		exit(1);
	}

	// Filling rest of RRQ/WRQ packet
	bufpoint = buffer + 2; // move pointer to file name
	strcpy(bufpoint, filename); // add file name to buffer
	bufpoint += strlen(filename) + 1; //move pointer and add null byte
	char modePointer[5] = {'o','c','t','e','t'};
	strcpy(bufpoint, modePointer); // add mode to buffer
	bufpoint += strlen(modePointer) + 1; // move pointer and add null byte

	// Print packet to client console
	tftp::PrintPacket(buffer);

	// Send RRQ/WRQ to server
	tftp::SendPacket(progname, sockfd, buffer, serv_addr);

	// Save file name for future reference
    std::string fileNameString = tftp::GetFileNameStr(buffer);

	// Perform appropriate tasks to complete either RRQ or WRQ
	PerformRequest(op, sockfd, serv_addr, buffer, fileNameString);
	std::cout<< "Request Operation Complete"<<std::endl;

	// Close socket when done
	close(sockfd);
	return 0;
}
