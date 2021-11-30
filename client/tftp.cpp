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

// to be moved to shared tftp file
#define RRQ	1
#define WRQ 2
#define DATA 3
#define ACK 4
#define	ERROR 5
#define MAXMESG 516
#define MAXDATA 512

#include "tftp.h"

// This function is called if the server gets an RRQ
// or of the client sends an WRQ
void tftp::SendMessage(int sockfd, struct sockaddr* sending_addr, struct sockaddr* receiving_addr, char* fileName) {
	std::cout<< "in tftp::SendMessage"<<std::endl;
	/* General idea:
	 * receive the socket, the origin address, the recipient's address, and the name of the file to send
	 *
	 * in a loop: can ignore loop implementation for now, just need to send one packet
	 *
	 *
	 * 		create a new packet
	 * 		stream the OP type Data to the packet (2 bytes)
	 * 		stream the block number for this package into the packet (2 bytes)
	 * 		stream data from the file to a DATA packet (up to 512 number of bytes)
	 *
	 *		send packet
	 *		wait for ack
	 *		check it is an ack
	 *			if so, manage acks // for only one packet, the process has completed
	 *			else, if its an error, resend packet
	 *				else, more ACKs management needed
	 *
	 * */





	int m; // for debugging
	m = SendMessageHelper(sockfd, receiving_addr, fileName);
	if (m < 0) { // for debugging
		printf(": sendto error\n");
		exit(3);
	}

	// Hopefully, Receive Acknowledgement
	char mesg[MAXMESG];
	ReceivePacketHelper(sockfd, sending_addr, mesg);
	// break down the package to see if its an ACK or something else.
	int receivedOPCode = std::stoi(std::string(1,mesg[0]) + mesg[1]);
	if (receivedOPCode == ACK) {
		std::string blockNumString = std::string(1,mesg[2]) + mesg[3];
		std::cout << "Acknowledgement Received! Block Number: " << blockNumString << std::endl;
	} else if (receivedOPCode == ERROR) {
		std::cout << "Received ERROR!" << std::endl;
	}

	// Perform more header checks HERE
	/*
	if (some issue) {
		buffer = BuildErrMessage(int blockNumber, reinterpret_cast<char **>(buffer))
	} else { track acknowledgements}
	*/
}

// This function is called if the server receives a WRQ
// or if the client sends an RRQ
void tftp::ReceiveMessage(int sockfd, struct sockaddr* sending_addr, struct sockaddr* receiving_addr, char buffer[MAXMESG]) {
	std::cout<< "tftp::ReceiveMessage()"<<std::endl;
	/* General idea:
	 * receive the socket, the origin address, the recipient's address, and the name of the file want to receive
	 *
	 * in a loop: can ignore loop implementation for now, just need to receive one packet
	 *
	 * 		wait for data packet
	 * 		check if it is a data packet
	 * 			perform various checks and management functions
	 *
	 * 		store data into desired file
	 *
	 * 		create ack packet
	 *		send ack packet
	 *
	 *		wait for no retransmissions
	 *			check for anything regarding this
	 *
	 * */

	// NOTE: LOOPS WILL BE REQUIRED FOR CERTAIN FUNCTIONALITIES

	int n, m, clilen;
	//char buffer[MAXMESG];
	bzero(buffer, (MAXMESG));
	clilen = sizeof(struct sockaddr);
	// Receive Something
	ReceivePacketHelper(sockfd, sending_addr, buffer);

	// translating it aback to ntohs
	unsigned short* bufferPointer = nullptr;
	bufferPointer = reinterpret_cast<unsigned short *>(buffer);
	unsigned short opNumb = ntohs(*bufferPointer);
	int opCode = (int)opNumb;
	std::cout << "convert ntohs op: " << opCode << std::endl;

	if (opCode == RRQ) {
		std::cout<< "RRQ received"<<std::endl;
	} else if (opCode == WRQ) {
		std::cout<< "WRQ received"<<std::endl;
	}else if (opCode == DATA) {
		std::cout<< "DATA received"<<std::endl;
	} else if (opCode == ACK) {
		std::cout<< "ACK received"<<std::endl;
	} else if (opCode == ERROR) {
		std::cout<< "ERROR received"<<std::endl;
	} else {
		std::cout<< "Error: Unsupported OP code received:" << opCode <<std::endl;
	}


	/*// assuming it received a data packet
	char* buffer[MAXMESG];
	BuildAckMessage(*//*Get Block Number*//*1, reinterpret_cast<char **>(buffer));

	// Send Acknowledgement
	m = sendto(sockfd, *buffer, MAXMESG, 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
	if (m < 0) {
		printf(": sendto error\n");
		exit(3);
	}*/
}

