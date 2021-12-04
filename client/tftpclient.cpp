#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <typeinfo>
#include "tftp.cpp"


#define SERV_UDP_PORT 51709
#define SERV_HOST_ADDR "10.158.82.41" //REPLACE WITH SERVER IP ADDRESS; lab11: 10.158.82.41

char *progname;

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("%s: invalid number of arguments\n",progname);
		exit(1);
	}
	std::cout<< "correct number of command line arguments"<<std::endl;

	progname = argv[0];
	const char *op = argv[1];
	char *filename = argv[2];
	std::cout<< "assigned command line arguments to variables"<<std::endl;

	int sockfd;
	struct sockaddr_in cli_addr, serv_addr;

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERV_HOST_ADDR);
	serv_addr.sin_port = htons(SERV_UDP_PORT);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s: can't open datagram socket\n",progname);
		exit(2);
	} else {
		std::cout<< "socket established"<<std::endl;
	}

	bzero((char *) &cli_addr, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	cli_addr.sin_port = htons(0);

	if (bind(sockfd, (struct sockaddr *) &cli_addr, sizeof(cli_addr)) < 0) {
		printf("%s: can't bind local address\n",progname);
		exit(3);
	} else {
        std::cout<< "binded socket"<<std::endl;
    }

	char *bufpoint; // for building packet
	char buffer[MAXMESG]; // packet that will be sent
	bzero(buffer, sizeof(buffer));

	std::cout<< "seeing if OP is r or w" <<std::endl;
	if (op[1] == 'r') {
		std::cout<< "OP is r: " << RRQ <<std::endl;
		unsigned short opValue = RRQ;
		unsigned short* opCodePtr = (unsigned short *) buffer;
		*opCodePtr = htons(opValue);
	} else
	if (op[1] == 'w') {
		std::cout<< "OP is w: " << WRQ <<std::endl;
		unsigned short opValue = WRQ;
		unsigned short* opCodePtr = (unsigned short *) buffer;
		*opCodePtr = htons(opValue);
	} else {
		std::cout<< "neither r or w" <<std::endl;
	}

	/*std::cout<< "printing file name of length:" << strlen(filename)<<std::endl;
	for (int i = 0; i < strlen(filename); i++) {
		if (filename[i] == NULL) {
			std::cout << "~";
		}
		std::cout<< filename[i];
	}
	std::cout<<std::endl << "end of file name" << std::endl;*/

	std::cout<< "creating RRQ/WRQ request packet" <<std::endl;
	bufpoint = buffer + 2; // move pointer to file name
	strcpy(bufpoint, filename); // add file name to buffer
	bufpoint += strlen(filename) + 1; //move pointer and add null byte
	char modePointer[5] = {'o','c','t','e','t'};
	strcpy(bufpoint, modePointer); // add mode to buffer
	bufpoint += strlen(modePointer) + 1; // move pointer and add null byte

	/*std::cout<< "Printing fileName:" <<tftp::GetFileNameStr(buffer)<<std::endl;
	std::cout << "Printing mode:" << tftp::GetMode(buffer, tftp::GetFileNameStr(buffer))<<std::endl;*/

	tftp::PrintPacket(buffer);

	std::cout<< "sending packet" <<std::endl;
	int n = sendto(sockfd, buffer, bufpoint-buffer, 0, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (n < 0) {
		printf("%s: sendto error\n",progname);
		exit(4);
	} else {
		std::cout<< "no issue sending packet" <<std::endl;
	}

	std::cout<< "Saving file name:";
    // save file name before clearing buffer
    std::string fileNameString = tftp::GetFileNameStr(buffer);
	std::cout<< fileNameString <<std::endl;

    // clear out buffer for reuse
	bzero(buffer, sizeof(buffer));

	if (op[1] == 'r') {
		// if RRQ, call tftp shared receiving function
		tftp::ReceiveFile(progname, sockfd, serv_addr, fileNameString);
	} else if (op[1] == 'w') {
        std::cout<< "waiting for ACK0 from server" <<std::endl;
        tftp::ReceiveMessage(sockfd, (struct sockaddr *) &serv_addr, (struct sockaddr *) &cli_addr, buffer);

		// check if received packet is the ack
		tftp::PrintPacket(buffer);
		unsigned short ackOpNumb = tftp::GetPacketOPCode(buffer);
		if (ackOpNumb == ACK) {
			std::cout<< "ack received. transaction complete for block:"<< tftp::GetBlockNumber(buffer) <<std::endl;
		} else {
			std::cout<< "no ack received. received:"<<ackOpNumb<<std::endl;
		}

        /*char fileBuffer[MAXMESG];
		bzero(fileBuffer, MAXMESG);*/

        tftp::SendFile(progname, sockfd, serv_addr, sizeof(struct sockaddr_in), buffer, /*fileBuffer,*/ fileNameString);
		
	} else {
		std::cout<< "was not RRQ or WRQ" <<std::endl;
	}

	close(sockfd);

	return 0;
}























