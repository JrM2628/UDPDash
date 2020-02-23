#include "UDPDashCorsair.h"

/*
Initiates Corsair's protocol handshake and returns true if no errors occur (false otherwise)
*/
int initCorsair() {
    CorsairPerformProtocolHandshake();
    if (const auto error = CorsairGetLastError()) {
        printf("%s error occurred when attempting to load Corsair utilities :(\n", toStringErrorCorsair(error));
        return 0;
    }
    return 1;
}

/*
Prints the number of connected Corsair devices & loops over them.
Determines if each device is a keyboard or not and prints the device model name.
*/
void GetConnectedDevicesCorsair() {
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
const char* toStringErrorCorsair(CorsairError error)
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
While(true) loop that:
1. Calculates which key to light up & stores calue in rpm
2. Zeros out the LEDs for each number on the keyboard & sets others to be illuminated
3. Zeros out the numpad except for the number that correlates to the current gear
4. Sleeps for 50ms
*/
void ledLoopCorsair(struct RTCarInfo* info) {
    int redValue[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int greenValue[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int blueValue[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    CorsairLedId ledId[10] = { CLK_1, CLK_2, CLK_3, CLK_4, CLK_5, CLK_6, CLK_7, CLK_8, CLK_9, CLK_0 };
    CorsairLedId numpadId[10] = { CLK_Keypad0, CLK_Keypad1, CLK_Keypad2, CLK_Keypad3, CLK_Keypad4, CLK_Keypad5, CLK_Keypad6, CLK_Keypad7, CLK_Keypad8, CLK_Keypad9 };
    struct CorsairLedColor ledCol;
    ledCol = CorsairLedColor{ CLK_1, 0, 0, 0 };

    while (true) {
        int rpm = (((int)roundf(info->engineRPM / 1000)) - 1) % 10;
        //printf("%d, %f\n", rpm, info->engineRPM);

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
