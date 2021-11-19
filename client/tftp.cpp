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

// to be moved to shared tftp file
#define RRQ	1
#define WRQ 2
#define DATA 3
#define ACK 4
#define	ERROR 5


#include "tftp.h"

void DetermineOP(char op) {


	//
	// working on
	//
	//char op; // temporary for now, will initialize from arguments
	char *bufpoint; // for building packet
	char buffer[512]; // buffer with arbituary 512 size
	if (op == 'r') {
		*(short *)buffer = htons(RRQ);
		bufpoint = buffer + 2; // move pointer to file name
		strcpy(bufpoint, "test.txt"); // add file name to buffer
		bufpoint += strlen("test.txt") + 1; //move pointer and add null byte
		strcpy(bufpoint, "octet"); // add mode to buffer
		bufpoint += strlen("octet") + 1; // move pointer and add null byte
	}
	if (op == 'w') {
		*(short *)buffer = htons(WRQ);
		bufpoint = buffer + 2; // move pointer to file name
		strcpy(bufpoint, "test.txt"); // add file name to buffer
		bufpoint += strlen("test.txt") + 1; //move pointer and add null byte
		strcpy(bufpoint, "octet"); // add mode to buffer
		bufpoint += strlen("octet") + 1; // move pointer and add null byte
	}
	//
	//
	//

}

void RRQ() {

}

void WRQ() {

}

void Ack() {

}

void Err() {

}