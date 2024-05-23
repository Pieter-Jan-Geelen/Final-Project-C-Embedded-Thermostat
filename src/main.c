// Default C libraries
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h>

// Interrupts
#include <avr/interrupt.h>

// Custom added libraries
#include <usart.h>
#include <buttons.h>
#include <display.h>
#include <leds.h>
#include <sensor.h>

// Finals
#define MAX_ROOM_NAME_LENGTH 3
#define MAX_NUMBER_OF_ROOMS 2

#define MAX_ROOM_TEMPERATURE 400

#define DEBUG_TIMEOUT 500

#define TEMP_SENSOR PC4

volatile uint32_t counter = 0;
volatile uint16_t overflow_count = 0;

// Array of pointers to the structs
struct Thermostat **rooms;

// helper variable to the **rooms
int roomCounter = 0;

// which of the different screens is displayed [ 0: room selector; 1: Specific room; 2: detail ]
int currentScreen = 0;

// Determines which room is displayed on the homescreen
int currentRoom = 0;

// Choose the tab [ 0: MIN; 1: MAX; 2: BACK ]
int selectedTab = 0;

// set this to 1 to display the current temperature measured by the sensor
int showCurrentTemp = 0;

// The temperature from the sensor
int sensor;

/*

Thermostat struct, has the possibility to scale horizontally ( multiple rooms )

*/
struct Thermostat
{
  char roomName[MAX_ROOM_NAME_LENGTH];
  int minTemp;
  int maxTemp;
};

/*

Aesthetic function
Mallocs memory for the 4 rooms that can be initialised

*/
void initThermostateSystem()
{
  printString("\nStarting thermostat, please wait patient ...\n");
  _delay_ms(DEBUG_TIMEOUT);
  printString("Create room environment...\n");

  rooms = (struct Thermostat **) malloc(MAX_NUMBER_OF_ROOMS * sizeof(struct Thermostat *));

  // Makes sure that the malloc was succesfull;
  if (rooms == NULL)
  {
    printString("Memory allocation failed for rooms\n");
    return;
  }

  _delay_ms(DEBUG_TIMEOUT);
  printString("Starting succesfully ...\n");
}

/*

This function is going to create a new struct of Thermostat and will push a pointer to the new struct in the **rooms array

@param min The minimum temperature the room can to be
@param max The maximum temperature the room can be
@return void

*/
void createNewRoom(int min, int max)
{ 
  if (roomCounter >= MAX_NUMBER_OF_ROOMS)
  {
    printString("Max number of rooms reached\n");
    return;
  }

  struct Thermostat *pt = (struct Thermostat *) malloc(sizeof(struct Thermostat));

  if (pt == NULL)
  {
    printString("Memory allocation failed for new room\n");
    return;
  }

  char name[MAX_ROOM_NAME_LENGTH];
  sprintf(name, "r%d", roomCounter + 1);

  snprintf(pt -> roomName, MAX_ROOM_NAME_LENGTH, "%s", name);

  pt -> minTemp = min;
  pt -> maxTemp = max;

  rooms[roomCounter] = pt;
  roomCounter ++;
}

/*

Helper function for adjusting the min and max parameters of the existing rooms

@param increase This parameter determines whether or not we want to increase or decrease min / max (bool)
@return void

*/
void adjustCurrentRoomTemperature(int increase)
{
  // Min temperature
  if (selectedTab == 0)
  {
    if (increase)
    {
      // Min and Max cannot be equal to eachother
      if (rooms[currentRoom]->minTemp + 1 >= rooms[currentRoom]->maxTemp)
        return;

      rooms[currentRoom]->minTemp++;
      return;
    }

    // Decrease current room min temperature
    if (rooms[currentRoom]->minTemp - 1 < 0)
      return; 
      
    rooms[currentRoom]->minTemp--;
    return;
  }

  // Max temperature
  if (selectedTab == 1)
  {
    if (increase)
    {
      // Set the ceiling to the final
      if (rooms[currentRoom]->maxTemp + 1 > MAX_ROOM_TEMPERATURE)
        return;

      rooms[currentRoom]->maxTemp++;
      return;
    }

    // Decrease current room max temperature
    if (rooms[currentRoom]->maxTemp-1 == rooms[currentRoom]->minTemp || rooms[currentRoom]->maxTemp-1 == 0)
      return;

    rooms[currentRoom]->maxTemp--;
  }
}

/*

  This is a helper function to break down a number into three ints so we can display it
  Don't forget to free the memory when the numbers are no longer needed!

  @param num Number that you want to break down
  @return pointer returns a pointer to the array where the three ints are stored

*/
int* formatNumberToDisplay(int num) {
    int firstDigit = num / 100; // 263 / 100 = 2
    int secondDigit = (num / 10) % 10; // (263 / 10) % 10 = 6
    int decimalDigit = num % 10; // 263 % 10 = 3

    int *pArr = malloc(3 * sizeof(int));
    if (pArr == NULL) {
        return NULL;
    }

    pArr[0] = firstDigit;
    pArr[1] = secondDigit;
    pArr[2] = decimalDigit;

    return pArr;
}

