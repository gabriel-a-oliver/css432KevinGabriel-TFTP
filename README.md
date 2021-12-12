# css432KevinGabriel
CSS432 Term Project Made By Kevin Huang (huangk12) and Gabriel Oliver (gaog1999)

Directory Tree:
/css432KevinGabriel
    |--makefile
    |--/client
        |--tftpclient.cpp
        |--tftp.cpp
        |--tftp.h
    |--/server
        |--tftpserver.cpp
        |--tftp.cpp
        |--tftp.h

Instructions:
1. Linux lab machines: Linux lab machine #11 is used for the server. Log into
lab machine #11 and additional lab machines as necessary for the client programs.

2. Compiling the programs: Go to the css432KevinGabriel directory and run the
makefile with "./makefile" command. This will compile both of the server and
client programs.

3. Start the server: Go to the server directory and run the server program with
"./tftpserver 519709" command. The program takes 1 argument - port number,
519709 in our case.

4. Start the client: Go to the client directory and run the client program. The
client program takes 3 arguments - operation, file name, and port number. The 
operation will be in the format of either be "-r" or "-w" and file name will be
"filename.txt". An example command is "./tftpclient -r test.txt 519709".

Test cases:
1. Read small/large file: Have server running and "./tftpclient -r test.txt 519709"
command on the client.

2. Write small/large file: Have server running and "./tftpclient -w test.txt 519709"
command on client.

3. Timeout from not receiving DATA packet: While doing a read request, terminate
the server program or terminate the client program on a write request.

4. Timeout from not receiving ACK packet: While doing a write request, terminate
the server program or terminate the client program on a read request.

5. File does not exist error: Read request on a file that is not on the server with
"./tftpclient -r DoesNotExist.txt 519709" command.

6. File already exists (overwrite warning): Write request for a file that already
exists on the server with "./tftpclient -w AlreadyExists.txt 519709" command.

7. No permission to access the requested file: Read request on a file with no access
permission with "./tftpclient -r NoAccess.txt 519709" command.

8. Multiple simultaneous clients: Have multiple clients send requests at the same
time.
