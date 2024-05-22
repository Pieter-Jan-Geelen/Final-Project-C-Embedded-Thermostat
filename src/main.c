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

/*

Thermostat struct, has the possibility to scale horizontally ( multiple rooms )

*/
struct Thermostat
{
  char roomName[MAX_ROOM_NAME_LENGTH];
  double minTemp;
  double maxTemp;
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

@param increase This parameter determines whether or not we want to increase or decrease min / max
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

int * formatNumberToDisplay(int num) // example 200
{
  int firstDidget = num / 100; // 200 / 100 = 2
  int secondDidget = ( int ) ( num / 10 ) % 10; // 0
  int decimalDidget = ( int ) ( num%10 ); // 0

  int *numberArr = malloc(3 * sizeof(int));

  numberArr[0] = firstDidget;
  numberArr[1] = secondDidget;
  numberArr[2] = decimalDidget;

  return numberArr;
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
        if (currentRoom+1 >= roomCounter)
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
  initThermostateSystem();

  enableAllButtons();
  enableAllButtonInterrupts();

  enableAllLeds();

  sei();

  // End of initialisation

  createNewRoom(180, 210);
  createNewRoom(160, 240);

  int sensor;
  //int * numbers;

  while (1)
  {

    sensor = 15;

    for (int i = 0; i < roomCounter; i++)
    {
      if (rooms[i]->minTemp > sensor && getLedStatus(i) == 0)
      {
        turnLedOn(i);
        continue;
      }
      turnDownLed(i);
    }

    // Room selector
    if (currentScreen == 0)
    {
      writeCharToSegment(0, 'r');
      writeNumberToSegment(1, currentRoom+1);
    }

    if (currentScreen == 1)
    {
      if (selectedTab == 0)
      {
        writeString("min");
      }

      if (selectedTab == 1)
      {
        writeString("max");
      }

      if (selectedTab == 2)
      {
        writeString("back");
      }
    }

    if (currentScreen == 2)
    {
      if (selectedTab == 0)
      {
        //numbers = formatNumberToDisplay(rooms[currentRoom]->minTemp);
        
        writeNumber((rooms[currentRoom]->minTemp / 100), (( int ) ( rooms[currentRoom]->minTemp / 10 ) % 10), (( int ) rooms[currentRoom]->minTemp%10 ));
      }

      if (selectedTab == 1)
      {
        //numbers = formatNumberToDisplay(rooms[currentRoom]->maxTemp);
        
        writeNumber((rooms[currentRoom]->maxTemp / 100), (( int ) ( rooms[currentRoom]->maxTemp / 10 ) % 10), (( int ) rooms[currentRoom]->maxTemp%10 ));
      }
    }

  }

  return 0;
}