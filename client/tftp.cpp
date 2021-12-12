//
// Created by Gabriel Oliver and Kevin Huang on 11/16/2021.
//
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>          // for retrieving the error number.
#include <string.h>         // for strerror function.
#include <signal.h>         // for the signal handler registration.
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>

// op codes
#define RRQ	1
#define WRQ 2
#define DATA 3
#define ACK 4
#define	ERROR 5

// error codes
#define NO_FILE 1
#define NO_ACCESS 2
#define OVERWRITE 6
#define UNDEFINED 99

// max sizes
#define MAXMESG 516
#define MAXDATA 512

#include "tftp.h"

int timeoutCount;
#define TIMEOUT_TIME 3

// Parameters: int signum
//
// Post: Increments timeout counter
void sig_handler(int signum){
	std::cout<< "inside sig_handler" <<std::endl;
	timeoutCount++;
	std::cout<< "timeoutCounter incremented" <<std::endl;
}

// Parameters: char *progname, int sockfd, struct sockaddr_in receiving_addr, int clilen, char buffer[MAXMESG], std::string fileName
//
// Post: Stores all the data of a file with the provided std::string fileName, and sends them all to the sockaddr_in receiving_addr.
//		 A linear send and wait for ACK process.
void tftp::SendFile(char *progname, int sockfd, struct sockaddr_in receiving_addr, int clilen, char buffer[MAXMESG], std::string fileName) {
	// Initialize related variables
	// Find necessary number of packets to send all the file's data
	int numberOfRequiredPackets = GetNumberOfRequeiredPackets(fileName);
	char packetsList[numberOfRequiredPackets][MAXMESG];
	for (int i = 0; i < numberOfRequiredPackets; i++) {
		bzero(packetsList[i], MAXMESG);
	}

	// open file
	FILE* file = fopen(fileName.c_str(), "r");
	if (file == NULL) { // If opening the file causes an error, send error packet
		// create ERROR packet
		bzero(buffer, sizeof(buffer));
		std::cout<< "creating ERROR packet" <<std::endl;
		unsigned short opValue = ERROR;
		unsigned short* buffPtr = (unsigned short *) buffer;
		*buffPtr = htons(opValue);
		buffPtr++;

		if (errno == ENOENT) { // File does not exist
			tftp::CreateErrorPacketHelper(buffPtr, NO_FILE, "File not found.");
		} else if (errno == EACCES){ // No permission to access file
			tftp::CreateErrorPacketHelper(buffPtr, NO_ACCESS, "Access violation.");
		} else { // Some other undefined error
			tftp::CreateErrorPacketHelper(buffPtr, UNDEFINED, "Undefined error.");
		}

		// Send the ERROR packet
		std::cout<< "sending ERROR packet" <<std::endl;
		tftp::SendPacketHelper(progname, sockfd, buffer, receiving_addr);
		exit(13);
	}

	// create all data packets at the same time
	// iterator counter to track where in the file to continue for the next packet
	int fileStartIterator = 0;
	for (int i = 0; i < numberOfRequiredPackets; i++) {
		bzero(packetsList[i], (MAXMESG));
		char* bufpoint = packetsList[i];

		// Creating data packet
		unsigned short opValue = DATA;
		unsigned short* opCodePtr = (unsigned short *) packetsList[i];
		*opCodePtr = htons(opValue);
		opCodePtr++; // move pointer to block number
		unsigned short blockNumber = i + 1; // temporary for testing
		*opCodePtr = htons(blockNumber);
		bufpoint = packetsList[i] + 4; // move pointer to file name

		// copy file data into the packet data
		int n = fread(bufpoint, MAXDATA, 1, file);
		// iterator value for next packet
		fileStartIterator += n;
		if (n < 0) {
			std::cout<< "Read error:" << n <<std::endl;
		}
	}
	// close file
	fclose(file); // once finish reading whole file, close text file

	std::cout<< "Sending Data Packet(s)"<<std::endl;
	// Send all the data packets to the target machine
	for (int i = 0; i < numberOfRequiredPackets; i++) {
		// Send the data packet
		tftp::SendPacketHelper(progname, sockfd, packetsList[i], receiving_addr);

		// timeout implementation
		timeoutCount = 0;
		signal(SIGALRM,sig_handler); // Register signal handler
		siginterrupt( SIGALRM, 1 );

		while (true) { // Wait until the target machine returns all ACK packets, or until a timeout occurs
			bzero(buffer, sizeof(buffer));

			alarm(TIMEOUT_TIME); // set timer
			int n;
			n = recvfrom(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &receiving_addr, (socklen_t*)&clilen);
			if (n < 0) {
				std::cout<< "recvfrom value is -1"<<std::endl;
				if (errno == EINTR && timeoutCount <= 10) {
					std::cout<< "errno == EINTR"<<std::endl;
					std::cout<< "timeout count: " << timeoutCount <<std::endl;
					std::cout<< "resending last data packet"<<std::endl;
					tftp::SendPacketHelper(progname, sockfd, packetsList[i], receiving_addr);

					continue;
				} else {
					printf("%s: recvfrom error\n",progname);
					exit(4);
				}
			}
			alarm(0); // turn off alarm
			timeoutCount = 1;

			// check if received packet is the ack
			unsigned short ackOpNumb = tftp::GetPacketOPCode(buffer);
			if (ackOpNumb == ACK && tftp::GetBlockNumber(buffer) == (i+1)) {
				std::cout<< "ACK received. Transaction complete for block:"<< tftp::GetBlockNumber(buffer) <<std::endl;
				break;
			} else {
				std::cout<< "No correct ACK received. Received:"<<ackOpNumb<<std::endl;
				std::cout<< "Need ACK for block: "<< tftp::GetBlockNumber(buffer) <<std::endl;
			}
		}
	}
}

