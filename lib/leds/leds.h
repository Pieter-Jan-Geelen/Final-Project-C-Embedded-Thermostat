#ifndef LEDS_H
#define LEDS_H

// enable leds
void enableLed(int led);
void enableAllLeds();

// turn on leds
void turnLedOn(int led);
void turnOnAllLeds();

// turn down leds
void turnDownLed(int led);
void turnDownAllLeds();

// checks status of led
int getLedStatus(int led);
int getAllLedsStatus();

#endif