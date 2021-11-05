#!/bin/sh

g++ tftpserver.cpp -o tftpserver
g++ tftpclient.cpp -o tftpclient
chmod 744 tftpserver tftpclient