//
// Copyright (c) 2015, RF Digital Corp.
// All rights reserved.
 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//    1. Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//
//    2. Redistributions in binary form must on an RF Digital part and reproduce the 
//       above copyright notice, this list of conditions and the following disclaimer 
//       in the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
// WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//
// I2C is utilized to communicate with the LSM303 compass and OLED Display.
// The Wire library is used to manage the I2C protocol.
// SDA = IO 6 on RFduino
// SCL = IO 5 on RFduino
//
#include <Wire.h>
#include <RFduinoBLE.h>
#include "RFduino_Timer2.h"
#include <avr/pgmspace.h>
#include <OLEDDisplay.h>
#include <LSM303.h>

OLEDDisplay *oledDisplay;
LSM303 compass;

#define SSD1306_I2C_ADDRESS   0x3C	// 011110+SA0+RW - 0x3C or 0x3D
#define LOGO_X 100
#define LOGO_Y 0
#define DEVICE_NAME "RFduino Compass"
#define SERVICE_UUID "b329392a-fbcd-49aa-a823-3e87680ac33b"

// select a flash page that isn't in use (see Memory.h for more info)
#define  MY_FLASH_PAGE  251

// double level of indirection required to get gcc
// to apply the stringizing operator correctly
#define  str(x)   xstr(x)
#define  xstr(x)  #x

static const unsigned char PROGMEM Bth16 [] = {
0x00, 0x00, 0x01, 0xC0, 0x01, 0xE0, 0x01, 0xB0, 0x19, 0x98, 0x0D, 0x98, 0x07, 0xB0, 0x03, 0xE0,
0x01, 0xC0, 0x03, 0xE0, 0x07, 0xB0, 0x0D, 0x98, 0x09, 0x98, 0x11, 0xB0, 0x01, 0xE0, 0x01, 0xC0
};

// Connection will be true when in a BLE connection and false otherwise
bool connection = false;

// Advertising will be true when the BLE radio is advertising and false otherwise
bool advertising = false;

// Set to true in the interupt handler such that the next time through the main loop
// the compass will be read and the display updated.
volatile bool readCompass = false;
volatile bool updateDisplay = false;

struct stored_data_t
{
  float min_x;
  float min_y;
  float min_z;
  float max_x;
  float max_y;
  float max_z;
};

void myTimerCallback(void *user_data);

void setup() 
{
  // put your setup code here, to run once:
  
  Serial.begin(9600);
  Serial.println("Setup starting");

  Serial.print("Flash Used: ");
  Serial.println(flashUsed());
  
  //
  // Both display and compass are on the I2C bus.
  //
  Wire.begin();
  
  //
  // Create the display device and display the logo
  // in the library memory.
  //
  oledDisplay = new OLEDDisplay(SSD1306_I2C_ADDRESS);
  oledDisplay->on();
  oledDisplay->update();
  
  /* Initialise the compass */
  compass.init();
  compass.enableDefault();
  compass.setTimeout(300);

  // a flash page is 1K in length, so page 251 starts at address 251 * 1024 = 257024 = 3EC00 hex  
  Serial.println("reading flash");
  stored_data_t *storedData = (stored_data_t *)ADDRESS_OF_PAGE(MY_FLASH_PAGE);
    
  Serial.print("Read Min x:");
  Serial.print(storedData->min_x);
  Serial.print(" y:");
  Serial.print(storedData->min_y);
  Serial.print(" z:");
  Serial.print(storedData->min_z);
  Serial.print(" Max x:");
  Serial.print(storedData->max_x);
  Serial.print(" y:");
  Serial.print(storedData->max_y);
  Serial.print(" z:");
  Serial.println(storedData->max_z);
  
  // Calibration values determined from the calibrate method above.
  if(!isnan(storedData->min_x)) 
  {
    compass.m_min.x = storedData->min_x;
  }
  if(!isnan(storedData->min_y)) 
  {
    compass.m_min.y = storedData->min_y;
  }
  if(!isnan(storedData->min_z)) 
  {
    compass.m_min.z = storedData->min_z;
  }
  
  if(!isnan(storedData->max_x)) 
  {
    compass.m_max.x = storedData->max_x;
  }
  if(!isnan(storedData->max_y)) 
  {
    compass.m_max.y = storedData->max_y;
  }
  if(!isnan(storedData->max_z)) 
  {
    compass.m_max.z = storedData->max_z;
  }
     
  //
  // Setup the timer for flashing the Bluetooth icon
  // and reading the compass
  //
  timer2->setOnTimerExpires(myTimerCallback, NULL);
  timer2->prescaler(8);
  timer2->interval(50);
  timer2->start();
  
  //
  // Start the Bluetooth radio
  //
  // 128 bit base uuid
  // (generated with http://www.uuidgenerator.net)
  RFduinoBLE.deviceName = DEVICE_NAME;
  RFduinoBLE.customUUID = SERVICE_UUID;
  RFduinoBLE.begin();
  RFduinoBLE.sendInt(0);
  
  delay(5000);
  oledDisplay->clear();
  oledDisplay->update();
  oledDisplay->setTextSize(6);
  
  Serial.println("Setup done");
}

