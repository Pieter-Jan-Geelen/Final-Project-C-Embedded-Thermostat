#include <avr/io.h>

#define NUMBER_OF_BUTTONS 3

#define BUTTON_DDR DDRC
#define BUTTON_PORT PORTC
#define BUTTON_PIN PINC

#define BUTTON1 PC1
#define BUTTON2 PC2
#define BUTTON3 PC3

void enableAllButtons()
{
    for (int i = 0; i < NUMBER_OF_BUTTONS; i++)
    {
        BUTTON_DDR &= ~(0 << (PC1 + i));
        BUTTON_PORT |= (1 << (PC1 + i));
    }
}

// enables both the DDR and PORT
void enableButton(int number)
{   
    if (number < 0 || number > NUMBER_OF_BUTTONS - 1) return;
    BUTTON_DDR &= ~(0 << (PC1 + number));
    BUTTON_PORT |= (1 << (PC1 + number));
}

// disables both the DDR and PORT
void disableButton(int number)
{
    if (number < 0 || number > NUMBER_OF_BUTTONS - 1) return;
    BUTTON_DDR |= (1 << (PC1 + number));
    BUTTON_PORT &= ~(0 << (PC1 + number));
}

// enables interrupts for all buttons
void enableAllButtonInterrupts()
{
    PCICR |= (1 << (PCIE1));
    PCMSK1 = 0b00001110;
}

// enables interrupts for a specific button
void enableButtonInterrupt(int number)
{
    if (number < 0 || number > NUMBER_OF_BUTTONS - 1) return;
    PCICR |= (1 << (PCIE1));
    PCMSK1 |= (1 << (PC1 + number));
}

// checks of a specific button is pushed
int buttonPushed(int number)
{
    if (number < 0 || number > NUMBER_OF_BUTTONS - 1) return 0;
    if (bit_is_clear(BUTTON_PIN, (PC1 + number)))
    {
        return 1;
    }
    return 0;
}

// checks of a specific button is released
int buttonReleased(int number)
{
    return !buttonPushed(number);
}