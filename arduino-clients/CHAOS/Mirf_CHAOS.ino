#include <SPI.h>
#include <Mirf.h>
#include <Wire.h>
#include <MirfHardwareSpiDriver.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>

void setup();

#include "mirfscreenconfig.h"


#define MIRF_PINS_STANDARD
// #define MIRF_PINS_PCB


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
volatile byte updateRequested = true;
volatile byte paintRequested = true;
volatile long msTurnedOn = 0;



// const long msScreenUpdateInterval=1000;

long msScreenUpdated=0;

volatile long msGotForecast = 0;
volatile long msRequestedForecast = 0;
const byte Payload = 32;
char stringForecast0[Payload];
char stringForecast1[Payload];
char stringForecastNow[Payload];
char stringForecastLater[Payload];
char stringForecastConditions[Payload];
// char stringBuffer[Payload];


#define PIN_METER_ONE 3
#define PIN_METER_TWO 5
#define PIN_METER_THREE 6


void setup()
{
//  Serial.begin(115200);
//  Serial.println("hello");

  wdt_disable();
  power_adc_disable();

  pinMode(2,INPUT);
  pinMode(PIN_METER_ONE,OUTPUT);
  pinMode(PIN_METER_TWO,OUTPUT);
  pinMode(PIN_METER_THREE,OUTPUT);
  //  Serial.begin(115200);
  inputString.reserve(Payload+1);

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
  SendToBase("Starting");

  //SendMessage(inputString);

  byte status=Mirf.getStatus();
  // Serial.println(status);

  if (status==14) {

    // Successful startup.


  } 
  else {
    wanderMeters(5);
    pinMode(13,OUTPUT);
    while (true) {
      digitalWrite(13,HIGH);
      delay(1000);
      digitalWrite(13,LOW);
      delay(500);
    }
  }


  attachInterrupt(0,flagUpdate,RISING);
  flagUpdate();

  SendToBase("Started");


}


void flagUpdate() {
  updateRequested=true;
  msTurnedOn=millis();
  

}


void loop()
{
  
  // waking();


  byte data[Payload];

  if (updateRequested==true) {

    updateRequested=false;
    if ( ( millis() - msGotForecast > msMaxDataAge ) || msGotForecast==0) {
      msRequestedForecast=millis();
      requestUpdate();


    } 
    else {
      // paintScreen();

    }

  }
  if ((millis() - msRequestedForecast > msTimeout ) && 
  (millis() - msRequestedForecast < ( msTimeout+msTimeout ) ) && 
  (millis() - msGotForecast > msMaxDataAge || msGotForecast==0 ) ) {
    //    updateRequested=true;
    //flagUpdate();
    SendToBase("Timeout");

    /*
    char theBuffer[Payload]="";
     ltoa(msGotForecast,theBuffer,10);
     SendToBase(theBuffer);
     */

    msRequestedForecast=millis();
    requestUpdate();

    //timeout
  }

  // is there any data pending? 
  if( Mirf.dataReady() )
  {

    // if (millis() - msRequestedForecast < msTimeout) {
    // msGotForecast = millis();
    // We don't want to cache unrelated status updates from the base station
    // - if a message is received that was not recently requested, do not update receive time

    //}



    Mirf.getData((byte *) &data);
    String thePayload=String((char *)data);

    // Serial.println(thePayload);

    char theCommand=thePayload[5];
    char theMessage[Payload];
    thePayload.substring(6).toCharArray(theMessage,Payload);
    char valueOne[4]="000";
    char valueTwo[4]="000";
    char valueThree[4]="000";
    
    //    SendToBase("Ack");


    for (byte a=0;a<Payload;a++) {
      if (theMessage[a]=='\r' || theMessage[a]=='\n'){
        theMessage[a]='\0';
        // Look for linebreaks and swap them out for end-of-string
      }
    }
    byte length=strlen(theMessage);

    switch (theCommand) {

    case 'A':  // Ack request; don't print anything
      msRequestedForecast=millis();
      break;

    case 'V':  // Values
      if (length>8) {
        msGotForecast=millis();
        thePayload.substring(7,9).toCharArray(valueOne,4);
        thePayload.substring(10,12).toCharArray(valueTwo,4);

        thePayload.substring(13,15).toCharArray(valueThree,4);
   
   /*     Serial.println(thePayload);

        Serial.println(valueOne);
        Serial.println(valueTwo);
        Serial.println(valueThree);
*/
        analogWrite(PIN_METER_ONE,atoi(valueOne));
        analogWrite(PIN_METER_TWO,atoi(valueTwo));
        analogWrite(PIN_METER_THREE,atoi(valueThree));

      } 
      else {
     //   Serial.println(length);
      }


      break;
    case 'u':
    case 'r':
    case 's': 
      // Radio crosstalk  
      break;
    default:
      break;
    }





  }



  // wanderMeters(10);
  delay(1000);

  // delay(250);

  //  char theBuffer[Payload]="";
  //  ltoa(msTurnedOn,theBuffer,10);
  //  SendToBase(theBuffer);

  if (millis() - msTurnedOn > msOnTime ) {



    //   sleeping();

  } 
  else {
    if ( ( ( millis() - msGotForecast ) > msMaxDataAge ) && ( ( millis() - msRequestedForecast ) > msTimeout ) ) {
      msRequestedForecast=millis();
      // SendToBase("Refresh");
      requestUpdate();
      // screen is on, data is old, but let's not flagUpdate because that will keep screen on longer

    }

    //    paintScreen();

  }



}