void loop() 
{
  /* Used to time update function ~125 ms
  unsigned long starttime = millis();
  for(int i = 0; i < 100; i++)
  {
    oledDisplay->update();
  }
  Serial.println(millis() - starttime);
  */
  
  if(readCompass)
  {
    compass.read();
    float heading = compass.heading((LSM303::vector){0,-1,0});
    //
    // Since LSM303 is upside down on the on the breadboard, the
    // compass reading is 180 degrees off. Could update the library,
    // for now do it here.
    //
    heading += 180;
    if(360 <= heading) heading -= 360;
    
    drawHeading(heading + 0.5);
    if(connection)
    {
      RFduinoBLE.sendInt(heading + 0.5);
    }
    readCompass = false;
  }
  
  if(updateDisplay)
  {
    oledDisplay->update();
    updateDisplay = false;
  }
}

void drawHeading(int heading)
{
  static int y = 18;
  
  oledDisplay->fillRect(0, y, SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT - 18, BLACK);
  if(heading < 10)
  {
    oledDisplay->setLocation(52, y);
  } else if (heading < 100)
  {
    oledDisplay->setLocation(28, y);
  } else {
    oledDisplay->setLocation(16, y);
  }
  oledDisplay->print(heading);
  Serial.print("Heading: ");
  Serial.println(heading);
  
  updateDisplay = true;
}

void myTimerCallback(void *user_data)
{
  static volatile bool logoDisplayed = false;
  static int compassCount = 0;
  
  if(!connection)
  {
    if(advertising)
    {
      if(logoDisplayed)
      {
        oledDisplay->drawBitmap(LOGO_X, LOGO_Y, Bth16, 16, 16, WHITE, BLACK);
      } else {
        oledDisplay->drawBitmap(LOGO_X, LOGO_Y, Bth16, 16, 16, BLACK, BLACK);
      }
      logoDisplayed = !logoDisplayed;
    }
    updateDisplay = true;
  } else {
    oledDisplay->drawBitmap(LOGO_X, LOGO_Y, Bth16, 16, 16, WHITE, BLACK);
  }
  
  compassCount += 1;
  if(20 < compassCount)
  {
    readCompass = true;
    compassCount = 0;
  }
}

void RFduinoBLE_onAdvertisement(bool start)
{
  // turn the green led on if we start advertisement, and turn it
  // off if we stop advertisement
  advertising = start;
}

void RFduinoBLE_onConnect()
{
  connection = true;
  oledDisplay->drawBitmap(LOGO_X, LOGO_Y, Bth16, 16, 16, WHITE, BLACK);
  oledDisplay->update();
}

void RFduinoBLE_onDisconnect()
{
  connection = false;
  oledDisplay->drawBitmap(LOGO_X, LOGO_Y, Bth16, 16, 16, BLACK, BLACK);
  oledDisplay->update();
}

void RFduinoBLE_onReceive(char *data, int len)
{
  // if the first byte is 0xAA Calibrate the compass
  Serial.print("onReceive: length: ");
  Serial.print(len);
  if(len == 0) 
  {
    Serial.println();
    return;
  }
  Serial.print(" value: ");
  Serial.print(data[0], HEX);
  Serial.println(": done");
  
  if (data[0] == 0xAA)
  {
    RFduinoBLE.end();
    calibrate();
    RFduinoBLE.begin();
    oledDisplay->clear();
    oledDisplay->update(); 
  }
}

