/* Cor      22-4-2017 - Uitgangspunt voor de TTN Enschede bouwavond
/* Jeroen / 5-12-2016 - kale Ideetron versie genomen met kleine aanpassingen voor TTN Apeldoorn bouwavond 
/******************************************************************************************
* Copyright 2015, 2016 Ideetron B.V.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/
/****************************************************************************************
* File:     LoRaWAN_V31.ino
* Author:   Gerben den Hartog
* Compagny: Ideetron B.V.
* Website:  http://www.ideetron.nl/LoRa
* E-mail:   info@ideetron.nl
****************************************************************************************/
/****************************************************************************************
* Created on:         04-02-2016
* Supported Hardware: ID150119-02 Nexus board with RFM95
* 
* Description
* 
* Minimal Uplink for LoRaWAN
* 
* This code demonstrates a LoRaWAN connection on a Nexus board. This code sends a messege every minute
* on chanell 0 (868.1 MHz) Spreading factor 7.
* On every message the frame counter is raised
* 
* This code does not include
* Receiving packets and handeling
* Channel switching
* MAC control messages
* Over the Air joining* 
*
* Firmware version: 1.0
* First version
* 
* Firmware version 2.0
* Working with own AES routine
* 
* Firmware version 3.0
* Listening to receive slot 2 SF9 125 KHz Bw
* Created seperate file for LoRaWAN functions
* 
* Firmware version 3.1
* Removed a bug from the Waitloop file
* Now using AppSkey in Encrypt payload function (Thanks to Willem4ever)
* Switching to standby at start of send package function
*
* Firmware version 3.2
* Merged I2C_ReadAllData.ino from 
* https://github.com/sparkfun/SparkFun_BME280_Arduino_Library
* for interfacing with the BMP280
****************************************************************************************/

/*
*****************************************************************************************
* INCLUDE FILES
*****************************************************************************************
*/
#include "Wire.h"
#include <SPI.h>
#include "AES-128_V10.h"
#include "Encrypt_V31.h"
#include "LoRaWAN_V31.h"
#include "RFM95_V21.h"
#include "LoRaMAC_V11.h"
#include "Waitloop_V11.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

/*
*****************************************************************************************
* GLOBAL VARIABLES
*****************************************************************************************
*/

// This key is for thethingsnetwork (ABP device) - register your device on: https://staging.thethingsnetwork.org/applications
// Registreren en eigen keys invullen
// Address en sleutels voor staging.thethingsnetwork.org
//unsigned char NwkSkey[16] = { 0x9D, 0x69, 0x3B, 0x9B, 0xB5, 0x73, 0x63, 0xD4, 0x42, 0x4B, 0x35, 0x96, 0xE6, 0x48, 0x22, 0x32 };
//unsigned char AppSkey[16] = { 0x30, 0x2F, 0x26, 0x59, 0x56, 0x30, 0x4B, 0x74, 0x78, 0x0B, 0x3A, 0x05, 0x80, 0xDF, 0xC3, 0xEE };
//unsigned char DevAddr[4] = {  0x92, 0xF0, 0x2B, 0xE4 };

// Address en sleutels voor console.thethingsnetwork.org
unsigned char NwkSkey[16] = { 0x06, 0x16, 0x67, 0x62, 0x10, 0x0B, 0xD7, 0xDD, 0x8A, 0xC9, 0xA3, 0x9A, 0xF6, 0x05, 0xDA, 0x57 };
unsigned char AppSkey[16] = { 0x9C, 0x82, 0x5A, 0x25, 0x06, 0xAA, 0xBD, 0xDA, 0xC5, 0x20, 0xA4, 0x93, 0x49, 0x5B, 0x21, 0x28 };
unsigned char DevAddr[4] = { 0x26, 0x01, 0x11, 0x35 };

// Knippen en plakken van 
//    https://console.thethingsnetwork.org/applications/cb_sensor/devices/sensor1

// const char *devAddr = "26011135";
// const char *nwkSKey = "06166762100BD7DD8AC9A39AF605DA57";
// const char *appSKey = "9C825A2506AABDDAC520A493495B2128";

int FC = 0;
// SF to set in LoRaWAN)V31.h (default SF9 is set) - quick and dirty implemented (TBC)

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11 
#define BMP_CS 10

Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

void setup() 
{
  //Initialize the UART
  Serial.begin(115200);
  Serial.println("---");
  Serial.println("What: TTN Enschede - LoRa node bouwavond / 'hello world'");
  Serial.println("Setup: Initialized serial port");

   //Initialise the SPI port
  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000,MSBFIRST,SPI_MODE0));
  Serial.println("Setup: SPI initialized");
  
  //Initialize I/O pins
  pinMode(DIO0,INPUT);
  pinMode(DIO1,INPUT); 
  pinMode(DIO5,INPUT);
  pinMode(DIO2,INPUT);
  pinMode(CS,OUTPUT);
  
  digitalWrite(CS,HIGH);
  
  WaitLoop_Init();

  //Wait until RFM module is started
  WaitLoop(20);   
  Serial.println("Setup: Completed");
  Serial.println("---");

  if (!bmp.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }


  Serial.print("Program Started\n");
    
  Serial.println();
}

void loop() 
{
  unsigned char Test = 0x00;

  unsigned char Sleep_Sec = 0x00;
  unsigned char Sleep_Time = 0x01;

  unsigned char Data_Tx[256];
  unsigned char Data_Rx[64];
  unsigned char Data_Length_Tx;
  unsigned char Data_Length_Rx = 0x00;
  char msg[64];

  //Initialize RFM module
  Serial.println("Loop: Initializing RFM95");
  RFM_Init();
  delay(1000);

    // Here the sensor information should be retrieved 
    Serial.print(F("Temperature = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");
    
    Serial.print(F("Pressure = "));
    Serial.print(bmp.readPressure() / 100.0);
    Serial.println(" hPa");
  
  Serial.println();
   
    //Construct data 
    sprintf(msg,"Hello world!");
    sprintf(msg,"%d.%02d C %ld.%02ld hPa", (int)bmp.readTemperature(), (int)(bmp.readTemperature()*100)%100, (long)bmp.readPressure()/100, (long)bmp.readPressure()%100);
    memcpy(Data_Tx, msg, strlen(msg));
    Data_Length_Tx = strlen(msg);

    Serial.print("Loop: Message content:");
    Serial.println(msg);

    Serial.println("Loop: Sending data");
    Data_Length_Rx = LORA_Cycle(Data_Tx, Data_Rx, Data_Length_Tx);
    FC = FC + 1;
    
    //Delay of 1 minute 
    Serial.println("Loop: Start waiting loop (1 minutes)");
    delay(60000);
    Serial.println("---");
}
