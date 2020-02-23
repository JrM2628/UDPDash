#include "UDPDashLogi.h"

int initLogi() {
	if (LogiLedInitWithName("UDPDash")) {
		return 1;
	}
	return 0;
}


int getConnectedDevicesLogi() {
	return 0;
}

/*
While(true) loop that:
1. Calculates which key to light up & stores calue in rpm
2. Zeros out the LEDs for each number on the keyboard & sets others to be illuminated
3. Zeros out the numpad except for the number that correlates to the current gear
4. Sleeps for 50ms
*/
void ledLoopLogi(struct RTCarInfo* info) {

    LogiLed::KeyName ledId[10] = { LogiLed::KeyName::ONE, LogiLed::KeyName::TWO, LogiLed::KeyName::THREE, LogiLed::KeyName::FOUR, LogiLed::KeyName::FIVE, LogiLed::KeyName::SIX, LogiLed::KeyName::SEVEN, LogiLed::KeyName::EIGHT, LogiLed::KeyName::NINE, LogiLed::KeyName::ZERO };
    LogiLed::KeyName numpadId[10] = { LogiLed::KeyName::NUM_ZERO, LogiLed::KeyName::NUM_ONE, LogiLed::KeyName::NUM_TWO, LogiLed::KeyName::NUM_THREE, LogiLed::KeyName::NUM_FOUR, LogiLed::KeyName::NUM_FIVE, LogiLed::KeyName::NUM_SIX, LogiLed::KeyName::NUM_SEVEN, LogiLed::KeyName::NUM_EIGHT, LogiLed::KeyName::NUM_NINE};
    //unsigned char bitmap [LOGI_LED_BITMAP_SIZE];

    while (true) {
        int rpm = (((int)roundf(info->engineRPM / 1000)) - 1) % 10;
        //printf("%d, %f\n", rpm, info->engineRPM);

        for (int i = 0; i < sizeof(ledId) / sizeof(ledId[0]); i++) {
            if (i <= rpm) {
                LogiLedSetLightingForKeyWithKeyName(ledId[i], 100, 0, 0);
            }
            else {
                LogiLedSetLightingForKeyWithKeyName(ledId[i], 0, 0, 0);
            }
        }

        int gear = (info->gear - 1) % 10;
        for (int i = 0; i < sizeof(numpadId) / sizeof(numpadId[0]); i++) {
            if (i == gear) {
                LogiLedSetLightingForKeyWithKeyName(numpadId[i], 100, 0, 0);
            }
            else {
                LogiLedSetLightingForKeyWithKeyName(numpadId[i], 0, 0, 0);
            }
        }

        printf("%f %f\n", info->gas, info->brake);
        int greenPercent = int (100 * info->gas);
        int redPercent = int(100 * info->brake);

        LogiLedSetLightingForTargetZone(LogiLed::Mouse, 0, 0, greenPercent, 0);
        LogiLedSetLightingForTargetZone(LogiLed::Mouse, 1, redPercent, 0, 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

