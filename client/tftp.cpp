//
// Created by Gabriel Oliver and Kevin Huang on 11/16/2021.
//
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
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <cassert>
#include <sstream>
#include <stdio.h>

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


// timeout implementation
void sig_handler(int signum){
	std::cout<< "inside sig_handler" <<std::endl;
	timeoutCount++;
	std::cout<< "timeoutCounter incremented" <<std::endl;
}


void tftp::SendFile(char *progname, int sockfd, struct sockaddr_in receiving_addr, int clilen, char buffer[MAXMESG], /*char fileBuffer[MAXMESG],*/ std::string fileName) {
	std::cout<< "In tftp::SendFile()"<<std::endl;
	int numberOfRequiredPackets = GetNumberOfRequeiredPackets(fileName);
	char packetsList[numberOfRequiredPackets][MAXMESG];
	for (int i = 0; i < numberOfRequiredPackets; i++) {
		bzero(packetsList[i], MAXMESG);
	}

	std::cout << "File Name:" << fileName <<std::endl;

	// open file
	FILE* file = fopen(fileName.c_str(), "r");

	if (file == NULL) {
		// create ERROR packet
		bzero(buffer, sizeof(buffer));
		std::cout<< "creating ERROR packet" <<std::endl;
		unsigned short opValue = ERROR;
		unsigned short* buffPtr = (unsigned short *) buffer;
		*buffPtr = htons(opValue);
		std::cout<< "OP is: " << opValue <<std::endl;
		buffPtr++;

		if (errno == ENOENT) {
			tftp::CreateErrorPacketHelper(buffPtr, NO_FILE, "File not found.");
		} else if (errno == EACCES){
			tftp::CreateErrorPacketHelper(buffPtr, NO_ACCESS, "Access violation.");
		} else {
			// errno is UNDEFINED
			tftp::CreateErrorPacketHelper(buffPtr, UNDEFINED, "Undefined error.");
		}

		// Send the ERROR packet
		std::cout<< "sending ERROR packet" <<std::endl;
		tftp::SendPacketHelper(progname, sockfd, buffer, receiving_addr);

		exit(13);
	}

	std::cout<< "file exists" <<std::endl;

	// create data packets
	int fileStartIterator = 0;
	std::cout<< "fileStartIterator:" <<fileStartIterator<<std::endl;
	for (int i = 0; i < numberOfRequiredPackets; i++) {
		bzero(packetsList[i], (MAXMESG));
		char* bufpoint = packetsList[i];

		std::cout<< "creating data packet" <<std::endl;

		unsigned short opValue = DATA;
		unsigned short* opCodePtr = (unsigned short *) packetsList[i];
		*opCodePtr = htons(opValue);
		std::cout<< "OP:";
		std::cout << opValue <<std::endl;

		opCodePtr++; // move pointer to block number
		std::cout<< "Block#:";
		unsigned short blockNumber = i + 1; // temporary for testing
		*opCodePtr = htons(blockNumber);
		std::cout << blockNumber <<std::endl;

		bufpoint = packetsList[i] + 4; // move pointer to file name

		int n = fread(bufpoint, MAXDATA, 1, file);
		fileStartIterator += n;
		if (n < 0) {
			std::cout<< "read error:" << n <<std::endl;
		} else {
			std::cout << "read successful:" << n << std::endl;
		}

		std::cout<< "fileStartIterator:" <<fileStartIterator<<std::endl;
		PrintPacket(packetsList[i]);
	}
	// close file
	fclose(file); // once finish reading whole file, close text file
	std::cout<< "closed file"<<std::endl;


	for (int i = 0; i < numberOfRequiredPackets; i++) {
		// Send the data packet
		std::cout<< "sending data packet" <<std::endl;
		tftp::SendPacketHelper(progname, sockfd, packetsList[i], receiving_addr);

		// timeout implementation
		timeoutCount = 0;
		signal(SIGALRM,sig_handler); // Register signal handler
		siginterrupt( SIGALRM, 1 );

		while (true) {
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
			std::cout << "received something" << std::endl;
			alarm(0); // turn off alarm
			timeoutCount = 1;

			// check if received packet is the ack
			PrintPacket(buffer);
			unsigned short ackOpNumb = tftp::GetPacketOPCode(buffer);
			if (ackOpNumb == ACK && tftp::GetBlockNumber(buffer) == (i+1)) {
				std::cout<< "ACH received. Transaction complete for block:"<< tftp::GetBlockNumber(buffer) <<std::endl;
				break;
			} else {
				std::cout<< "No correct ACK received. Received:"<<ackOpNumb<<std::endl;
				std::cout<< "Need ACK for block: "<< tftp::GetBlockNumber(buffer) <<std::endl;
			}
		}
	}
}

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

