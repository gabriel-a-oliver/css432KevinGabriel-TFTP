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


void tftp::SendMessage(int sockfd, sockaddr sending_addr, sockaddr_in receiving_addr) {
	int n, m, clilen;
	// Send Data
	char buffer[MAXMESG];
	m = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
	if (m < 0) {
		printf("%s: sendto error\n");
		exit(3);
	}
	// Perform more header checks HERE

	// Pack message(s) into data from file

	// Receive Acknowledgement
	char mesg[MAXMESG];
	clilen = sizeof(struct sockaddr);
	n = recvfrom(sockfd, mesg, MAXMESG, 0, &sending_addr, (socklen_t*)&clilen);
	if (n < 0) {
		printf("%s: recvfrom error\n");
		exit(4);
	}
}


void tftp::ReceiveMessage(int sockfd, sockaddr sending_addr, sockaddr_in receiving_addr) {
	int n, m, clilen;
	char mesg[MAXMESG];
	clilen = sizeof(struct sockaddr);

	// Receive Data
	n = recvfrom(sockfd, mesg, MAXMESG, 0, &sending_addr, (socklen_t*)&clilen);
	if (n < 0) {
		printf("%s: recvfrom error\n");
		exit(3);
	}
	// Perform more header checks HERE

	// Unpack message for data

	// Send Acknowledgement
	char buffer[MAXMESG];
	m = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &receiving_addr, sizeof(receiving_addr));
	if (m < 0) {
		printf("%s: sendto error\n");
		exit(3);
	}
}

char* tftp::BuildAckMessage(int blockNumber) {
	char *bufpoint; // for building packet
	char buffer[512]; // buffer with arbituary 512 size
	*(short *)buffer = htons(ACK);
	bufpoint = buffer + 2; // move pointer to file name
	*(short *)buffer = htons(blockNumber);

	return buffer;
}

char* tftp::BuildErrMessage(int blockNumber) {
	char *bufpoint; // for building packet
	char buffer[512]; // buffer with arbituary 512 size
	*(short *)buffer = htons(ERROR);
	bufpoint = buffer + 2; // move pointer to file name
	*(short *)buffer = htons(blockNumber);

	return buffer;
}
