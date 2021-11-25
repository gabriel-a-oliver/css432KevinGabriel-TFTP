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

// to be moved to shared tftp file
#define RRQ	1
#define WRQ 2
#define DATA 3
#define ACK 4
#define	ERROR 5
#define MAXMESG 516

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
	char* mesg;
	mesg = ReceivePacketHelper(sockfd, sending_addr);
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
void tftp::ReceiveMessage(int sockfd, struct sockaddr* sending_addr, struct sockaddr* receiving_addr) {
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
	char* mesg;
	clilen = sizeof(struct sockaddr);
	// Receive Data
	mesg = ReceivePacketHelper(sockfd, sending_addr);
	// Check if receivedpacket is a DATA packet

	int receivedOPCode = std::stoi(std::string(1,mesg[0]) + mesg[1]);
	if (receivedOPCode == ERROR) {

	}
	if (receivedOPCode == DATA) {
		std::string blockNumString = std::string(1, mesg[2]) + mesg[3];
		std::cout << "Data Received! Block Number: " << blockNumString << std::endl;
	} else {
		std::cout << "DATA not received" << std::endl;
	}

	//// STILL NEEDS WORK ////////////////////////
	// Perform more header checks HERE
	/*
	 if (some issue) {
	 	buffer = BuildErrMessage(int blockNumber, reinterpret_cast<char **>(buffer))
	 } else { build acknowledgement}
	*/
	// Unpack message for data //


	// assuming it received a data packet
	char* buffer[MAXMESG];
	BuildAckMessage(/*Get Block Number*/1, reinterpret_cast<char **>(buffer));

	// Send Acknowledgement
	m = sendto(sockfd, *buffer, MAXMESG, 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
	if (m < 0) {
		printf(": sendto error\n");
		exit(3);
	}
}

void tftp::BuildAckMessage(int blockNumber, char* buffer[MAXMESG]) {
	char *bufpoint; // for building packet
	//char buffer[MAXMESG]; // buffer with arbituary 512 size
	*(short *)*buffer = htons(ACK);
	bufpoint = *buffer + 2; // move pointer to file name
	*(short *)buffer = htons(blockNumber);
}

void tftp::BuildErrMessage(int blockNumber, char* buffer[MAXMESG]) {
	char *bufpoint; // for building packet
	//char buffer[512]; // buffer with arbituary 512 size
	*(short *)*buffer = htons(ERROR);
	bufpoint = *buffer + 2; // move pointer to file name
	*(short *)buffer = htons(blockNumber);
}

void tftp::BuildDataMessage(int blockNumber, char* buffer[MAXMESG]) {
	char *bufpoint; // for building packet
	//char buffer[MAXMESG]; // buffer with arbituary 512 size
	*(short *)*buffer = htons(DATA);
	bufpoint = *buffer + 2; // move pointer to file name
	*(short *)buffer = htons(blockNumber);
	bufpoint = *buffer + 4;
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

// returns the content of the byteStream already interpreted as a char*
char* tftp::ReceivePacketHelper(int sockfd, struct sockaddr* sending_addr) {
	std::cout<< "in tftp::ReceivePacketHelper()"<<std::endl;
	int n; // for debugging
	int clilen;
	char mesg[MAXMESG];
	clilen = sizeof(struct sockaddr);
	n = recvfrom(sockfd, mesg, MAXMESG, 0, &*sending_addr, (socklen_t*)&clilen);
	if (n < 0) { // for debugging
		printf(": recvfrom error\n");
		exit(4);
	} else {
		std::cout<< "no issue receiving"<<std::endl;
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

	switch (opNumber) {
		case RRQ:
			mesg[0] = '1';
		case WRQ:
			mesg[0] = '2';
		case DATA:
			mesg[0] = '3';
		case ACK:
			mesg[0] = '4';
		case ERROR:
			mesg[0] = '5';
		default:
			std::cout<< "opNumber error:"<< opNumber <<std::endl;
	}

	//char buffPointer[MAXMESG];
	//*buffPointer = static_cast<char>(ntohs(mesg));

	return mesg;
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
