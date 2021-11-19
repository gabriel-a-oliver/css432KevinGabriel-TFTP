//
// Created by Gabriel Oliver and Kevin Huang on 11/16/2021.
//

#include "tftp.h"

void DetermineOP() {


	//
	// working on
	//
	/*char op; // temporary for now, will initialize from arguments
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
	//*/

}

void RRQ() {

}

void WRQ() {

}

void Ack() {

}

void Err() {

}