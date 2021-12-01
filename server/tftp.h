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


class tftp {

// to be moved to shared tftp file
#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5

public:
	static void SendMessage(int sockfd, struct sockaddr* sending_addr, struct sockaddr* receiving_addr, char* fileName);
	static void ReceiveMessage(int sockfd, struct sockaddr* sending_addr, struct sockaddr* receiving_addr, char buffer[MAXMESG]);
	static void BuildAckMessage(int blockNumber, char buffer[MAXMESG]);
	static void BuildErrMessage(int blockNumber, char buffer[MAXMESG]);
	static void BuildDataMessage(int blockNumber, char buffer[MAXMESG]);
	static void WriteToFile(char *fileName, char *dataBuffer);
    static void CreateDataPacket(std::string fileName, char fileBuffer[MAXMESG]);
	static void PrintPacket(char buffer[MAXMESG]);
	static std::string PacketToString(char buffer[MAXMESG]);
	static unsigned short GetPacketOPCode(char buffer[MAXMESG]);
	static unsigned short GetBlockNumber(char buffer[MAXMESG]);
	static std::string ConvertUnsignedShortToString(unsigned short number);
	static std::string GetFileNameStr(char buffer[MAXMESG]);
	static void GetFileNameCharPointer(char* filename, char buffer[MAXMESG]);
	static std::string GetMode(char buffer[MAXMESG], std::string fileName);
	static void GetDataContent(char buffer[MAXMESG], char (& dataArray)[MAXDATA]);
private:
	static int SendMessageHelper(int sockfd, struct sockaddr* receiving_addr, char* fileName);
	static void ReceivePacketHelper(int sockfd, struct sockaddr* sending_addr, char mesg[MAXMESG]);
	static char** GetFileData(char* fileName);

};
#endif //CSS432KEVINGABRIEL_TFTP_H
