/*
CLIENT CODE
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <iostream>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_PORT "27015"
#define ipaddrconnect "localhost"

using namespace std;

int sendData(SOCKET s, const char* data, int len, int& actualbytessent)
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
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    int iResult;


    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(ipaddrconnect, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    //send the data  to the client
    const char* sendata = "this is the msg from client";
    int bytessent = 0;
    int sendres = sendData(ConnectSocket, sendata, strlen(sendata), bytessent);
    if (bytessent <= 0)
    {
        cout << "error occured while sending . error : " << WSAGetLastError() << endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }


    //recv the data from the server
    char* data = nullptr;
    int datalen = 0;
    int recvres = recvData(ConnectSocket, data, datalen);
    if (datalen > 0)
    {
        string strdata = convertToString(data, datalen);
        cout << "len of data recvd : " << datalen << endl << "data recvd : \n" << strdata << endl;;
        //need to free the memory on which data was recvd
        delete[] data;
    }

    closesocket(ConnectSocket);
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


TCP HANDSHAKE : 
The start state of every socket is the CLOSED state. When a client initiates a connection, it sends a SYN
packet to the server and puts the client socket in the SYN_SENT state. When the server receives the SYN
packet, it sends a SYN-ACK packet, which the client responds to with an ACK packet. At this point, the client's
socket is in the ESTABLISHED state. If the server never sends a SYN-ACK packet, the client times out and
reverts to the CLOSED state.
When a server's socket is bound and is listening on a local interface and port, the state of the socket is
LISTEN. When a client attempts a connection, the server receives a SYN packet and responds with a
SYN-ACK packet. The state of the server's socket changes to SYN_RCVD. Finally, the client sends an ACK
packet, which causes the state of the server's socket to change to ESTABLISHED

*/