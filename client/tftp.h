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
	static void SendFile(char *progname, int sockfd, struct sockaddr_in pcli_addr, int clilen, char buffer[MAXMESG], /*char fileBuffer[MAXMESG],*/ std::string fileName);
	static void SendMessage(int sockfd, struct sockaddr* sending_addr, struct sockaddr* receiving_addr, char* fileName);
	static void ReceiveFile(char *progname, int sockfd, struct sockaddr_in sending_addr, std::string fileNameString);
	static void ReceiveMessage(int sockfd, struct sockaddr* sending_addr, char buffer[MAXMESG]);
	static void WriteToFile(std::ofstream writeFile, char *dataBuffer);
	static void CreateDataPacket(FILE *pFile, char fileBuffer[MAXMESG], int& fileStartIterator);
	static void PrintPacket(char buffer[MAXMESG]);
	static std::string PacketToString(char buffer[MAXMESG]);
	static unsigned short GetPacketOPCode(char buffer[MAXMESG]);
	static unsigned short GetBlockNumber(char buffer[MAXMESG]);
	static std::string ConvertUnsignedShortToString(unsigned short number);
	static std::string GetFileNameStr(char buffer[MAXMESG]);
	static void GetFileNameCharPointer(char* filename, char buffer[MAXMESG]);
	static std::string GetMode(char buffer[MAXMESG], std::string fileName);
	static void GetDataContent(char buffer[MAXMESG], char (& dataArray)[MAXDATA]);
	static int GetNumberOfRequeiredPackets(std::string filename);
	static bool CheckIfLastDataPacket(char buffer[MAXMESG]);
	static void CreateAckPacket(char buffer[MAXMESG], unsigned short blockNumber);
private:
	//static int SendMessageHelper(int sockfd, struct sockaddr* receiving_addr, char* fileName);
	static void ReceivePacketHelper(int sockfd, struct sockaddr* sending_addr, char mesg[MAXMESG]);
	static char** GetFileData(char* fileName);

};
#endif //CSS432KEVINGABRIEL_TFTP_H
