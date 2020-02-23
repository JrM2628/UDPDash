#pragma once

#define CORSAIR_LIGHTING_SDK_DISABLE_DEPRECATION_WARNINGS

#include <stdio.h>
#include <thread> 

#include "ACstructs.h"

#include <CUESDK.h>
#include <CUESDKGlobal.h>
#include <CorsairKeyIdEnum.h>
#include <CorsairLedIdEnum.h>

int initCorsair();
void GetConnectedDevicesCorsair();
const char* toStringErrorCorsair(CorsairError error);
void ledLoopCorsair(struct RTCarInfo* info);
