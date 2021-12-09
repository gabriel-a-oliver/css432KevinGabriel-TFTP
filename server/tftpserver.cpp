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
#include "tftp.cpp"

//#define SERV_UDP_PORT 51709
#define MAX_CLIENT_CONNECTIONS 11

char *progname;
int serv_udp_port;

int main(int argc, char *argv[]) {
	std::cout << "in main" << std::endl;
	if (argc != 2) {
		printf("%s: invalid number of arguments\n",progname);
		exit(1);
    }
    
    int sockfd;
	struct sockaddr_in serv_addr;

	progname=argv[0];
	serv_udp_port = std::stoi(argv[1]);

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("%s: can't open datagram socket\n",progname);
		exit(2); 
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(serv_udp_port);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
		printf("%s: can't bind local address\n",progname);
		exit(3);
	}

	int n, clilen;
    char buffer[MAXMESG];
    bzero(buffer, sizeof(buffer));
    struct sockaddr_in pcli_addr;

    for ( ; ; ) {
		std::cout << "am in loop" << std::endl;
        clilen = sizeof(struct sockaddr_in);

        n = recvfrom(sockfd, buffer, MAXMESG, 0, (struct sockaddr *) &pcli_addr, (socklen_t*)&clilen);
		if (n < 0) {
			printf("%s: recvfrom error\n",progname);
			exit(4);
		}

		std::cout << "received something" << std::endl;
		tftp::PrintPacket(buffer);

		unsigned short opNumber = tftp::GetPacketOPCode(buffer);

		std::cout << "checking if op is RRQ or WRQ" << std::endl;
        if (opNumber == RRQ) {
			std::cout<< "op is RRQ" <<std::endl;
			/*char fileBuffer[MAXMESG];
			bzero(fileBuffer, MAXMESG);*/

			std::string fileName = tftp::GetFileNameStr(buffer);

            //call tftp SendFile to send DATA
			tftp::SendFile(progname, sockfd, pcli_addr, clilen, buffer, /*fileBuffer,*/ fileName);

		} else if (opNumber == WRQ) {
			std::cout<< "op is WRQ"<<std::endl;

            // save file name before clearing buffer
            std::string fileNameString = tftp::GetFileNameStr(buffer);

            // clear out buffer for reuse
	        bzero(buffer, sizeof(buffer));

            std::ifstream infile(fileNameString);
            if (infile.good()){
		        std::cout<< "file exists, delete data before writing to it"<<std::endl;

                // create ERROR packet
                char errBuff[MAXMESG];
                bzero(errBuff, sizeof(errBuff));
                std::cout<< "creating ERROR packet" <<std::endl;
                unsigned short opValue = ERROR;
                unsigned short* buffPtr = (unsigned short *) errBuff;
                *buffPtr = htons(opValue);
                std::cout<< "OP is: " << opValue <<std::endl;

                buffPtr++;
                unsigned short errorCode = OVERWRITE;
                *buffPtr = htons(errorCode);
                std::cout<< "Error Code is : " << errorCode <<std::endl;

                buffPtr++;
                std::string errormessage = "File already exists.";
                strcpy((char*)buffPtr, errormessage.c_str());
    
                // Send the ERROR packet
                std::cout<< "sending ERROR packet" <<std::endl;
                int n = sendto(sockfd, errBuff, MAXMESG/*sizeof(fileBuffer)*/, 0, (struct sockaddr *) &pcli_addr, sizeof(pcli_addr));
                if (n < 0) {
                    printf("%s: sendto error\n",progname);
                    exit(4);
                } else {
                    std::cout<< "no issue sending packet" <<std::endl;
                }

                exit(99); // temporary, should just be sending back error instead of exiting
            }

            std::cout<< "creating ack0 packet" <<std::endl;
            char ackBuffer[MAXMESG];
			bzero(ackBuffer, sizeof(ackBuffer));
			char* ackBufPoint = ackBuffer + 2;

			unsigned short ackOpValue = ACK;
			unsigned short* ackOpCodePtr = (unsigned short *) ackBuffer;
			*ackOpCodePtr = htons(ackOpValue);

			unsigned short ackBlockValue = 0;
			unsigned short* ackBlockPtr = (unsigned short *) ackBuffer + 1;
			*ackBlockPtr = htons(ackBlockValue);

			tftp::PrintPacket(ackBuffer);

			std::cout<< "sending ack0 packet" <<std::endl;
			int n = sendto(sockfd, ackBuffer, MAXMESG, 0, (struct sockaddr *) &pcli_addr, sizeof(pcli_addr));
			if (n < 0) {
				printf("%s: sendto error\n",progname);
				exit(4);
			} else {
				std::cout<< "no issue sending packet" <<std::endl;
			}

            //call ReceiveFile and wait for DATA from client
            tftp::ReceiveFile(progname, sockfd, pcli_addr, ackBuffer, fileNameString);
        } else {
			std::cout<< "op was neither"<<std::endl;
		}
    }

    return 0;
}
