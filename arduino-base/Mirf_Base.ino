#include <SPI.h>
#include <Mirf.h>

// #include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>
#include <EEPROM.h>

char nameBase[6] = "BASES";
char nameClient[6] = "BASES";

#include "MirfClient.h"

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

long msUpdateInterval=500;
long msLastUpdate=0;





void setup()
{
  Serial.begin(115200);
  inputString.reserve(100);

  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();

  // name the receiving channel - must match tranmitter setting!
  Mirf.setRADDR((byte *)"BASES");

  Mirf.payload=32; 
  
  Mirf.channel = 10;
  // configure 15 retries, 250us between attempts
  Mirf.configRegister(SETUP_RETR,(B0000<<ARD ) | (B1111<<ARC));



  // now config the device.... 
  Mirf.config();  

  // Set 1MHz data rate - this increases the range slightly
  Mirf.configRegister(RF_SETUP,0x06);
  
//  Mirf.configRegister(RF_SETUP, (B0000<<RF_DR_HIGH) | (B

  Mirf.setTADDR((byte *)"WCLKK");

  Mirf.config();
  Mirf.send((byte *)"BASES0Base station");
  blockForSend();
  Mirf.config();
  delay(100);
  Mirf.send((byte *)"BASES1initialized");
  blockForSend();
  delay(1000);
  Serial.print("BASES BASES Booted ");
  Serial.println(Mirf.getStatus());
  delay(1000);
}

void loop()
{ 
    
  if (millis() - msLastUpdate > msUpdateInterval ) {
    msLastUpdate=millis();
    Serial.println("BASES BASES UPDATE");
    Serial.flush();
    
  }
  

  byte data[Mirf.payload];

  // is there any data pending? 
  if( Mirf.dataReady() )
  {
    Mirf.getData((byte *) &data);

    // ... and write it out to the PC
    char theFrom[6];
    char theTo[6]="BASES";
    char theMessage[Mirf.payload];
    String thePayload=String((char *)data);
    //thePayload=*data;
    //    thePayload=strdup(data);


    thePayload.substring(0,5).toCharArray(theFrom,6);
    thePayload.substring(5).toCharArray(theMessage,Mirf.payload);

    Serial.print(theTo);
    Serial.print(" ");
    Serial.print(theFrom);
    Serial.print(" ");
    Serial.println(theMessage);
    Serial.flush();
    msLastUpdate=millis();
  }
  
  
  while (Serial.available() > 0) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
      break;
    } else {
      inputString += inChar;
    }
    
  }
  
  if (stringComplete==true) {
    
    if (inputString.length()>9){ 
      char theTarget[6];
      char theMessage[Mirf.payload];
      inputString.substring(0,5).toCharArray(theTarget,6);
      inputString.substring(5).toCharArray(theMessage,Mirf.payload);
      Mirf.setTADDR((byte *)theTarget);
      Mirf.setRADDR((byte *)"BASES");
      Mirf.config();
      Mirf.powerUpTx();
      Mirf.send((byte *)theMessage);
      blockForSend();
      Mirf.powerUpRx();
      
      Serial.print("BASES BASES Message_sent_to ");
      Serial.println(theTarget);
      Serial.flush();
      msLastUpdate=millis();
    }

    stringComplete=false;
    inputString="";

  }
  

    blockForSend();

       
}
 
 