// Parameters: char *progname, int sockfd, struct sockaddr_in pcli_addr
//
// Post: A helper function to send an error letting the sockaddr_in pcli_addr know that a file already exists
void tftp::SendFileAlreadyExistsError(char *progname, int sockfd, struct sockaddr_in pcli_addr) {
	// create ERROR packet
	char errBuff[MAXMESG];
	bzero(errBuff, sizeof(errBuff));
	unsigned short opValue = ERROR;
	unsigned short* buffPtr = (unsigned short *) errBuff;
	*buffPtr = htons(opValue);
	buffPtr++;
	tftp::CreateErrorPacketHelper(buffPtr, OVERWRITE, "File already exists.");

	// Send the ERROR packet
	std::cout<< "sending ERROR packet" <<std::endl;
	tftp::SendPacketHelper(progname, sockfd, errBuff, pcli_addr);
	exit(9);
}

// Parameters: char *progname, int sockfd, struct sockaddr_in sending_addr, char ackBuffer[MAXMESG], std::string fileNameString
//
// Post: Receives all the data of a file with the provided std::string fileNameString.
//		 A linear wait to receive and send corresponding ACK process.
void tftp::ReceiveFile(char *progname, int sockfd, struct sockaddr_in sending_addr, char ackBuffer[MAXMESG], std::string fileNameString) {
	// check file exists
	std::ifstream infile(fileNameString);
	if (infile.good()){
		tftp::SendFileAlreadyExistsError(progname, sockfd, sending_addr);
	}

	// Create new .txt file with same name
	// help from: https://stackoverflow.com/questions/478075/creating-files-in-c
	FILE* file = fopen(fileNameString.c_str(), "w");

	// Initialize local variables
	char buffer[MAXMESG];
	// fill buffer with dummy information to enter while loop
	for (int i = 0; i < MAXMESG; i++) {
		buffer[i] = '.';
	}
	unsigned short expectedBlockNum = 1;
	int rcvlen = sizeof(struct sockaddr);

	// time out implementation
	timeoutCount = 1;
	signal(SIGALRM,sig_handler); // Register signal handler
	siginterrupt( SIGALRM, 1 );

	while (!tftp::CheckIfLastDataPacket(buffer)) { // Keep going until received the last DATA packet, or a timeout
		// empty buffer of any previous data
		bzero(buffer, MAXMESG);
		alarm(TIMEOUT_TIME);
		int n = recvfrom(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &sending_addr, (socklen_t*)&rcvlen);
		if (n < 0) {
			std::cout<< "recvfrom value is -1"<<std::endl;
			if (errno == EINTR && timeoutCount <= 10) {
				std::cout<< "errno == EINTR"<<std::endl;
				std::cout<< "timeout count: " << timeoutCount <<std::endl;
				std::cout<< "resending last data packet"<<std::endl;
				tftp::SendPacketHelper(progname, sockfd, ackBuffer, sending_addr);

				continue;
			} else {
				printf("%s: recvfrom error\n",progname);
				exit(4);
			}
		}
		alarm(0); // turn off alarm
		timeoutCount = 0;

		// get its op code to determine packet type
		unsigned short opValue = tftp::GetPacketOPCode(buffer);
		if (opValue == ERROR) { // Print error message and exit
			unsigned short errorCode = GetBlockNumber(buffer);
			char* bufpoint = buffer + 4;
			int errMsgLength = 0;
			for (int i = 4; i < MAXMESG; i++) {
				if (buffer[i] == NULL) {
					break;
				}
				errMsgLength++;
			}
			char errMsg[errMsgLength];
			bcopy(bufpoint, errMsg, errMsgLength + 1);
			std::string result = std::string(errMsg);
			printf("%s: Error Code %d - %s\n",progname, errorCode, errMsg);
			exit(8);
		} else
		if (opValue == DATA) {
			// get packet block number
			unsigned short receivedBlockNum = tftp::GetBlockNumber(buffer);
			if (receivedBlockNum == expectedBlockNum) { // if received expected block number, write data to file
				//write packet to file
				char* bufpoint = buffer + 4;
				int writeSize = 0;
				for (int i = 4; i < MAXMESG; i++) {
					if (buffer[i] == NULL) {
						break;
					}
					writeSize++;
				}
				fwrite(bufpoint, writeSize, 1, file);
				expectedBlockNum++;
			}
			// regardless if expected packet, send ack packet of received block number
			bzero(ackBuffer, MAXMESG);
			tftp::CreateAckPacket(ackBuffer, receivedBlockNum);

			SendPacketHelper(progname, sockfd, ackBuffer, sending_addr);
		}
		else {
			std::cout<< "Unexpected OP code received:"<<opValue<<" ending process"<<std::endl;
			exit(9);
		}
	}
	// close file after leaving loop
	fclose(file);
}

