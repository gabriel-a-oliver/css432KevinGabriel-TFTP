//
// Created by Gabriel Oliver and Kevin Huang on 11/16/2021.
//

#ifndef CSS432KEVINGABRIEL_TFTP_H
#define CSS432KEVINGABRIEL_TFTP_H
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

void RRQ();
void WRQ();

void Ack();
void Err();



#endif //CSS432KEVINGABRIEL_TFTP_H