/*

  handles the timer for the leds

*/
void initTimer0() {
    // Set the timer to Normal mode (no PWM, just counting)
    TCCR0A = 0; // WGM00 = 0 and WGM01 = 0 --> Normal mode

    // Set prescaler to 1024
    TCCR0B |= _BV(CS02) | _BV(CS00); // CS02 = 1 and CS00 = 1 --> prescaler factor is now 1024

    // Enable overflow interrupt
    TIMSK0 |= _BV(TOIE0); // overflow interrupt enable
}

ISR(TIMER0_OVF_vect) {
    overflow_count++;
    if (overflow_count >= 61) // 15625 / 256 ≈ 61 
    {
      //printString("Interrupt every second\n");
      counter++;
      overflow_count = 0;

      sensor = (readADC(4) * 4.22) / 10;

      for (int i = 0; i < roomCounter; i++)
      {
        if (sensor * 10 < rooms[i]->minTemp)
        {
          turnLedOn(i);
          continue;
        }
        turnDownLed(i);
      }
    }
}

/*

Handle the interrupts coming from the buttons

*/
ISR(PCINT1_vect)
{
  // Left button
  if (buttonPushed(0))
  {
    // Debounce
    _delay_us(1000);

    if (buttonPushed(0))
    {
      // Room selector
      if (currentScreen == 0)
      {
        if (currentRoom == 0)
          return;

        if (showCurrentTemp)
        {
          showCurrentTemp = 0;
          return;
        }

        currentRoom--;
      }

      // Room details
      if (currentScreen == 1)
      {
        if (selectedTab == 0)
          return;

        selectedTab--;
      }

      // Change min / max temperature
      if (currentScreen == 2)
        adjustCurrentRoomTemperature(0);

    }
    return;
  }

  // Center button
  if (buttonPushed(1))
  {
    // Debounce
    _delay_us(1000);

    if (buttonPushed(1))
    {
      // Back button on details screen
      if (currentScreen == 2)
      {
        currentScreen = 1;
        selectedTab = 0;
        return;
      }

      // Select button on specific rooms screen
      if (currentScreen == 1)
      {
        // Min/Max temperatuur of room 
        if (selectedTab == 0 || selectedTab == 1)
        {
          currentScreen = 2;
          return;
        }

        // Back button
        if (selectedTab == 2)
        {
          currentScreen = 0;
          selectedTab = 0;
          return;
        }
      }

      // Room selector
      if (currentScreen == 0)
      {
        // room details
        currentScreen = 1;
        return;
      }
    }
    return;
  }

  // Right button
  if (buttonPushed(2))
  {
    // Debounce
    _delay_us(1000);

    if (buttonPushed(2))
    {
      // Room selector
      if (currentScreen == 0)
      {
        if (currentRoom+1 >= roomCounter && !showCurrentTemp)
        {
          showCurrentTemp = 1;
          return;
        }

        if (showCurrentTemp)
          return;
        
        currentRoom++;
        return;
      }

      // Room details
      if (currentScreen == 1)
      {
        if (selectedTab == 2)
          return;

        selectedTab++;
        return;
      }

      if (currentScreen == 2)
        adjustCurrentRoomTemperature(1);
    }
    return;
  }
}

int main()
{
  // debounce
  _delay_ms(DEBUG_TIMEOUT);

  // Initialisation process, do not change this or the application will break

  initUSART(); 
  initDisplay();
  initADC();
  initTimer0();
  initThermostateSystem();

  enableAllButtons();
  enableAllButtonInterrupts();

  enableAllLeds();

  sei();

  // End of initialisation

  createNewRoom(180, 210);
  createNewRoom(160, 240);

  int * numbers;

  while (1)
  {
    // Room selector
    if (currentScreen == 0 && !showCurrentTemp)
    {
      writeCharToSegment(0, 'r');
      writeNumberToSegment(1, currentRoom+1);
      continue;
    }

    if (currentScreen == 0 && showCurrentTemp)
    {
      numbers = formatNumberToDisplay(sensor * 10);

      writeNumber(numbers[0], numbers[1], numbers[2]);
      free(numbers);
      continue;
    }

    if (currentScreen == 1)
    {
      if (selectedTab == 0)
      {
        writeString("min");
        continue;
      }

      if (selectedTab == 1)
      {
        writeString("max");
        continue;
      }

      if (selectedTab == 2)
      {
        writeString("back");
        continue;
      }
    }

    if (currentScreen == 2)
    {
      if (selectedTab == 0)
      {
        numbers = formatNumberToDisplay(rooms[currentRoom]->minTemp);
        
        writeNumber(numbers[0], numbers[1], numbers[2]);
        free(numbers);
        continue;
      }

      if (selectedTab == 1)
      {
        numbers = formatNumberToDisplay(rooms[currentRoom]->maxTemp);
        
        writeNumber(numbers[0], numbers[1], numbers[2]);
        free(numbers);
        continue;
      }
    }

  }

  return 0;
}