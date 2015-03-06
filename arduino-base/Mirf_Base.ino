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

const long msUpdateInterval=250;
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

  Mirf.channel=10;
  // configure 15 retries, 250us between attempts
  Mirf.configRegister(SETUP_RETR,(B0000<<ARD ) | (B1111<<ARC));



  // now config the device.... 
  Mirf.config();  

  // Set 1MHz data rate - this increases the range slightly
  Mirf.configRegister(RF_SETUP,0x06);

  //  Mirf.configRegister(RF_SETUP, (B0000<<RF_DR_HIGH) | (B
 
  SendMessage("WCLKKBASES0Base station");
  SendMessage("WCLKKBASES1initialized");

  Serial.print(nameClient);
  Serial.print(" ");
  Serial.print(nameBase);
  Serial.print(" Booted " );
  Serial.println(Mirf.getStatus());
  
  Mirf.powerUpRx();


}

void loop()
{ 

  if (millis() - msLastUpdate > msUpdateInterval ) {
    msLastUpdate=millis();

    Serial.print(nameClient);
    Serial.print(" ");
    Serial.print(nameBase);
    Serial.println(" UPDATE " );
    Serial.flush();

  }



  // is there any data pending? 
  if( Mirf.dataReady() )
  {
    byte data[Mirf.payload];
    Mirf.getData((byte *) &data);

    // ... and write it out to the PC
    char theFrom[6]; 
    char theMessage[Mirf.payload];
    String thePayload=String((char *)data);
    //thePayload=*data;
    //    thePayload=strdup(data);


    thePayload.substring(0,5).toCharArray(theFrom,6);
    thePayload.substring(5).toCharArray(theMessage,Mirf.payload);

    Serial.print(nameBase);
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
    
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
      break;
    } 
    else {
      inputString += inChar;
    }

  }

  if (stringComplete==true) {

    if (inputString.length()>9){ 

      SendMessage(inputString);

      Serial.print("BASES BASES Message_sent_to ");
      Serial.println(inputString.substring(0,5));
      Serial.flush();
      msLastUpdate=millis();
    }

    stringComplete=false;
    inputString="";

  }




}




