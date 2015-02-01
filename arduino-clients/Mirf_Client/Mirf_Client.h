
#include <SPI.h>
#include <Mirf.h>
#include <Wire.h>
#include <MirfHardwareSpiDriver.h>
#include <WString.h>

//void Mirf_Client_Setup();

//void SendToBase(String theMessage);
//void SendMessage(String theMessage);
//void blockForSend();

extern void SendToBase(String theMessage);
extern void SendMessage(String theMessage);
extern void blockForSend();

#ifndef nameBase
#endif