void tftp::ReceiveFile(char *progname, int sockfd, struct sockaddr_in sending_addr, char ackBuffer[MAXMESG], std::string fileNameString) {
	std::cout<< "In tftp::ReceiveFile()"<<std::endl;
	// check file exists
	std::ifstream infile(fileNameString);
	if (infile.good()){
		tftp::SendFileAlreadyExistsError(progname, sockfd, sending_addr);
	}

	std::cout<< "file does not exist, creating file of same name"<<std::endl;
	// help from: https://stackoverflow.com/questions/478075/creating-files-in-c
	FILE* file = fopen(fileNameString.c_str(), "w");

	char buffer[MAXMESG];
	// fill buffer with information to enter while loop
	for (int i = 0; i < MAXMESG; i++) {
		buffer[i] = '.';
	}
	unsigned short expectedBlockNum = 1;

	int rcvlen = sizeof(struct sockaddr);

	timeoutCount = 1;
	signal(SIGALRM,sig_handler); // Register signal handler
	siginterrupt( SIGALRM, 1 );

	while (!tftp::CheckIfLastDataPacket(buffer)) {
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

		std::cout << "received something" << std::endl;
		alarm(0); // turn off alarm
		timeoutCount = 0;

		// get its op code to determine packet type
		unsigned short opValue = tftp::GetPacketOPCode(buffer);
		if (opValue == ERROR) {
			unsigned short errorCode = GetBlockNumber(buffer);
			char* bufpoint = buffer + 4;
			int errMsgLength = 0;
			for (int i = 4; i < MAXMESG; i++) {
				std::cout << buffer[i];
				if (buffer[i] == NULL) {
					std::cout<< "null found in getting ErrMsg"<<std::endl;
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

			if (receivedBlockNum == expectedBlockNum) {
				//write packet to file
				char* bufpoint = buffer + 4;
				int writeSize = 0;
				for (int i = 4; i < MAXDATA; i++) {
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
			std::cout<< "unexpected OP code received:"<<opValue<<" ending process"<<std::endl;
			exit(9);
		}
	}

	// close file after leaving loop
	fclose(file);

}

// return any packet as a string
std::string tftp::PacketToString(char buffer[MAXMESG]) {
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
		std::cout<< "Error. Unsupported OP code received:"<< opNumber <<std::endl;
		exit(6);
	}
	return printResult;
}

// print any and the entire packet to console
void tftp::PrintPacket(char buffer[MAXMESG]) {
	std::cout<< "PRINTING CONTENT OF PACKET:"<<std::endl;
	std::string printResult = PacketToString(buffer);
	std::cout<< printResult <<std::endl;
	std::cout<< "END OF PACKET PRINT" <<std::endl;
}

unsigned short tftp::GetPacketOPCode(char buffer[MAXMESG]) {
	unsigned short* bufferTempPointer = nullptr;
	bufferTempPointer = reinterpret_cast<unsigned short *>(buffer);
	unsigned short opNumb = ntohs(*bufferTempPointer);
	return opNumb;
}

unsigned short tftp::GetBlockNumber(char buffer[MAXMESG]) {
	unsigned short* bufferBPointer = nullptr;
	bufferBPointer = reinterpret_cast<unsigned short *>(buffer + 2);
	unsigned short bNumber = ntohs(*bufferBPointer);
	return bNumber;
}

// help from: https://stackoverflow.com/questions/14871388/convert-from-unsigned-short-to-string-c/14871427
std::string tftp::ConvertUnsignedShortToString(unsigned short number) {
	std::string result = "";
	std::stringstream ss;
	ss << number;
	result = ss.str();
	return result;
}

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

// pass char[] by reference help from: https://stackoverflow.com/questions/12987760/passing-char-array-by-reference
void tftp::GetDataContent(char buffer[MAXMESG], char (& dataArray)[MAXDATA]) {
	for (int i = 4; i < MAXMESG; i++) {
		if (buffer[i] == NULL) {
			break;
		}
		dataArray[i - 4] = buffer[i];
	}
}

int tftp::GetNumberOfRequeiredPackets(std::string filename) {
	std::ifstream file( filename, std::ios::binary | std::ios::ate);
	int fileSize = file.tellg();
	std::cout<< "File size:" << fileSize << " Bytes" <<std::endl;

	if (fileSize < MAXDATA) {
		std::cout<< "only need one packet to send all of the data"<<std::endl;
		return 1;
	}
	int numberOfPackets = (fileSize / MAXDATA) + 1;
	std::cout<< "file requires: " <<numberOfPackets << " number of packets"<<std::endl;
	return numberOfPackets;
}

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

void tftp::CreateAckPacket(char buffer[MAXMESG], unsigned short blockNumber) {
	bzero(buffer, MAXMESG);

	unsigned short ackOpValue = ACK;
	unsigned short* ackOpCodePtr = (unsigned short *) buffer;
	*ackOpCodePtr = htons(ackOpValue);

	unsigned short ackBlockValue = blockNumber;
	unsigned short* ackBlockPtr = (unsigned short *) buffer + 1;
	*ackBlockPtr = htons(ackBlockValue);
}

void tftp::SendPacketHelper(char* progname, int sockfd, char buffer[MAXMESG], struct sockaddr_in receiving_addr) {
	std::cout<< "in tftp::SendPacketHelper()"<<std::endl;
	int n = sendto(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
	if (n < 0) {
		printf("%s: sendto error\n",progname);
		exit(4);
	} else {
		std::cout<< "no issue sending packet" <<std::endl;
	}
}

void tftp::CreateErrorPacketHelper(unsigned short* buffPtr, unsigned short errorCode, std::string errorMessage) {
	unsigned short myErrorCode = errorCode;
	*buffPtr = htons(errorCode);

	buffPtr++;
	std::string myErrorMessage = errorMessage;
	strcpy((char*)buffPtr, myErrorMessage.c_str());
}
