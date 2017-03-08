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
****************************************************************************************/

/*
*****************************************************************************************
* INCLUDE FILES
*****************************************************************************************
*/
#include <SPI.h>
#include "AES-128_V10.h"
#include "Encrypt_V31.h"
#include "LoRaWAN_V31.h"
#include "RFM95_V21.h"
#include "LoRaMAC_V11.h"
#include "Waitloop_V11.h"

/*
*****************************************************************************************
* GLOBAL VARIABLES
*****************************************************************************************
*/

// This key is for thethingsnetwork (ABP device) - register your device on: https://staging.thethingsnetwork.org/applications
// Registreren en eigen keys invullen
//unsigned char NwkSkey[16] = { 0x00, 0x96, 0x00, 0x2F, 0x0F, 0x0A, 0xAA, 0x1F, 0xCB, 0x33, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 };
//unsigned char AppSkey[16] = { 0x00, 0x3F, 0x00, 0x12, 0xD9, 0xE7, 0x35, 0x0C, 0xE1, 0xC8, 0xD2, 0x00, 0x00, 0x00, 0x00, 0x00 };
//unsigned char DevAddr[4] = { 0x00, 0x19, 0x00, 0x00 };

int FC = 0;
// SF to set in LoRaWAN)V31.h (default SF9 is set) - quick and dirty implemented (TBC)

void setup() 
{
  //Initialize the UART
  Serial.begin(115200);
  Serial.println("---");
  Serial.println("What: TTN Apeldoorn - LoRa node bouwavond / 'hello world'");
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

  while(1)
  {
    // Here the sensor information should be retrieved 
   
    //Construct data 
    sprintf(msg,"Hello world!");
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
  }//While(1)
}
