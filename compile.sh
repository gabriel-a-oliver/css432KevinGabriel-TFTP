#!/bin/sh

g++ tftpserver.cpp -pthread -o tftpserver
g++ tftpclient.cpp -o tftpclient
g++ tftp.cpp
chmod 744 server client
