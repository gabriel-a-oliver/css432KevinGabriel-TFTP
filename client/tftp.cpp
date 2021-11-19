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


#include "tftp.h"

void tftp::ProcessOP(char op, char *bufpoint, char buffer[512]) {


	switch (op) {
		case 'r':
			*(short *)buffer = htons(RRQ);
			bufpoint = buffer + 2; // move pointer to file name
			strcpy(bufpoint, "test.txt"); // add file name to buffer
			bufpoint += strlen("test.txt") + 1; //move pointer and add null byte
			strcpy(bufpoint, "octet"); // add mode to buffer
			bufpoint += strlen("octet") + 1; // move pointer and add null byte
		case 'w':
			*(short *)buffer = htons(WRQ);
			bufpoint = buffer + 2; // move pointer to file name
			strcpy(bufpoint, "test.txt"); // add file name to buffer
			bufpoint += strlen("test.txt") + 1; //move pointer and add null byte
			strcpy(bufpoint, "octet"); // add mode to buffer
			bufpoint += strlen("octet") + 1; // move pointer and add null byte
		case 'd':
			*(short *)buffer = htons(DATA);
			bufpoint = buffer + 2; // move pointer to file name
			strcpy(bufpoint, "test.txt"); // add file name to buffer
			bufpoint += strlen("test.txt") + 1; //move pointer and add null byte
			strcpy(bufpoint, "octet"); // add mode to buffer
			bufpoint += strlen("octet") + 1; // move pointer and add null byte
		case 'a':
			*(short *)buffer = htons(ACK);
			bufpoint = buffer + 2; // move pointer to file name
			strcpy(bufpoint, "test.txt"); // add file name to buffer
			bufpoint += strlen("test.txt") + 1; //move pointer and add null byte
			strcpy(bufpoint, "octet"); // add mode to buffer
			bufpoint += strlen("octet") + 1; // move pointer and add null byte
		case 'e':
			*(short *)buffer = htons(ERROR);
			bufpoint = buffer + 2; // move pointer to file name
			strcpy(bufpoint, "test.txt"); // add file name to buffer
			bufpoint += strlen("test.txt") + 1; //move pointer and add null byte
			strcpy(bufpoint, "octet"); // add mode to buffer
			bufpoint += strlen("octet") + 1; // move pointer and add null byte
		default:
			std::cout << "Error, non-supported OP code: " << op << std::endl;
			exit(4);
	}

}

void tftp::ProcessMessage() {
	char op; // temporary for now, will initialize from arguments
	char *bufpoint; // for building packet
	char buffer[512]; // buffer with arbituary 512 size

	ProcessOP(op, bufpoint, buffer);
}

void ReadRequest() {

}

void WriteRequest() {

}

void Ack() {

}

void Err() {

}