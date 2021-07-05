/*
SERVER CODE
*/

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")

#define DEFAULT_PORT "27015"

using namespace std;


int sendData(SOCKET s, const char* data ,  int len, int& actualbytessent)
{
    int total = 0; // how many bytes we've sent till now
    int bytesleft = len; // how many we have left to send
    int n = 0;  //actual bytes sent after send() call
    while (total < len) {
        n = send(s, data + total, bytesleft, 0);
        if (n == -1) {
            /* print/log error details */
            break;
        }
        total += n;
        bytesleft -= n;
    }
    actualbytessent = total; // return number actually sent here . if after sending some bytes ,send throws error , then this tells how much bytes we actually able to sent
    return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

int recvData(SOCKET s, char*& data, int& datalen)
{
    const int BUFF_SIZE = 512;
    datalen = 0;
    int totalbytesrecv = 0;
    int recvbytes = 0;
   // char recvdata[BUFF_SIZE];
    char* recvdata = new char[BUFF_SIZE];
    data = recvdata;
    while (1)
    {
        recvbytes = recv(s, recvdata, BUFF_SIZE, 0);
        if (recvbytes < 0)
        {
            cout << "error occured while receiving .  error : " << WSAGetLastError() << endl;
            return -1;
        }
        else if (recvbytes == 0)
        {
            //no more to be received
            return 0;
        }
        datalen += recvbytes;
        recvdata += recvbytes;

        if (datalen >= BUFF_SIZE)
        {
            cout << "data recvd exceeded the max buffer size\n";
            return 0;
        }
    }

    return 0;
}

string convertToString(char* a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

int  main()
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;


    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    cout<< "listening on server started on port : 27015" << endl;
    while (1)
    {
        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        //recv the data from the client
        char* data = nullptr;;
        int datalen = 0;
        int recvres = recvData(ClientSocket, data, datalen);
        if (datalen > 0)
        {
            string strdata = convertToString(data, datalen);
            cout << "len of data recvd : " << datalen << endl << "data recvd : \n" << strdata << endl;;
            //need to free the memory on which data was recvd
            delete[] data;
        }

        //send the data back to the client
        const char* sendata = "this is the msg from server";
        int bytessent = 0;
        int sendres = sendData(ClientSocket, sendata, strlen(sendata), bytessent);
        if (bytessent <= 0)
        {
            cout << "error occured while sending . error : " << WSAGetLastError() << endl;
        }

        // shutdown the connection since we're done
        iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        // cleanup
        closesocket(ClientSocket);
        cout << "socket closed\n";

        // Receive until the peer shuts down the connection
        //do {

        //    iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        //    if (iResult > 0) {
        //        recvbuf[iResult] = '\0';
        //        printf("Bytes received: %d and recv buffer : %s\n", iResult,recvbuf);

        //        // Echo the buffer back to the sender

        //       /* iSendResult = send(ClientSocket, recvbuf, iResult, 0);*/
        //        iSendResult = send(ClientSocket, sampleresstr, 200, 0);
        //        
        //        if (iSendResult == SOCKET_ERROR) {
        //            printf("send failed with error: %d\n", WSAGetLastError());
        //            closesocket(ClientSocket);
        //            WSACleanup();
        //            return 1;
        //        }
        //        printf("Bytes sent: %d\n", iSendResult);
        //    }
        //    else if (iResult == 0)
        //        printf("Connection closing...\n");
        //    else {
        //        printf("recv failed with error: %d\n", WSAGetLastError());
        //        closesocket(ClientSocket);
        //        WSACleanup();
        //        return 1;
        //    }

        //} while (iResult > 0);

        //// shutdown the connection since we're done
        //iResult = shutdown(ClientSocket, SD_SEND);
        //if (iResult == SOCKET_ERROR) {
        //    printf("shutdown failed with error: %d\n", WSAGetLastError());
        //    closesocket(ClientSocket);
        //    WSACleanup();
        //    return 1;
        //}

        //// cleanup
        //closesocket(ClientSocket);
        //cout << "socket closed\n";
    }
   
    WSACleanup();

    return 0;
}



/*
//notes
Transmission Control Protocol/Internet Protocol (TCP/IP) and Internetwork Packet
Exchange (IPX).

WSA added to the api means winsock2 api

few exceptions to this naming rule. WSAStartup, WSACleanup, WSARecvEx,
and WSAGetLastError are in the Winsock 1.1 specification

When compiling your application with WINSOCK2.H, you should link with
WS2_32.LIB library

WSAGetLastError()   -> check for error

ipv4 and ipv6
In IPv4, computers are assigned an address that is represented as a 32-bit quantity.

in Intel x86 processors,
multibyte numbers are represented in little-endian form: the bytes are ordered from
least significant to most significant.
big-endian form (most significant byte to least significant), normally referred to as
network-byte order.


*/