// Parameters: char buffer[MAXMESG]
//
// Post: Return any packet as a std::string
std::string tftp::PacketToString(char buffer[MAXMESG]) {
	// Initialize local variables
	unsigned short opNumber = tftp::GetPacketOPCode(buffer);
	std::string opStr;
	unsigned short blockNumber = 0;
	std::string blockStr;
	std::string fileNameStr;
	std::string modeStr;
	std::string printResult;

	if (opNumber == RRQ) {
		opStr = "1";
		fileNameStr = tftp::GetFileNameStr(buffer);
		modeStr = tftp::GetMode(buffer, fileNameStr);
		printResult = opStr + fileNameStr + " " + modeStr + " ";
	} else if (opNumber == WRQ) {
		opStr = "2";
		fileNameStr = tftp::GetFileNameStr(buffer);
		modeStr = tftp::GetMode(buffer, fileNameStr);
		printResult = opStr + fileNameStr + " " + modeStr + " ";
	} else if (opNumber == DATA) {
		opStr = "3";
		blockNumber = tftp::GetBlockNumber(buffer);
		blockStr = tftp::ConvertUnsignedShortToString(blockNumber);

		char dataArray[MAXDATA];
		bzero(dataArray, sizeof(dataArray));
		tftp::GetDataContent(buffer, dataArray);

		std::string dataStr = std::string(dataArray);
		printResult = opStr + blockStr + dataStr;
	} else if (opNumber == ACK) {
		opStr = "4";
		blockNumber = tftp::GetBlockNumber(buffer);
		blockStr = tftp::ConvertUnsignedShortToString(blockNumber);
		printResult = opStr + blockStr;
	} else if (opNumber == ERROR) {
		opStr = "5";
		unsigned short errorNumber = tftp::GetBlockNumber(buffer);
		std::string errorCodeStr = ConvertUnsignedShortToString(errorNumber);

		char errorMessageArray[MAXDATA];
		bzero(errorMessageArray, sizeof(errorMessageArray));
		tftp::GetDataContent(buffer, errorMessageArray);
		std::string errorMessageStr = std::string(errorMessageArray);

		printResult = opStr + errorCodeStr + errorMessageStr;
	} else {
		std::cout<< "Error: Unsupported OP code received:"<< opNumber <<std::endl;
		exit(6);
	}
	return printResult;
}

