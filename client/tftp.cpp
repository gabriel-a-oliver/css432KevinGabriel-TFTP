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
void tftp::SendMessage(int sockfd, sockaddr sending_addr, sockaddr_in receiving_addr, char* fileName) {
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
void tftp::ReceiveMessage(int sockfd, sockaddr sending_addr, sockaddr receiving_addr) {
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

int tftp::SendMessageHelper(int sockfd, sockaddr_in receiving_addr, char* fileName) {
	fileName = const_cast<char*>("ClientTest.txt"); // temporary for testing

	// NOTE: LOOPS WILL BE REQUIRED FOR CERTAIN FUNCTIONALITIES
	char buffer[MAXMESG];
	int blockNumber = 1; // temporary for testing

	// Pack message(s) into data from file
	// For right now, its only one packet for a 512 byte file
	char *bufpoint; // for building packet
	*(short *)buffer = htons(DATA);
	bufpoint = buffer + 2; // move pointer to block number
	strcpy(bufpoint, reinterpret_cast<const char *>(blockNumber)); // add file name to buffer
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
char* tftp::ReceivePacketHelper(int sockfd, sockaddr sending_addr) {
	int n; // for debugging
	int clilen;
	uint16_t mesg = 0;
	clilen = sizeof(struct sockaddr);
	n = recvfrom(sockfd, reinterpret_cast<void *>(mesg), MAXMESG, 0, &sending_addr, (socklen_t*)&clilen);
	if (n < 0) { // for debugging
		printf(": recvfrom error\n");
		exit(4);
	}
	char buffPointer[MAXMESG];
	*buffPointer = static_cast<char>(ntohs(mesg));

	return buffPointer;
}
