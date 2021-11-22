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
	 *
	 * */

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

	int n, m, clilen;
	// Send message(s)
	m = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
	if (m < 0) {
		printf(": sendto error\n");
		exit(3);
	}

	// Receive Acknowledgement
	char mesg[MAXMESG];
	clilen = sizeof(struct sockaddr);
	n = recvfrom(sockfd, mesg, MAXMESG, 0, &sending_addr, (socklen_t*)&clilen);
	if (n < 0) {
		printf(": recvfrom error\n");
		exit(4);
	}
	// Perform more header checks HERE
	/*
	if (some issue) {
		buffer = BuildErrMessage(int blockNumber, reinterpret_cast<char **>(buffer))
	} else { track acknowledgements}
	*/
}

// This function is called if the server receives a WRQ
// or
void tftp::ReceiveMessage(int sockfd, sockaddr sending_addr, sockaddr_in receiving_addr) {
	// NOTE: LOOPS WILL BE REQUIRED FOR CERTAIN FUNCTIONALITIES

	int n, m, clilen;
	char mesg[MAXMESG];
	clilen = sizeof(struct sockaddr);

	// Receive Data
	n = recvfrom(sockfd, mesg, MAXMESG, 0, &sending_addr, (socklen_t*)&clilen);
	if (n < 0) {
		printf(": recvfrom error\n");
		exit(3);
	}

	char buffer[MAXMESG];
	// Perform more header checks HERE
	/*
	 if (some issue) {
	 	buffer = BuildErrMessage(int blockNumber, reinterpret_cast<char **>(buffer))
	 } else { build acknowledgement}
	 */
	BuildAckMessage(/*Get Block Number*/1, reinterpret_cast<char **>(buffer));

	// Unpack message for data //

	// Send Acknowledgement
	m = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
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