// Parameters: char buffer[MAXMESG]
//
// Post: Print entire packet to console
void tftp::PrintPacket(char buffer[MAXMESG]) {
	std::cout<< "PRINTING CONTENT OF PACKET:"<<std::endl;
	std::string printResult = PacketToString(buffer);
	std::cout<< printResult <<std::endl;
	std::cout<< "END OF PACKET PRINT" <<std::endl;
}

// Parameters: char buffer[MAXMESG]
//
// Post: Return packet's OP code as an unsigned short
unsigned short tftp::GetPacketOPCode(char buffer[MAXMESG]) {
	unsigned short* bufferTempPointer = nullptr;
	bufferTempPointer = reinterpret_cast<unsigned short *>(buffer);
	unsigned short opNumb = ntohs(*bufferTempPointer);
	return opNumb;
}

// Parameters: char buffer[MAXMESG]
//
// Post: Return packet's block number as an unsigned short
unsigned short tftp::GetBlockNumber(char buffer[MAXMESG]) {
	unsigned short* bufferBPointer = nullptr;
	bufferBPointer = reinterpret_cast<unsigned short *>(buffer + 2);
	unsigned short bNumber = ntohs(*bufferBPointer);
	return bNumber;
}

// Parameters: unsigned short number
//
// Post: Convert an unsigned short and return it as an std::string
// help from: https://stackoverflow.com/questions/14871388/convert-from-unsigned-short-to-string-c/14871427
std::string tftp::ConvertUnsignedShortToString(unsigned short number) {
	std::string result;
	std::stringstream ss;
	ss << number;
	result = ss.str();
	return result;
}

// Parameters: char buffer[MAXMESG]
//
// Post: Return packet's file name as an std::string
std::string tftp::GetFileNameStr(char buffer[MAXMESG]) {
	char* bufpoint = buffer + 2;
	int fileNameLength = 0;
	for (int i = 2; i < MAXMESG; i++) {
		if (buffer[i] == NULL) {
			break;
		}
		fileNameLength++;
	}
	char filename[fileNameLength];
	bcopy(bufpoint, filename, fileNameLength + 1);
	std::string result = std::string(filename);
	return result;
}

// Parameters: char buffer[MAXMESG], std::string fileName
//
// Post: Return the mode of the RRQ/WRQ packet as an std::string
std::string tftp::GetMode(char buffer[MAXMESG], std::string fileName) {
	std::string result = "";
	int modeCharLength = 0;
	char* bufpoint;
	bufpoint = buffer + 2; // move pointer to file name
	bufpoint += fileName.length() + 1; //move pointer and add null byte
	for (int i = 2 + fileName.length() + 1; i < (MAXMESG - (2 + fileName.length() + 1)); i++) {
		if (buffer[i] == NULL) {
			break;
		}
		modeCharLength++;
	}
	char modeChar[modeCharLength];
	bcopy(bufpoint, modeChar, modeCharLength + 1);
	result = std::string(modeChar);
	return result;
}