void requestUpdate() {

  //  msRequestedForecast=millis();
  //      inputString="BASESWCLKKupdate";
  //      SendMessage(inputString);
  SendToBase("update");
  //  blockForSend();
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
    } 
  }
}

void SendToBase(String theMessage) {
  char thePayload[Payload];
  char theMessageChar[Payload];

//  Serial.println(theMessage);

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
    Mirf.powerUpRx(); 

}






void waking() {

  //  setup_watchdog( 	WDTO_500MS ); //Setup watchdog to go off after 500ms

  wdt_reset();

}

void sleeping() {


  // setup_watchdog( 	WDTO_8S ); //Setup watchdog to go off after 500ms
  set_sleep_mode(SLEEP_MODE_IDLE);
  //pinMode(LCD_BACKLIGHT_PIN,INPUT); 

  Mirf.powerDown(); 
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  waking();
  Mirf.powerUpRx(); 
  wdt_disable();


}

ISR(WDT_vect) {


  // buttonPress();  


}



void wanderMeters(int seconds) {

  long msQuitTime=millis() + (seconds * 1000) ;

  byte one=random(255);
  byte two=random(255);
  byte three=random(255);
  byte speed_one=random(5);
  byte speed_two=random(5);
  byte speed_three=random(5);

  analogWrite(PIN_METER_ONE,random(255));
  analogWrite(PIN_METER_TWO,random(255));
  analogWrite(PIN_METER_THREE,random(255));

  while (millis() < msQuitTime) {
    one+=speed_one;
    two+=speed_two;
    three+=speed_three;
    analogWrite(PIN_METER_ONE,one);
    analogWrite(PIN_METER_TWO,two);
    analogWrite(PIN_METER_THREE,three);

    /*    OCR0B++;
     OCR0A++;
     OCR2B++;
     */
    delay(20);
  }




}





void setup_watchdog(int timerPrescaler) {

  if (timerPrescaler > 9 ) timerPrescaler = 9; //Correct incoming amount if need be

  byte bb = timerPrescaler & 7; 
  if (timerPrescaler > 7) bb |= (1<<5); //Set the special 5th bit if necessary

#ifdef __AVR_ATmega328P__
  MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
  WDTCSR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
  WDTCSR = bb; //Set new watchdog timeout value
  WDTCSR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int

#elif defined __AVR_ATtiny85__

  // attiny

  //This order of commands is important and cannot be combined
  MCUSR &= ~(1<<WDRF); //Clear the watch dog reset
  WDTCR |= (1<<WDCE) | (1<<WDE); //Set WD_change enable, set WD enable
  WDTCR = bb; //Set new watchdog timeout value
  WDTCR |= _BV(WDIE); //Set the interrupt enable, this will keep unit from resetting after each int

#else 
  //??
#error "I don't know how to handle your AVR in setup_watchdog"
#endif


}