// returns the content of the byteStream already interpreted as a char*
void tftp::ReceivePacketHelper(int sockfd, struct sockaddr* sending_addr, char mesg[MAXMESG]) {
	std::cout<< "in tftp::ReceivePacketHelper()"<<std::endl;
	int n; // for debugging
	int clilen;
	//char mesg[MAXMESG];
	clilen = sizeof(struct sockaddr);
	n = recvfrom(sockfd, mesg, MAXMESG, 0, &*sending_addr, (socklen_t*)&clilen);
	if (n < 0) { // for debugging
		printf(": recvfrom error\n");
		exit(4);
	} else {
		std::cout<< "no issue receiving"<<std::endl;
	}
/*
	// translating it aback to ntohs
	unsigned short* bufferPointer = nullptr;
	bufferPointer = reinterpret_cast<unsigned short *>(mesg);
	unsigned short opNumb = ntohs(*bufferPointer);
	std::cout << "convert ntohs op: " << opNumb << std::endl;

	switch (opNumb) {
		case RRQ:
			//mesg[0] = '1';
		case WRQ:
			//mesg[0] = '2';
		case DATA:
			//mesg[0] = '3';
		case ACK:
			//mesg[0] = '4';
		case ERROR:
			//mesg[0] = '5';
		default:
			std::cout<< "opNumber error. Received:"<< opNumb <<std::endl;
	}


	std::cout<< "whole buffer after being received:" << *mesg << *mesg + 1;
	for (int i = 2; i < MAXMESG; ++i) {
		std::cout<< mesg[i];
	}
	std::cout<<std::endl;

	std::cout<< "testing converting back with ntohs:";
	unsigned short testNumShort = (*mesg + 1 << *mesg);
	int opNumber = (int)testNumShort;
	std::cout<< opNumber<<std::endl;




	//char buffPointer[MAXMESG];
	//*buffPointer = static_cast<char>(ntohs(mesg));*/

	//return mesg;
}

void tftp::BuildAckMessage(int blockNumber, char buffer[MAXMESG]) {
	char *bufpoint; // for building packet
	//char buffer[MAXMESG]; // buffer with arbituary 512 size
	*(short *)*buffer = htons(ACK);
	bufpoint = buffer + 2; // move pointer to file name
	*(short *)buffer = htons(blockNumber);
}

void tftp::BuildErrMessage(int blockNumber, char buffer[MAXMESG]) {
	char *bufpoint; // for building packet
	//char buffer[512]; // buffer with arbituary 512 size
	*(short *)*buffer = htons(ERROR);
	bufpoint = buffer + 2; // move pointer to file name
	*(short *)buffer = htons(blockNumber);
}

void tftp::BuildDataMessage(int blockNumber, char buffer[MAXMESG]) {
	char *bufpoint; // for building packet
	//char buffer[MAXMESG]; // buffer with arbituary 512 size
	*(short *)*buffer = htons(DATA);
	bufpoint = buffer + 2; // move pointer to file name
	*(short *)buffer = htons(blockNumber);
	bufpoint = buffer + 4;
}