void clearDisplay(bool drawBug)
{
  oledDisplay->clear();
  oledDisplay->update(); 
  
  if(drawBug)
  {
  }
}

void calibrate() 
{ 
  LSM303::vector running_min = {2047, 2047, 2047}, running_max = {-2048, -2048, -2048};

  clearDisplay(false);
  oledDisplay->setTextSize(2);

  oledDisplay->setLocation(10,16);
  oledDisplay->print("Calibrate");
  oledDisplay->setLocation(0,32);
  oledDisplay->print("rotate in circles");
  oledDisplay->update();

  running_min.x = 0;
  running_min.y = 0;
  running_min.z = 0;
  
  running_max.x = 0;
  running_max.y = 0;
  running_max.z = 0;

  oledDisplay->setTextSize(6);

  int countdown = 10;
  for(int i = 0; i < 260; i++)
  { 
    compass.read();
  
    running_min.x = min(running_min.x, compass.m.x);
    running_min.y = min(running_min.y, compass.m.y);
    running_min.z = min(running_min.z, compass.m.z);

    running_max.x = max(running_max.x, compass.m.x);
    running_max.y = max(running_max.y, compass.m.y);
    running_max.z = max(running_max.z, compass.m.z);
  
    if(i % 20 == 0 && i > 59)
    {
      if(countdown == 10)
      {
        oledDisplay->clear();
        oledDisplay->update();  
      }
      drawHeading(countdown--);
      oledDisplay->update();
    }
    delay(50);
  }
  
    Serial.print("M min ");
  Serial.print("X: ");
  Serial.print((int)running_min.x);
  Serial.print(" Y: ");
  Serial.print((int)running_min.y);
  Serial.print(" Z: ");
  Serial.print((int)running_min.z);

  Serial.print(" M max ");  
  Serial.print("X: ");
  Serial.print((int)running_max.x);
  Serial.print(" Y: ");
  Serial.print((int)running_max.y);
  Serial.print(" Z: ");
  Serial.println((int)running_max.z);

  Serial.print("M min ");
  Serial.print("X: ");
  Serial.print((int)compass.m_min.x);
  Serial.print(" Y: ");
  Serial.print((int)compass.m_min.y);
  Serial.print(" Z: ");
  Serial.print((int)compass.m_min.z);

  Serial.print(" M max ");  
  Serial.print("X: ");
  Serial.print((int)compass.m_max.x);
  Serial.print(" Y: ");
  Serial.print((int)compass.m_max.y);
  Serial.print(" Z: ");
  Serial.println((int)compass.m_max.z);

  compass.m_min.x = running_min.x;
  compass.m_min.y = running_min.y;
  compass.m_min.z = running_min.z;
  compass.m_max.x = running_max.x;
  compass.m_max.y = running_max.y;
  compass.m_max.z = running_max.z;
  
  stored_data_t updates;
  
  stored_data_t *storedData = (stored_data_t *)ADDRESS_OF_PAGE(MY_FLASH_PAGE);

  Serial.print("Attempting to erase flash page " str(MY_FLASH_PAGE) ": ");
  int rc = flashPageErase(MY_FLASH_PAGE);
  if (rc == 0)
    Serial.println("Success");
  else if (rc == 1)
    Serial.println("Error - the flash page is reserved");
  else if (rc == 2)
    Serial.println("Error - the flash page is used by the sketch");

  updates.min_x = compass.m_min.x;
  updates.min_y = compass.m_min.y;
  updates.min_z = compass.m_min.z;
  updates.max_x = compass.m_max.x;
  updates.max_y = compass.m_max.y;
  updates.max_z = compass.m_max.z;
  
  /*
  updates.min_x = 6;
  updates.min_y = 6;
  updates.min_z = 6;
  updates.max_x = 6;
  updates.max_y = 6;
  updates.max_z = 6;
  */
  
  rc = flashWriteBlock(storedData, &updates, sizeof(updates));
  if (rc == 0)
    Serial.println("Success");
  else if (rc == 1)
    Serial.println("Error - the flash page is reserved");
  else if (rc == 2)
    Serial.println("Error - the flash page is used by the sketch");     
}

