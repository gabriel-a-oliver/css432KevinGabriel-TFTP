#!/bin/sh

cd server
g++ tftp.cpp
g++ tftpserver.cpp -lpthread -o tftpserver
cd ..
cd client
g++ tftp.cpp
g++ tftpclient.cpp -o tftpclient