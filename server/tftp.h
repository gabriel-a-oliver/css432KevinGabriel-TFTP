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

	// Parameters: char *progname, int sockfd, struct sockaddr_in receiving_addr, int clilen, char buffer[MAXMESG], std::string fileName
	//
	// Post: Stores all the data of a file with the provided std::string fileName, and sends them all to the sockaddr_in receiving_addr.
	//		 A linear send and wait for ACK process.
	static void SendFile(char *progname, int sockfd, struct sockaddr_in pcli_addr, int clilen, char buffer[MAXMESG], /*char fileBuffer[MAXMESG],*/ std::string fileName);

	// Parameters: char *progname, int sockfd, struct sockaddr_in sending_addr, char ackBuffer[MAXMESG], std::string fileNameString
	//
	// Post: Receives all the data of a file with the provided std::string fileNameString.
	//		 A linear wait to receive and send corresponding ACK process.
	static void ReceiveFile(char *progname, int sockfd, struct sockaddr_in sending_addr, char ackBuffer[MAXMESG], std::string fileNameString);

	// Parameters: char buffer[MAXMESG]
	//
	// Post: Print entire packet to console
	static void PrintPacket(char buffer[MAXMESG]);

	// Parameters: char buffer[MAXMESG]
	//
	// Post: Return any packet as a std::string
	static std::string PacketToString(char buffer[MAXMESG]);

	// Parameters: char buffer[MAXMESG]
	//
	// Post: Return packet's OP code as an unsigned short
	static unsigned short GetPacketOPCode(char buffer[MAXMESG]);

	// Parameters: char buffer[MAXMESG]
	//
	// Post: Return packet's block number as an unsigned short
	static unsigned short GetBlockNumber(char buffer[MAXMESG]);

	// Parameters: unsigned short number
	//
	// Post: Convert an unsigned short and return it as an std::string
	// help from: https://stackoverflow.com/questions/14871388/convert-from-unsigned-short-to-string-c/14871427
	static std::string ConvertUnsignedShortToString(unsigned short number);

	// Parameters: char buffer[MAXMESG]
	//
	// Post: Return packet's file name as an std::string
	static std::string GetFileNameStr(char buffer[MAXMESG]);

	// Parameters: char buffer[MAXMESG], std::string fileName
	//
	// Post: Return the mode of the RRQ/WRQ packet as an std::string
	static std::string GetMode(char buffer[MAXMESG], std::string fileName);

	// Parameters: char buffer[MAXMESG], char (& dataArray)[MAXDATA]
	//
	// Post: Store the data from buffer into dataArray
	// pass char[] by reference help from: https://stackoverflow.com/questions/12987760/passing-char-array-by-reference
	static void GetDataContent(char buffer[MAXMESG], char (& dataArray)[MAXDATA]);

	// Parameters: std::string filename
	//
	// Post: Return number of needed packets to send all the data from the file as an int
	static int GetNumberOfRequeiredPackets(std::string filename);

	// Parameters: char buffer[MAXMESG]
	//
	// Post: Return bool if the passed packet is the last packet
	static bool CheckIfLastDataPacket(char buffer[MAXMESG]);

	// Parameters: char buffer[MAXMESG], unsigned short blockNumber
	//
	// Post: Create an ACK packet and store it in buffer
	static void CreateAckPacket(char buffer[MAXMESG], unsigned short blockNumber);

	// Parameters: char *progname, int sockfd, struct sockaddr_in pcli_addr
	//
	// Post: A helper function to send an error letting the sockaddr_in pcli_addr know that a file already exists
	static void SendFileAlreadyExistsError(char *progname, int sockfd, struct sockaddr_in pcli_addr);

	// Parameters: char* progname, int sockfd, char buffer[MAXMESG], struct sockaddr_in receiving_addr
	//
	// Post: Public version of tftp::SendPacketHelper().
	//		 Send packet buffer to the target sockaddr_in receiving_addr
	static void SendPacket(char* progname, int sockfd, char buffer[MAXMESG], struct sockaddr_in receiving_addr);
private:

	// Parameters: char* progname, int sockfd, char buffer[MAXMESG], struct sockaddr_in receiving_addr
	//
	// Post: Send packet buffer to the target sockaddr_in receiving_addr
	static void SendPacketHelper(char* progname, int sockfd, char buffer[MAXMESG], struct sockaddr_in receiving_addr);

	// Parameters: unsigned short* buffPtr, unsigned short errorCode, std::string errorMessage
	//
	// Post: Creates Error packet and stores in buffPtr
	static void CreateErrorPacketHelper(unsigned short* buffPtr, unsigned short errorCode, std::string errorMessage);
};
#endif //CSS432KEVINGABRIEL_TFTP_H
