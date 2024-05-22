#include <stdio.h>

#ifndef SENSOR_H
#define SENSOR_H

void initADC();

uint16_t readADC(uint8_t channel);

#endif