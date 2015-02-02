
#include <SPI.h>
#include <Mirf.h>
#include <Wire.h>
#include <MirfHardwareSpiDriver.h>


void blockForSend();
void SendToBase(char nameClient[6], char nameBase[6] , String theMessage);
void SendMessage(char nameClient[6], char nameBase[6] , String theMessage);


const byte Payload = 32;


void SetupMirfClient(char nameClient[6], char nameBase[6] ) {
	
	
  Mirf.spi = &MirfHardwareSpi;
#ifdef MIRF_PINS_PCB
  Mirf.cePin=8;
  Mirf.csnPin=9;
#endif
  Mirf.init();

  // name the receiving channel - must match tranmitter setting!
  Mirf.setRADDR((byte *)nameClient);

  Mirf.setTADDR((byte *)nameBase);


  Mirf.payload=Payload;

  Mirf.channel = 10;


  // configure 15 retries, 500us between attempts
  Mirf.configRegister(SETUP_RETR,(B0001<<ARD ) | (B1111<<ARC));

  // Set 1MHz data rate - this increases the range slightly
  Mirf.configRegister(RF_SETUP,0x06);

  // now config the device.... 
  Mirf.config();  
	
	
}




void SendToBase(char nameClient[6], char nameBase[6] , String theMessage) {

//  Mirf.powerUpTx();
  
//  delay(1);
  
  char thePayload[Payload];
  char theMessageChar[Payload];

  Mirf.setTADDR((byte *)nameBase);
  Mirf.setRADDR((byte *)nameClient);
  Mirf.config();

  strcpy(thePayload,nameClient);
  theMessage.toCharArray(theMessageChar,Payload);
  strcat(thePayload,theMessageChar);

  Mirf.send((byte *)thePayload);
  blockForSend();

  // something changes the RADDR to nameBase... auto ack? let's change it back

  Mirf.setRADDR((byte *)nameClient);
  Mirf.config();
  Mirf.powerUpRx();
  
}




void SendMessage(char nameClient[6], char nameBase[6] , String theMessage) {

//  Mirf.powerUpTx();

//  delay(1);
  
  char theTarget[6];
  char thePayload[Payload];
  char theSource[6];

  theMessage.substring(0,5).toCharArray(theTarget,6);
  theMessage.substring(5).toCharArray(thePayload,Payload);
  theMessage.substring(5,5).toCharArray(theSource,6);


  Mirf.setTADDR((byte *)theTarget);
  Mirf.setRADDR((byte *)nameClient);

  //    Mirf.setRADDR((byte *)theSource);
  // do we want this? malformed message could make this node unreachable 

  Mirf.config();
  Mirf.send((byte *)thePayload);
  blockForSend();

  // something changes the RADDR to nameBase... auto ack? let's change it back

  Mirf.setRADDR((byte *)nameClient);
  Mirf.config();
  Mirf.powerUpRx();
  
}

void blockForSend() {

  while( Mirf.isSending() )
  {
    delay(1);
  }
}



