
#include <SPI.h>
#include <Mirf.h>
#include <Wire.h>
#include <MirfHardwareSpiDriver.h>
#include <WString.h>

void SendToBase(String theMessage) {
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

}




void SendMessage(String theMessage) {
  char theTarget[6];
  char thePayload[Payload];
  char theSource[6];
  theMessage.substring(0,5).toCharArray(theTarget,6);
  theMessage.substring(5).toCharArray(thePayload,Payload);
  //   Serial.println(inputString);
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

}

void blockForSend() {

  while( Mirf.isSending() )
  {
    delay(1);
  }
}