int tftp::SendMessageHelper(int sockfd, struct sockaddr* receiving_addr, char* fileName) {
	std::cout<< "tftp::SendMessageHelper()"<<std::endl;

	fileName = const_cast<char*>("ClientTest.txt"); // temporary for testing

	// NOTE: LOOPS WILL BE REQUIRED FOR CERTAIN FUNCTIONALITIES
	char buffer[MAXMESG];
	int blockNumber = 1; // temporary for testing

	// Pack message(s) into data from file
	// For right now, its only one packet for a 512 byte file
	char *bufpoint; // for building packet

	// setting OP code for DATA
	unsigned short htonsNum = 3;
	*buffer = htons(htonsNum);

	//*(short *)buffer = htons(DATA);
	bufpoint = buffer + 2; // move pointer to block number
	// setting blockNumber for packet
	htonsNum = (unsigned short)blockNumber;
	*buffer = htons(htonsNum);

	//strcpy(bufpoint, reinterpret_cast<const char *>(blockNumber)); // add file name to buffer
	bufpoint = buffer + 4; //move pointer to index 5 to fill with data






	// From: https://stackoverflow.com/questions/36658734/c-get-all-bytes-of-a-file-in-to-a-char-array/36658802
	//open file
	std::ifstream infile("ClientTest.text");
	//read file
	if (!(infile.read(bufpoint, sizeof(buffer)-4))) // read up to the size of the buffer
	{
		if (!infile.eof()) // end of file is an expected condition here and not worth
			// clearing. What else are you going to read?
		{
			std::cout << "reached end of buffer with more data to read" << std:: endl;
			// something went wrong while reading. Find out what and handle.
		}
	}
	//////////////////////////////////////////////////////////////////////////////////////////////

	int m;
	// Send message(s)
	m = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
	return m;
}

char** tftp::GetFileData(char* fileName) {

	//open and reading Linux commands:
#define MAXDATA 512

	int fd = open(fileName, O_RDONLY); // open text file
	std::cout<< "fd value:" << fd <<std::endl;
	if (fd < 0) {
		std::cout<< "linux open file error"<<std::endl;
		// error opening fileName
	} else {
		std::cout<< "no issue opening file"<<std::endl;
	}

	char data[MAXDATA];
	bzero(data, sizeof(data));
	int result = read(fd, data, MAXDATA); // read up to MAXDATA bytes
	// if result == 0: end of file; if result > 0: error

	// create DATA packet and call sendto

	close(fd); // once finish reading whole file, close text file
	return nullptr;
}

void tftp::WriteToFile(char *fileName, char *dataBuffer) {
	std::cout<< "in tftp::WriteToFile()"<<std::endl;

	std::cout<< "printing whole buffer before writting to file"<<std::endl;
	for (int i = 0; i < MAXMESG; ++i) {
		if (dataBuffer[i] == NULL)
		{
			std::cout<< " ";
		}
		std::cout<< dataBuffer[i];
	}
	std::cout<<std::endl << "END OF FILE DATA" << std::endl;

	// Checks if file exists
	std::ifstream infile(fileName);
	if (infile.good()) {
		std::cout<< "File exists, writing to existing file:"<<std::endl;
		// File exists, write starting at the last point of the file
		// help from: https://mathbits.com/MathBits/CompSci/Files/End.htm
		int filePosition = 0;
		char datum;
		std::ifstream fin;

		fin.open(fileName);
		assert (!fin.fail());
		fin >> datum;

		while(!fin.eof()) {
			std::cout<< datum <<std::endl;
			fin >> datum;
			filePosition++;
		}

		fin.close();
		assert(!fin.fail());

		// write to file
		std::fstream output;
		output.open(fileName);
		for (int i = 0; i < sizeof(dataBuffer); i++) {
			if (dataBuffer[i] == NULL) {
				std::cout << std::endl << "Encountered null in file. Ending writing"<<std::endl;
				break;
			}
			output.seekp(filePosition);
			output.put(dataBuffer[i]);
			filePosition++;
		}

	} else {
		std::cout<< "File does not exist, creating and writing to new file"<<std::endl;
		// File doesn't exist. create new file with provided filename and write starting at the beginning
		// help from: https://stackoverflow.com/questions/478075/creating-files-in-c
		std::string myFileName = fileName;
		std::ofstream outfile (myFileName);
		std::cout << "pushing data to file:" << std::endl;
		for (int i = 0; i < MAXDATA; i++) {
			if (dataBuffer[i] == NULL) {
				std::cout<< "encountered null in file"<<std::endl;
				break;
			}
			std::cout << dataBuffer[i];
			outfile << dataBuffer[i];
		}
		std::cout << std::endl << "End of data pushing to file" << std::endl;

		outfile.close();
	}
}

