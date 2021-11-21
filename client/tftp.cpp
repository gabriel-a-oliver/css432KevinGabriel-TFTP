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

// to be moved to shared tftp file
#define RRQ	1
#define WRQ 2
#define DATA 3
#define ACK 4
#define	ERROR 5
#define MAXMESG 512

#include "tftp.h"


void tftp::SendMessage(int sockfd, sockaddr sending_addr, sockaddr_in receiving_addr /*fileName to be sent*/) {
	// NOTE: LOOPS WILL BE REQUIRED FOR CERTAIN FUNCTIONALITIES

	int n, m, clilen;
	char buffer[MAXMESG];

	// Pack message(s) into data from file

	// Send message(s)
	m = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
	if (m < 0) {
		printf("%s: sendto error\n");
		exit(3);
	}

	// Receive Acknowledgement
	char mesg[MAXMESG];
	clilen = sizeof(struct sockaddr);
	n = recvfrom(sockfd, mesg, MAXMESG, 0, &sending_addr, (socklen_t*)&clilen);
	if (n < 0) {
		printf("%s: recvfrom error\n");
		exit(4);
	}
	// Perform more header checks HERE
	/*
	if (some issue) {
		buffer = BuildErrMessage(int blockNumber, reinterpret_cast<char **>(buffer))
	} else { track acknowledgements}
	*/
}


void tftp::ReceiveMessage(int sockfd, sockaddr sending_addr, sockaddr_in receiving_addr) {
	// NOTE: LOOPS WILL BE REQUIRED FOR CERTAIN FUNCTIONALITIES

	int n, m, clilen;
	char mesg[MAXMESG];
	clilen = sizeof(struct sockaddr);

	// Receive Data
	n = recvfrom(sockfd, mesg, MAXMESG, 0, &sending_addr, (socklen_t*)&clilen);
	if (n < 0) {
		printf("%s: recvfrom error\n");
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
		printf("%s: sendto error\n");
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
