#define CORSAIR_LIGHTING_SDK_DISABLE_DEPRECATION_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <WinSock2.h>
#include <ACstructs.h>
#include <thread> 

#include <CUESDK.h>
#include <CUESDKGlobal.h>
#include <CorsairKeyIdEnum.h>
#include <CorsairLedIdEnum.h>


/*
Prints the number of connected Corsair devices & loops over them.
Determines if each device is a keyboard or not and prints the device model name.
*/
void GetConnectedCorsairDevices() {
    int devCount = CorsairGetDeviceCount();
    printf("Device count: %d\n", devCount);
    for (int i = 0; i < devCount; i++) {
        CorsairDeviceInfo* devInfo = CorsairGetDeviceInfo(i);
        printf("%d\n", i);
        if (devInfo->type == CDT_Keyboard) {
            printf("It's a keyboard!\n");
        }
        else {

            printf("It's not a keyboard :(\n");
        }
        printf("\t%s\n", devInfo->model);
    }
    return;
}

/*
Credit: Corsair demo code
Simple little toString method for errors involving the Corsair SDK
*/
const char* toStringError(CorsairError error)
{
    switch (error) {
    case CE_Success:
        return "CE_Success";
    case CE_ServerNotFound:
        return "CE_ServerNotFound";
    case CE_NoControl:
        return "CE_NoControl";
    case CE_ProtocolHandshakeMissing:
        return "CE_ProtocolHandshakeMissing";
    case CE_IncompatibleProtocol:
        return "CE_IncompatibleProtocol";
    case CE_InvalidArguments:
        return "CE_InvalidArguments";
    default:
        return "unknown error";
    }
}

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
int sendFirst(SOCKET socket, struct sockaddr_in address, int slen) {
    struct handshaker hs;
    struct handshakerResponse hsr;

    hs.identifier = 0;
    hs.operationId = 0;
    hs.version = 1;
    memset(&hsr, 0, sizeof(hsr));

    int timeOutCount = 1;
    int noResponse = 1;

    while (timeOutCount < 6 && noResponse) {
        int send1 = sendto(socket, (char*)&hs, sizeof(handshaker), 0, (struct sockaddr*) & address, slen);
        printf("Sent: %d\n", send1);
        if (recvfrom(socket, (char*)&hsr, sizeof(struct handshakerResponse), 0, (struct sockaddr*) & address, &slen) == SOCKET_ERROR) {
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
            if (sendFirst(socket, address, slen)) {
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
While(true) loop that:
1. Calculates which key to light up & stores calue in rpm
2. 'Zeros' out the LEDs for each number on the keyboard
3. Illuminates the LED calculated in step one
4. Sleeps for 50ms
TODO: Switch statement for gears & numpad
*/
void ledLoop(struct RTCarInfo *info) {
    int redValue[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int greenValue[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int blueValue[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    CorsairLedId ledId[10] = { CLK_1, CLK_2, CLK_3, CLK_4, CLK_5, CLK_6, CLK_7, CLK_8, CLK_9, CLK_0 };
    CorsairLedId numpadId[10] = {CLK_Keypad0, CLK_Keypad1, CLK_Keypad2, CLK_Keypad3, CLK_Keypad4, CLK_Keypad5, CLK_Keypad6, CLK_Keypad7, CLK_Keypad8, CLK_Keypad9};
    struct CorsairLedColor ledCol;
    ledCol = CorsairLedColor{CLK_1, 0, 0, 0};
    
    while (true) {
        int rpm = (((int)roundf(info->engineRPM / 1000)) -1) % 10;
        printf("%d, %f\n", rpm, info->engineRPM);
       
        for (int i = 0; i < sizeof(ledId) / sizeof(ledId[0]); i++) {
            ledCol.ledId = ledId[i];
            if (i <= rpm) {
                ledCol.r = 255;
            } 
            else {
                ledCol.r = 0;
            }
            ledCol.g = 0;
            ledCol.b = 0;
            CorsairSetLedsColors(1, &ledCol);
        }

        int gear = (info->gear - 1) % 10;
        for (int i = 0; i < sizeof(numpadId) / sizeof(numpadId[0]); i++) {
            ledCol.ledId = numpadId[i];
            if (i == gear) {
                ledCol.r = 255;
            }
            else {
                ledCol.r = 0;
            }
            ledCol.g = 0;
            ledCol.b = 0;
            CorsairSetLedsColors(1, &ledCol);
        }
        CorsairSetLedsColorsFlushBuffer();

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
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

    printf("Initializing\n");
    CorsairPerformProtocolHandshake();
    if (const auto error = CorsairGetLastError()) {
        printf("%s error occurred when attempting to load Corsair utilities :(\n", toStringError(error));
        return -1;
    }
    GetConnectedCorsairDevices();
    

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAstartup failed with error %d :( \n", WSAGetLastError());
        return -1;
    }
    
    printf("Enter IP (default 127.0.0.1): ");
    scanf_s("%s", &ip, sizeof(ip));
    printf("Enter port (default 9996): ");
    scanf_s("%d", &port);
   
    SOCKET sock = initSock();
    struct sockaddr_in x = initAddr(ip, port);
    int slen = sizeof(x);
    
    if (sendFirst(sock, x, slen) != 1) {
        printf("Could not reach host :(\n");
        return -1;
    }

    std::thread netThread(enterMainNetLoop, sock, x, slen, &info);
    std::thread ledThread(ledLoop, &info);
    netThread.join();
    printf("Net thread join called\n");
    ledThread.join();
    printf("LED thread join called\n");


    //hs.operationId = 3;
    //sendto(sock, (char*)&hs, sizeof(handshaker), 0, (struct sockaddr*)(&x), sizeof(x));
    closesocket(sock);
    WSACleanup();
	return 0;
}