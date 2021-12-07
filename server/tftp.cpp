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

// max sizes
#define MAXMESG 516
#define MAXDATA 512

#include "tftp.h"



/*
// timeout implementation
#define TIMEOUT_TIME 3
void sig_handler(int signum){
    printf("Inside timeout handler function\n");
    // retransmit packet here?
    alarm(TIMEOUT_TIME); // Schedule a new alarm
}
*/



void tftp::SendFile(char *progname, int sockfd, struct sockaddr_in receiving_addr, int clilen, char buffer[MAXMESG], /*char fileBuffer[MAXMESG],*/ std::string fileName) {
	std::cout<< "In tftp::SendFile()"<<std::endl;
	int numberOfRequiredPackets = GetNumberOfRequeiredPackets(fileName);
	char packetsList[numberOfRequiredPackets][MAXMESG];
	for (int i = 0; i < numberOfRequiredPackets; i++) {
		bzero(packetsList[i], MAXMESG);
	}

	// open file
	std::cout << "File Name:" << fileName <<std::endl;

	std::ifstream infile(fileName);
	if (infile.good()){
		std::cout<< "file exists" <<std::endl;
		// help from: https://stackoverflow.com/questions/17032970/clear-data-inside-text-file-in-c
	} else {
		std::cout<< "file does not exist, send error back"<<std::endl;
		// help from: https://stackoverflow.com/questions/478075/creating-files-in-c

		// create ERROR packet
		bzero(buffer, sizeof(buffer));
		std::cout<< "creating ERROR packet" <<std::endl;
		unsigned short opValue = ERROR;
		unsigned short* buffPtr = (unsigned short *) buffer;
		*buffPtr = htons(opValue);
		std::cout<< "OP is: " << opValue <<std::endl;

		buffPtr++;
		unsigned short errorCode = NO_FILE;
		*buffPtr = htons(errorCode);
		std::cout<< "Error Code is : " << errorCode <<std::endl;

		buffPtr++;
		std::string errormessage = "File not found.";
		strcpy((char*)buffPtr, errormessage.c_str());

		// Send the ERROR packet
		std::cout<< "sending ERROR packet" <<std::endl;
		int n = sendto(sockfd, buffer, MAXMESG/*sizeof(fileBuffer)*/, 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
		if (n < 0) {
			printf("%s: sendto error\n",progname);
			exit(4);
		} else {
			std::cout<< "no issue sending packet" <<std::endl;
		}

		exit(99); // temporary, should just be sending back error instead of exiting
	}

	std::fstream readFile (fileName);

	// create data packets
	int fileStartIterator = 0;
	std::cout<< "fileStartIterator:" <<fileStartIterator<<std::endl;
	for (int i = 0; i < numberOfRequiredPackets; i++) {
		// create and initialize buffer
		// fill buffer with up to MAXDATA amount of bytes
		// store buffer i packetsList[i]
		//CreateDataPacket(readFile, packetsList[i], fileStartIterator);



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


		int n = readFile.readsome(bufpoint, MAXDATA);
		fileStartIterator += n;
		//int n = read(fd, bufpoint, MAXDATA); // read up to MAXDATA bytes
		if (n < 0) {
			std::cout<< "read error:" << n <<std::endl;
		} else {
			std::cout << "read successful:" << n << std::endl;
		}




		std::cout<< "fileStartIterator:" <<fileStartIterator<<std::endl;
		PrintPacket(packetsList[i]);
	}
	// close file
	readFile.close(); // once finish reading whole file, close text file
	std::cout<< "closed file"<<std::endl;


	for (int i = 0; i < numberOfRequiredPackets; i++) {
		// Send the data packet
		std::cout<< "sending data packet" <<std::endl;
		int n = sendto(sockfd, packetsList[i], MAXMESG/*sizeof(fileBuffer)*/, 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
		if (n < 0) {
			printf("%s: sendto error\n",progname);
			exit(4);
		} else {
			std::cout<< "no issue sending packet" <<std::endl;
		}



		/*
		// timeout implementation
		int timeoutCount = 0;
		signal(SIGALRM,sig_handler); // Register signal handler
		alarm(TIMEOUT_TIME); // set timer
		alarm(0); // turn off alarm
		*/



		// Wait to receive ACK from
		std::cout<< "Waiting to receive ack from client"<<std::endl;
		bzero(buffer, sizeof(buffer));
		n = recvfrom(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &receiving_addr, (socklen_t*)&clilen);
		if (n < 0) {
			printf("%s: recvfrom error\n",progname);
			exit(4);
		}
		std::cout << "received something" << std::endl;

		// check if received packet is the ack
		PrintPacket(buffer);
		unsigned short ackOpNumb = tftp::GetPacketOPCode(buffer);
		if (ackOpNumb == ACK) {
			std::cout<< "ack received. transaction complete for block:"<< tftp::GetBlockNumber(buffer) <<std::endl;
		} else {
			std::cout<< "no ack received. received:"<<ackOpNumb<<std::endl;
		}
	}

}

// This function is called if the server gets an RRQ
// or of the client sends an WRQ
/*void tftp::SendMessage(int sockfd, struct sockaddr* sending_addr, struct sockaddr* receiving_addr, char* fileName) {
	std::cout<< "in tftp::SendMessage"<<std::endl;

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
	*//*
	if (some issue) {
		buffer = BuildErrMessage(int blockNumber, reinterpret_cast<char **>(buffer))
	} else { track acknowledgements}
	*//*
}*/

void tftp::ReceiveFile(char *progname, int sockfd, struct sockaddr_in sending_addr, std::string fileNameString) {

	/*
	 * tftp::ReceiveFile should do:
	 *
	 * open file
	 * loop until last packet is received
	 * 		receivePacket
	 * 		check what type of packet
	 * 		if error
	 * 			not sure what to do here, we will work it out later. probably just end the process and delete everything
	 * 		if data
	 * 			extract block#
	 * 			if expected block number
	 *				write data to file
	 *				send back appropriate ack packet
	 *				save last packet to be checked if it's the last one
	 * 			if not expected block number
	 * 				send appropriate ack packet
	 *
	 * close file
	 * */


	// check file exists
	std::ifstream infile(fileNameString);
	if (infile.good()){
		std::cout<< "file exists, deleting data before writing to it"<<std::endl;

		// create ERROR packet
		char errBuff[MAXMESG];
		bzero(errBuff, sizeof(errBuff));
		std::cout<< "creating ERROR packet" <<std::endl;
		unsigned short opValue = ERROR;
		unsigned short* buffPtr = (unsigned short *) errBuff;
		*buffPtr = htons(opValue);
		std::cout<< "OP is: " << opValue <<std::endl;

		buffPtr++;
		unsigned short errorCode = NO_FILE;
		*buffPtr = htons(errorCode);
		std::cout<< "Error Code is : " << errorCode <<std::endl;

		buffPtr++;
		std::string errormessage = "File already exists.";
		strcpy((char*)buffPtr, errormessage.c_str());

		// Send the ERROR packet
		std::cout<< "sending ERROR packet" <<std::endl;
		int n = sendto(sockfd, errBuff, MAXMESG/*sizeof(fileBuffer)*/, 0, (struct sockaddr *) &sending_addr, sizeof(sending_addr));
		if (n < 0) {
			printf("%s: sendto error\n",progname);
			exit(4);
		} else {
			std::cout<< "no issue sending packet" <<std::endl;
		}

		exit(99); // temporary, should just be sending back error instead of exiting

		/*
		// help from: https://stackoverflow.com/questions/17032970/clear-data-inside-text-file-in-c
		std::ofstream ofs;
		ofs.open(fileNameString, std::ofstream::out | std::ofstream::trunc);
		ofs.close();
		*/
	} else {
		std::cout<< "file does not exist, creating file of same name"<<std::endl;
		// help from: https://stackoverflow.com/questions/478075/creating-files-in-c
		std::ofstream outfile(fileNameString);
	}

	// open file to be written at the end of the file
	//std::ofstream writeFile(fileNameString);
	std::ofstream writeFile (fileNameString);
	//writeFile.open(fileNameString, std::ios::app);

	char buffer[MAXMESG];
	// fill buffer with information to enter while loop
	for (int i = 0; i < MAXMESG; i++) {
		buffer[i] = '.';
	}
	unsigned short expectedBlockNum = 1;

	while (!tftp::CheckIfLastDataPacket(buffer)) {
		// empty buffer of any previous data
		bzero(buffer, MAXMESG);

		// get latest packet
		tftp::ReceivePacketHelper(sockfd, (struct sockaddr *) &sending_addr, buffer);
		PrintPacket(buffer);

		// get its op code to determine packet type
		unsigned short opValue = tftp::GetPacketOPCode(buffer);
		if (opValue == ERROR) {
			std::cout<< "received error. end everything"<<std::endl;
			exit(8);
		} else
		if (opValue == DATA) {
			// get packet block number
			unsigned short receivedBlockNum = tftp::GetBlockNumber(buffer);

			if (receivedBlockNum == expectedBlockNum) {
				//write packet to file
				for (int i = 4; i < MAXMESG; i++) {
					if (buffer[i] == NULL) {
						break;
					}
					//std::cout <<"i:" << i << buffer[i] <<std::endl;
					writeFile << buffer[i];
				}

				expectedBlockNum++;
			}
			// regardless if expected packet, send ack packet of received block number
			char ackBuffer[MAXMESG];
			bzero(ackBuffer, MAXMESG);
			tftp::CreateAckPacket(ackBuffer, receivedBlockNum);
			int n = sendto(sockfd, ackBuffer, MAXMESG, 0, (struct sockaddr *) &sending_addr, sizeof(sending_addr));
			if (n < 0) {
				printf("%s: sendto error\n",progname);
				exit(4);
			} else {
				std::cout<< "no issue sending packet" <<std::endl;
			}
		}
		else {
			std::cout<< "unexpected OP code received:"<<opValue<<" ending process"<<std::endl;
			exit(9);
		}
	}

	// close file after leaving loop
	infile.close();




	///////////////////////////////OLD CODE/////////////////////////////////////////////////////////////////////////////////////////
	/*// receiving DATA
	ReceiveMessage(sockfd, (struct sockaddr *) &sending_addr, (struct sockaddr *) &receiving_addr, buffer);

	PrintPacket(buffer);

	char* fileContentBuffer = buffer + 4;
    char* filename = &fileNameString[0];

	WriteToFile(filename, fileContentBuffer);

	// create ACK packet
	unsigned short opNumb = tftp::GetPacketOPCode(buffer);
	if (opNumb == DATA) {

        // checking if data packet is the last one
        if (tftp::CheckIfLastDataPacket(buffer)) {
            std::cout<< "Last data packet!"<<std::endl;
        } else {
            std::cout<< "not last data packet, expect more!"<<std::endl;
        }

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

        PrintPacket(buffer);

        std::cout<< "sending packet" <<std::endl;
        int n = sendto(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &sending_addr, sizeof(sending_addr));
        if (n < 0) {
            printf("%s: sendto error\n",progname);
            exit(4);
        } else {
            std::cout<< "no issue sending packet" <<std::endl;
        }
    }*/
}

// This function is called if the server receives a WRQ
// or if the client sends an RRQ
void tftp::ReceiveMessage(int sockfd, struct sockaddr* sending_addr, char buffer[MAXMESG]) {
	std::cout<< "tftp::ReceiveMessage()"<<std::endl;

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
}

/*void tftp::BuildAckMessage(int blockNumber, char buffer[MAXMESG]) {
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
}*/

/*int tftp::SendMessageHelper(int sockfd, struct sockaddr* receiving_addr, char* fileName) {
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
}*/

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

// assumes the file is already open and writing to end of file
void tftp::WriteToFile(std::ofstream writeFile, char *dataBuffer) {

	for (int i = 0; i < sizeof(dataBuffer); i++) {
		writeFile << dataBuffer[i];
	}

	/*std::cout<< "in tftp::WriteToFile()"<<std::endl;

	std::cout<< "printing whole buffer before writing to file"<<std::endl;
	tftp::PrintPacket(dataBuffer);

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
	}*/
}

void tftp::CreateDataPacket(std::fstream readFile, char fileBuffer[MAXMESG], int& fileStartIterator) {
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
	/*std::cout << "File Name:" << fileName <<std::endl;
	int fd = open(const_cast<char*>(fileName.c_str()), O_RDONLY); // open text file
	std::cout<< "fd value:" << fd <<std::endl;
	if (fd < 0) {
		std::cout<< "linux open file error"<<std::endl;
		std::cout << "in the future, send an ERROR packet. but for now, just end program" << std::endl;
		exit(7);
		// error opening fileName
	} else {
		std::cout<< "no issue opening file"<<std::endl;
	}*/

	//char data[MAXDATA];
	//bzero(bufpoint, MAXDATA);

	// I get a segmentation fault when ere are multiple packets. that's because we cannot do a pFile + int
	//int n = fread(bufpoint, 1, MAXDATA, pFile + fileStartIterator);
	int n = readFile.readsome(fileBuffer, MAXMESG);
	fileStartIterator += n;
	//int n = read(fd, bufpoint, MAXDATA); // read up to MAXDATA bytes
	if (n < 0) {
		std::cout<< "read error:" << n <<std::endl;
	} else {
		std::cout << "read successful:" << n << std::endl;
	}
	// if result == 0: end of file; if result < 0: error

	// create DATA packet and call sendto

	//close(fd); // once finish reading whole file, close text file
	///////////////////////////////////////////////////////////////////////////
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
		printResult = opStr + blockStr;
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
		std::cout << buffer[i];
		if (buffer[i] == NULL) {
			std::cout<< "null found in getting file name"<<std::endl;
			break;
		}
		fileNameLength++;
	}
	char filename[fileNameLength];
	bcopy(bufpoint, filename, fileNameLength + 1);
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
	//char* ackBufPoint = buffer + 2;

	unsigned short ackOpValue = ACK;
	unsigned short* ackOpCodePtr = (unsigned short *) buffer;
	*ackOpCodePtr = htons(ackOpValue);

	unsigned short ackBlockValue = blockNumber;
	unsigned short* ackBlockPtr = (unsigned short *) buffer + 1;
	*ackBlockPtr = htons(ackBlockValue);
}
