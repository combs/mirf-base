
#include <SPI.h>
#include <Mirf.h>
#include <Wire.h>
#include <MirfHardwareSpiDriver.h>
#include <EEPROM.h>


void blockForSend();
void SendToBase(String theMessage);
void SendMessage(String theMessage);


const byte Payload = 32;
const int addressEEPROM=900;



// Is this file yacking about NameClient and NameBase? 
// Include mirfscreenconfig.h or define them before including this


void SetupMirfClient() {
	
	
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


 


void SendToBase(String theMessage) {

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
 
  
}




void SendMessage(String theMessage) {

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


  
}

void getNameClient () {
	char nameDefault[6]="FRESH";
	for (int a=0;a<5;a++) {
		byte thisByte=EEPROM.read(900+a);
		if (thisByte<168 && thisByte>31) {
			nameClient[a]=thisByte;
		} else {
			strcpy(nameClient,nameDefault);
			break;
		}
	}	
}

void getNameBase() {
	char nameDefault[6]="BASES";
	for (int a=0;a<5;a++) {
		byte thisByte=EEPROM.read(905+a);
		if (thisByte<168 && thisByte>31) {
			nameBase[a]=thisByte;
		} else {
			strcpy(nameBase,nameDefault);
			break;
		}
	}
}
byte setNameClient(char nameNew[6]) {
	for (int a=0;a<5;a++) {
		if (nameNew[a]<168 && nameNew[a]>31) {
		} else {
			return false;
		}
	}	
	for (int a=0;a<5;a++) {
		EEPROM.write(900+a,nameNew[a]);
	}	
	strcpy(nameClient,nameNew);
	
	return true;
}
byte setNameBase(char nameNew[6]) {
	for (int a=0;a<5;a++) {
		if (nameNew[a]<168 && nameNew[a]>31) {
		} else {
			return false;
		}
	}	
	for (int a=0;a<5;a++) {
		EEPROM.write(900+a,nameNew[a]);
	}	
	strcpy(nameBase,nameNew);
	
	return true;
}

void blockForSend() {

  while( Mirf.isSending() )
  {
    delay(1);
  }
 
  // something changes the RADDR to nameBase... auto ack? let's change it back

  Mirf.setRADDR((byte *)nameClient);
  Mirf.config();
  Mirf.powerUpRx();
  
}

void HandleBuiltinMessage(char *thePayload) {
	char theCommand=thePayload[0];
	char theMessage[Payload];
	char theName[6];
	
	for (byte a=1;a<Payload;a++) {
        if (thePayload[a]=='\0' ) {
        	theMessage[a-1]='\0';
        	break;
        }
		theMessage[a-1]=thePayload[a];
     }
	switch (theCommand) {
		case 'B':
			strcpy(theName,theMessage);
			setNameBase(theName);
			SendToBase("Name-saved");
			break;
		case 'C':
			strcpy(theName,theMessage);
			setNameClient(theName);
			SendToBase("Name-saved");
			break;
		default:
			SendToBase("Unrecognized");
		break;
			
	}
	
	
}




