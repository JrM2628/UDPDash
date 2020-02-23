#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <ACstructs.h>
#include <thread> 
#include "UDPDashCorsair.h"
#include "UDPDashLogi.h"

/*
Returns an IP socket address struct given an IP and port
*/
struct sockaddr_in initAddr(char* ip, int port) {
    struct sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    #pragma warning(suppress : 4996)
    sockAddr.sin_addr.S_un.S_addr = inet_addr(ip);
    sockAddr.sin_port = htons(port);
    return sockAddr;
}

/*
Returns a UDP socket object with a timeout of 5 seconds
*/
SOCKET initSock() {
    SOCKET sock = INVALID_SOCKET;
    int iFamily = AF_INET;
    int iType = SOCK_DGRAM;
    int iProtocol = IPPROTO_UDP;
    int timeout = 5000;

    struct sockaddr_in sockAddr;
    int slen = sizeof(sockAddr);

    sock = socket(iFamily, iType, iProtocol);
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    return sock;
}

/*
1. Sends initial handshake datagram to host running AC
2. Attempts to recv back from host. If the host responds, returns 1. If an error occurs (including timeout), retries up to 5 times and returns 0.
*/
int sendFirst(SOCKET socket, struct sockaddr_in address, int slen, int retryAttempts) {
    struct handshaker hs;
    struct handshakerResponse hsr;

    hs.identifier = 0;
    hs.operationId = 0;
    hs.version = 1;
    memset(&hsr, 0, sizeof(hsr));

    int timeOutCount = 1;
    int noResponse = 1;

    while (timeOutCount < retryAttempts + 1 && noResponse) {
        int send1 = sendto(socket, (char*)&hs, sizeof(handshaker), 0, (struct sockaddr*) &address, slen);
        printf("Sent: %d\n", send1);
        if (recvfrom(socket, (char*)&hsr, sizeof(struct handshakerResponse), 0, (struct sockaddr*) &address, &slen) == SOCKET_ERROR) {
            printf("Timeout #%d\n", timeOutCount);
            timeOutCount++;
        }
        else {
            noResponse = 0;
        }
    }

    if (noResponse == 0) {
        printf("Recieved: %s\n", hsr.driverName);
        return 1;
    }
    return 0;
}

/*
Called after handshake is complete.
1. Sends another handshake (OP ID = 1) to host in order to begin the flood of data
2. Enters while(true) loop in order to constantly recv data from the host and saves that to the RTCarInfo struct info
3. If an error occurs while reveiving data, calls sendFirst which attempts to reestablish the handshake
*/
int enterMainNetLoop(SOCKET socket, struct sockaddr_in address, int slen, struct RTCarInfo *info) {
    struct handshaker hs;

    hs.operationId = 1;
    hs.identifier = 0;
    hs.version = 1;
    sendto(socket, (char*) &hs, sizeof(handshaker), 0, (struct sockaddr*) &address, slen);

    while (true)
    {
        if (recvfrom(socket, (char*) info, sizeof(struct RTCarInfo), 0, (struct sockaddr*) &address, &slen) == SOCKET_ERROR) {
            printf("An error has occurred! Attempting to reconnect\n");
            if (sendFirst(socket, address, slen, 1000)) {
                sendto(socket, (char*) &hs, sizeof(handshaker), 0, (struct sockaddr*) &address, slen);
            }
            else {
                return -1;
            }
        }
    }
    return 1;
}

/*
===============================================================================
MAIN 
1. Corsair Protocol Handshake + printing connected device info for debug purposes
2. WSAStartup
3. Prompt user for IP/Port
4.initSock & initAddr called
5. Spawns mainLoop thread (Networking recieve loop)
6. Spawns LED thread (LED tachometer loop)
7. Joins threads and closes socket
TODO: Handle networking renegotiation so I don't have to restart the program every time a race ends
TODO: Handle socket, thread, and Corsair cleanup
===============================================================================
*/

int main() {
    WSADATA wsaData;
    char ip[16] = "127.0.0.1";
    int port = ACSERVERPORT;
    struct RTCarInfo info;
    memset(&info, 0, sizeof(struct RTCarInfo));
    int corsairEnabled = 0;
    int logiEnabled = 0;

    printf("Initializing\n");

    if (corsairEnabled = initCorsair()){
        printf("CUE Detected!\n");
        GetConnectedDevicesCorsair();
    }
    if (logiEnabled = initLogi()) {
        printf("Logitech detected!\n");
    }
    else
    {
        printf("CUE nor Logitech detected!\n");
        return -1;
    }    

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAstartup failed with error %d :( \n", WSAGetLastError());
        return -2;
    }
    
    printf("Enter IP (default 127.0.0.1): ");
    scanf_s("%s", &ip, sizeof(ip));
    printf("Enter port (default 9996): ");
    scanf_s("%d", &port);
   
    SOCKET sock = initSock();
    struct sockaddr_in x = initAddr(ip, port);
    int slen = sizeof(x);
    
    if (sendFirst(sock, x, slen, 5) != 1) {
        printf("Could not reach host :(\n");
        return -3;
    }


    std::thread netThread, ledThreadCorsair, ledThreadLogi;

    netThread = std::thread(enterMainNetLoop, sock, x, slen, &info);

    if (corsairEnabled) {
        ledThreadCorsair = std::thread(ledLoopCorsair, &info);
    }
    if (logiEnabled) {
        ledThreadLogi = std::thread(ledLoopLogi, &info);
    }

    netThread.join();
    printf("Net thread join called\n");
    
    if (corsairEnabled) {
        ledThreadCorsair.join();
    }
    if (logiEnabled) {
        ledThreadLogi.join();
    }
    printf("LED thread join called\n");


    //hs.operationId = 3;
    //sendto(sock, (char*)&hs, sizeof(handshaker), 0, (struct sockaddr*)(&x), sizeof(x));
    closesocket(sock);
    WSACleanup();
	return 0;
}