// Parameters: char buffer[MAXMESG], char (& dataArray)[MAXDATA]
//
// Post: Store the data from buffer into dataArray
// pass char[] by reference help from: https://stackoverflow.com/questions/12987760/passing-char-array-by-reference
void tftp::GetDataContent(char buffer[MAXMESG], char (& dataArray)[MAXDATA]) {
	for (int i = 4; i < MAXMESG; i++) {
		if (buffer[i] == NULL) {
			break;
		}
		dataArray[i - 4] = buffer[i];
	}
}

// Parameters: std::string filename
//
// Post: Return number of needed packets to send all the data from the file as an int
int tftp::GetNumberOfRequeiredPackets(std::string filename) {
	std::ifstream file( filename, std::ios::binary | std::ios::ate);
	int fileSize = file.tellg();
	std::cout<< "File size:" << fileSize << " Bytes" <<std::endl;

	if (fileSize < MAXDATA) {
		std::cout<< "File requires 1 packet"<<std::endl;
		return 1;
	}
	int numberOfPackets = (fileSize / MAXDATA) + 1;
	std::cout<< "File requires: " <<numberOfPackets << " number of packets"<<std::endl;
	return numberOfPackets;
}

// Parameters: char buffer[MAXMESG]
//
// Post: Return bool if the passed packet is the last packet
bool tftp::CheckIfLastDataPacket(char buffer[MAXMESG]) {
	bool result = false;
	if (tftp::GetPacketOPCode(buffer) != DATA) {
		return result;
	}
	int dataLength = 0;
	for (int i = 4; i < MAXMESG; i++) {
		if (buffer[i] == NULL) {
			break;
		}
		dataLength++;
	}
	if (dataLength < MAXDATA) {
		result = true;
	}
	return result;
}

// Parameters: char buffer[MAXMESG], unsigned short blockNumber
//
// Post: Create an ACK packet and store it in buffer
void tftp::CreateAckPacket(char buffer[MAXMESG], unsigned short blockNumber) {
	bzero(buffer, MAXMESG);

	unsigned short ackOpValue = ACK;
	unsigned short* ackOpCodePtr = (unsigned short *) buffer;
	*ackOpCodePtr = htons(ackOpValue);

	unsigned short ackBlockValue = blockNumber;
	unsigned short* ackBlockPtr = (unsigned short *) buffer + 1;
	*ackBlockPtr = htons(ackBlockValue);
}

// Parameters: char* progname, int sockfd, char buffer[MAXMESG], struct sockaddr_in receiving_addr
//
// Post: Send packet buffer to the target sockaddr_in receiving_addr
void tftp::SendPacketHelper(char* progname, int sockfd, char buffer[MAXMESG], struct sockaddr_in receiving_addr) {
	int n = sendto(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
	if (n < 0) {
		printf("%s: sendto error\n",progname);
		exit(4);
	}
}

// Parameters: unsigned short* buffPtr, unsigned short errorCode, std::string errorMessage
//
// Post: Creates Error packet and stores in buffPtr
void tftp::CreateErrorPacketHelper(unsigned short* buffPtr, unsigned short errorCode, std::string errorMessage) {
	*buffPtr = htons(errorCode);
	buffPtr++;
	std::string myErrorMessage = errorMessage;
	strcpy((char*)buffPtr, myErrorMessage.c_str());
}

// Parameters: char* progname, int sockfd, char buffer[MAXMESG], struct sockaddr_in receiving_addr
//
// Post: Public version of tftp::SendPacketHelper().
//		 Send packet buffer to the target sockaddr_in receiving_addr
void tftp::SendPacket(char* progname, int sockfd, char buffer[MAXMESG], struct sockaddr_in receiving_addr) {
	tftp::SendPacketHelper(progname, sockfd, buffer, receiving_addr);
}