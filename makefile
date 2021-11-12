#!/bin/sh

cd server
g++ tftpserver.cpp -o tftpserver
cd ..
cd client
g++ tftpclient.cpp -o tftpclient