// return any packet as a string
std::string tftp::PacketToString(char buffer[MAXMESG]) {
	unsigned short opNumber = tftp::GetPacketOPCode(buffer);
	unsigned short blockNumber = NULL;
	//std::cout<< opTempNumber; // This printing is wrong.  cout prints the bytes in ascii format, the value at buffer[1] is 1 , which is a non-printable ascii character, so you won't see anything on screen
	//Instead, print the hex value of first two bytes. should be 0,1 for RRQ, 0,2 for WRQ
	std::string opStr = "";
	std::string blockStr = "";
	std::string fileNameStr = "";
	std::string modeStr = "";
	char dataArray[MAXDATA];
	bzero(dataArray, sizeof(dataArray));
	std::string dataStr = "";
	unsigned short errorNumber = NULL;
	std::string errorCodeStr = "";
	char errorMessageArray[MAXDATA];
	bzero(errorMessageArray, sizeof(errorMessageArray));
	std::string errorMessageStr = "";

	std::string printResult = "";

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
		tftp::GetDataContent(buffer, dataArray);
		dataStr = std::string(dataArray);
		printResult = opStr + blockStr + dataStr;
	} else if (opNumber == ACK) {
		opStr = "4";
		blockNumber = tftp::GetBlockNumber(buffer);
		blockStr = tftp::ConvertUnsignedShortToString(blockNumber);
	} else if (opNumber == ERROR) {
		opStr = "5";
		errorNumber = tftp::GetBlockNumber(buffer);
		errorCodeStr = ConvertUnsignedShortToString(errorNumber);
		tftp::GetDataContent(buffer, errorMessageArray);
		errorMessageStr = std::string(errorMessageArray);
		printResult = opStr + errorCodeStr + errorMessageStr;
	} else {
		std::cout<< "Error. Unsupported OP code received:"<< opNumber <<std::endl;
		exit(6);
	}
	return printResult;
}

// print any and the entire packet to console
void tftp::PrintPacket(char buffer[MAXMESG]) {
	std::cout<< "Printing Content of Packet:"<<std::endl;
	std::string printResult = PacketToString(buffer);
	std::cout<< printResult <<std::endl;
	std::cout<< "END OF PACKET PRINT" <<std::endl;

	/*std::cout<< "Printing:";


	printf("%x,%x", dataBuffer[0], dataBuffer[1]);
	unsigned short blockNum = ntohs(dataBuffer[3]);
	std::cout<< blockNum; // This printing is wrong.  cout prints the bytes in ascii format, the value at buffer[1] is 1 , which is a non-printable ascii character, so you won't see anything on screen
	//Instead, print the hex value of first two bytes. should be 0,1 for RRQ, 0,2 for WRQ
	printf("%x,%x", dataBuffer[2], dataBuffer[3]);
	for (int i = 4; i < MAXMESG; ++i) {
		if (dataBuffer[i] == NULL)
		{
			std::cout<< " ";
		}
		std::cout<< dataBuffer[i];
	}
	std::cout<<std::endl << "END OF FILE DATA" << std::endl;*/
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
	bcopy(bufpoint, filename, fileNameLength);
	std::string result = std::string(filename);
	return result;
}

// This function struggles to actually work, caution!!!
void tftp::GetFileNameCharPointer(char* filename, char buffer[MAXMESG]) {
	char* bufpoint = buffer + 2;
	int fileNameLength = 0;
	for (int i = 2; i < MAXMESG; i++) {
		if (buffer[i] == NULL) {
			break;
		}
		fileNameLength++;
	}
	char tempFilename[fileNameLength];
	bcopy(bufpoint, tempFilename, fileNameLength);
	bcopy(tempFilename, filename, fileNameLength);
	//filename = tempFilename;
	//std::string result = std::string(filename);
	//return filename;
}

std::string tftp::GetMode(char buffer[MAXMESG], std::string fileName) {
	std::string result = "";
	int modeCharLength = 0;
	//std::cout<< "creating RRQ packet" <<std::endl;
	char* bufpoint;
	bufpoint = buffer + 2; // move pointer to file name
	bufpoint += fileName.length() + 1; //move pointer and add null byte
	//strcpy(bufpoint, "octet"); // add mode to buffer
	for (int i = 2 + fileName.length() + 1; i < (MAXMESG - (2 + fileName.length() + 1)); i++) {
		if (buffer[i] == NULL) {
			break;
		}
		modeCharLength++;
	}
	char modeChar[modeCharLength];
	bcopy(bufpoint, modeChar, modeCharLength);
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