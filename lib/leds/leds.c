#include <avr/io.h>

#define NUMBER_OF_LEDS 4

#define LED_DDR DDRB
#define LED_PORT PORTB

#define LED1 PB2
#define LED2 PB3
#define LED3 PB4
#define LED4 PB4

// enables specific led and enables the LED PORT so the led stays down
void enableLed(int led)
{
    if (led < 0 || led > NUMBER_OF_LEDS - 1) return;
    LED_DDR |= (1 << (PB2 + led));
    LED_PORT |= (1 << (PB2 + led));
}

// enables all leds and enables the LED PORT so the led stays down
void enableAllLeds()
{
    for (int i = 0; i < NUMBER_OF_LEDS; i++)
    {
        LED_DDR |= (1 << (PB2 + i));
        LED_PORT |= (1 << (PB2 + i));
    }
}

// turns on specific led
void turnLedOn(int led)
{
    if (led < 0 || led > NUMBER_OF_LEDS -1 ) return;
    LED_PORT &= ~(1 << (PB2 + led));
}

// turns on all leds
void turnOnAllLeds()
{
    for (int i = 0; i < NUMBER_OF_LEDS; i++)
    {
        LED_PORT &= ~(1 << (PB2 + i));
    }
}

// turns down specific led
void turnDownLed(int led)
{
    if (led < 0 || led > NUMBER_OF_LEDS - 1) return;
    LED_PORT |= (1 << (PB2 + led));
}

// turns down all leds
void turnDownAllLeds()
{
    for (int i = 0; i < NUMBER_OF_LEDS; i++)
    {
        LED_PORT |= (1 << (PB2 + i));
    }
}

// checks the status of a specific led
int getLedStatus(int led)
{
    if (bit_is_clear(LED_PORT, (PB2 + led)))
    {
        return 1;
    }
    return 0;
}

// returns 1 if one of the leds is turned on
int getAllLedsStatus()
{
    for (int i = 0; i < NUMBER_OF_LEDS; i++)
    {
        if (bit_is_clear(LED_PORT, (PB2 + i)))
        {
            return 1;
        }
        continue;
    }
    return 0;
}