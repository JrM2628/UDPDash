#pragma once

#include <stdio.h>
#include <thread> 

#include "ACstructs.h"
#include <LogitechLEDLib.h>

int initLogi();
void ledLoopLogi(struct RTCarInfo* info);
