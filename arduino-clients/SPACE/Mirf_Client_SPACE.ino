#include <Arduino.h>
#include <SPI.h>
#include <Mirf.h>
#include <Wire.h>
#include <MirfHardwareSpiDriver.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <EEPROM.h>

void setup();

#include "mirfscreenconfig.h"
#include "watchdog.h"
#include "MirfClient.h"

#define STATE_IDLE 'I'
#define STATE_UPDATE_NEEDED 'U'
#define STATE_SLEEPING 'S'
#define STATE_APPROACHING 'A'
#define STATE_OVERHEAD 'O'
#define STATE_DEPARTING 'D'
#define STATE_TIME_NEEDED 'T' 


volatile char stateCurrent=STATE_TIME_NEEDED;



#define MIRF_PINS_STANDARD
// #define MIRF_PINS_PCB

volatile long secondsUnixTime = 1428000000L;
volatile long secondsSinceStartup = 0L;


volatile long millisZero=0L;
volatile long secondsUpdateRequested = 0L;
volatile long secondsTimeRequested = 0L;



#define PIN_OUTPUT 5
#define PIN_GROUND 4
#define PIN_NRF A3

volatile long secondsRise = 0L;
volatile long secondsDuration = 0L;

void setup()
{
  //  Serial.begin(115200);
  //  Serial.println("hello");

  wdt_disable();

  power_adc_disable();
  power_timer1_disable();
  power_timer2_disable();
  power_twi_disable();
  // power_usart0_disable();

  getNameClient();
  getNameBase();

  pinMode(PIN_NRF,OUTPUT);  
  pinMode(PIN_OUTPUT,OUTPUT); 
  pinMode(PIN_GROUND,OUTPUT);
  digitalWrite(PIN_GROUND,LOW);

  wakeRadio();

  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  digitalWrite(PIN_OUTPUT,HIGH);
  delay(1000);
  digitalWrite(13,LOW);
  digitalWrite(PIN_OUTPUT,LOW);
  delay(500);


  Serial.begin(115200);


  SendToBase("Starting");

  byte status=Mirf.getStatus(); 

  if (status==14) { 

  } 
  else {  
    while (true) {
      pinMode(13,OUTPUT);
      digitalWrite(13,HIGH);
      delay(1000);
      digitalWrite(13,LOW);
      delay(500); 
    }
  }

  setup_watchdog(WDTO_1S);

  SendToBase("Started");
  Serial.println("Started");
}



void loop()
{
  byte data[Payload];

  Serial.print(stateCurrent);

  switch(stateCurrent) {
  case STATE_IDLE :
  case STATE_SLEEPING : 
    if (isTimeNeeded() ) {
      stateCurrent=STATE_TIME_NEEDED;
    } 
    else if (isUpdateNeeded() ) {
      stateCurrent=STATE_UPDATE_NEEDED;
    } 
    else if (isOverhead() ) {
      stateCurrent=STATE_OVERHEAD;
    } 
    else if (isApproaching() ){
      millisZero=millis();
      stateCurrent=STATE_APPROACHING;
    } 
    else if (isDeparting() ) {
      millisZero=millis();
      stateCurrent=STATE_DEPARTING;
    }
    else { 
      sleeping();
    }
    break;
  case STATE_UPDATE_NEEDED :
    if (isUpdateNeeded()==false) {
      stateCurrent=STATE_IDLE;
    } 
    else if (isUpdateRequested()==false) {
      requestUpdate();
      Serial.print("r");

    } 
    else {
      Serial.print("n");
      napping(); 
    }

    break; 
  case STATE_APPROACHING :
    if (isOverhead() ) {
      stateCurrent=STATE_OVERHEAD;
    }     
    else {
      long thisTime= ( 30 - (secondsRise - secondsUnixTime) ) * 1000L;
      thisTime += ((millis() - millisZero) % 1000);
      analogWrite(PIN_OUTPUT,map(thisTime,0,30000,0,255)); 
    }
    break;
  case STATE_OVERHEAD :
    if (isDeparting() ) {
      millisZero=millis();
      stateCurrent=STATE_DEPARTING;
    } 
    else {
      digitalWrite(PIN_OUTPUT,HIGH);
      sleeping();
    }

    break;
  case STATE_DEPARTING :
    if (isDeparting()==false) {
      stateCurrent=STATE_IDLE;
      digitalWrite(PIN_OUTPUT,LOW);
    } 
    else {
      long thisTime = (  (secondsRise + secondsDuration + 30) - secondsUnixTime) * 1000L;
      thisTime +=((millis() - millisZero) % 1000); 
      analogWrite(PIN_OUTPUT,map(thisTime,0,30000,0,255));
    }

    break;
  case STATE_TIME_NEEDED :
    if (isTimeNeeded()==false) {
      stateCurrent=STATE_IDLE;
    } 
    else if (isTimeRequested()==false) {
      requestTime();
      Serial.print("r");
    } 
    else {
      Serial.print("n");
      napping(); 
    }
    break;  
  }

  if( Mirf.dataReady() )
  { 
    Mirf.getData((byte *) &data);
    String thePayload=String((char *)data);
    char theCommand=thePayload[5];
    char theMessage[Payload];
    thePayload.substring(6).toCharArray(theMessage,Payload);

    for (byte a=0;a<Payload;a++) {
      if (theMessage[a]=='\r' || theMessage[a]=='\n'){
        theMessage[a]='\0';
      }
    }

    byte length=strlen(theMessage);
    switch (theCommand) {

    case '$':  
      HandleBuiltinMessage(theMessage);   
      break;

    case 'S':
      // Sleep 

      break;

    case 's':
      // spaaaace 
      {
        volatile long secondsChecksum = 0L;

        secondsRise=atol(strtok(theMessage," "));
        secondsDuration=atol(strtok(NULL," "));
        secondsChecksum=atol(strtok(NULL," "));

        if (secondsChecksum != (secondsRise - secondsDuration) ) {
          secondsRise=0;
          secondsDuration=0;
          SendToBase("Checksum-failure");
          SendToBase(theMessage);
        }


        /*
        char debugMessage[Payload]="";
         ltoa(secondsRise,debugMessage,10);
         SendToBase(debugMessage);
         ltoa(secondsDuration,debugMessage,10);
         SendToBase(debugMessage);
         */

        break;
      }
    case 't':
      stateCurrent=STATE_IDLE;
      {
        // unix time

        char theFirst[11];
        char theSecond[11];
        byte updateDirty=0;

        byte length=strlen(theMessage);
        if (length != 21) {
          updateDirty=1;
        }
        byte theVar=1;
        theFirst[10]='\0';
        theSecond[10]='\0';

        for (byte a=0;a<21;a++) {
          if (theMessage[a]=='|') {
            theVar=2; 
          } 
          else {
            if (theVar==1) {
              theFirst[a]=theMessage[a];
            } 
            else if (theVar==2) {
              theSecond[a-11]=theMessage[a];
              if (theMessage != '\0' && theMessage[a-11] != theMessage[a]) {
                updateDirty=1;
              }

            }

          }

        }
        if (updateDirty==0) {
          secondsUnixTime=atol(theFirst);
          SendToBase(theFirst);
        } 
        else {
          SendToBase("Time-rejected");
        }

        break;
      }
    case 'A':  // Ack request; don't print anything

      // reset the timeout.
      break;
    case 'r': // Send rise time
      {
        char theTime[Payload]="rise-";
        char buffer [sizeof(long)*8+1];
        ltoa(secondsRise,buffer,10);
        strcat(theTime,buffer);
        SendToBase(theTime);
      }


      break;

    case 'u': // Send unix time
      {
        char theTime[Payload]="unix-";
        char buffer [sizeof(long)*8+1];
        ltoa(secondsUnixTime,buffer,10);
        strcat(theTime,buffer);
        SendToBase(theTime);
      }
      break;

    case 'c': // Send current state
      {
        char theState[8]="state- ";
        theState[6]=stateCurrent;
        SendToBase(theState);
      }

      break;


    default:
      break;
    }


  }

}

void requestTime() {

  if ( digitalRead(PIN_NRF)==LOW) {
    wakeRadio();
    Serial.print("w");  
}
  
  SendToBase("get-time");
  secondsTimeRequested=secondsUnixTime;
}


void requestUpdate() {
  if ( digitalRead(PIN_NRF)==LOW) {
    wakeRadio();
  }
  SendToBase("update");
  secondsUpdateRequested=secondsUnixTime;
}



byte isApproaching() {

  if ( (secondsRise != 0) && (secondsUnixTime > secondsRise - 30) && (secondsUnixTime < secondsRise)) {
    return true;
  } 
  else { 
    return false;
  }     
}



byte isOverhead() {
  if ( (secondsRise != 0 ) && ( secondsUnixTime >= secondsRise ) && ( secondsUnixTime <= ( secondsRise + secondsDuration ) ) ) {
    return true;
  } 
  else {
    return false;
  }
}


byte isDeparting() {
  if ( (secondsRise != 0) && (secondsUnixTime > (secondsRise + secondsDuration)) && 
    ( secondsUnixTime < (secondsRise + secondsDuration + 30))) {
    return true;
  } 
  else {
    return false;
  }  
}


byte isTimeNeeded() {
  if ( secondsSinceStartup % 3600 == 0 || secondsUnixTime < 1428000000 + 10) {
    return true;
  } 
  else {
    return false;
  }
}

byte isUpdateNeeded() {
  if (  ( secondsUnixTime > (secondsRise + secondsDuration + 30 )) || secondsRise==0 ) {
    return true;
  } 
  else {
    return false;
  }  
}



byte isUpdateRequested() {
  if (secondsUnixTime - secondsUpdateRequested > secondsTimeout) {
    return false;
  } 
  else {
    return true;
  }
}

byte isTimeRequested() {
  if (secondsUnixTime - secondsTimeRequested > secondsTimeout) {
    return false;
  } 
  else {
    return true;
  }
}


void waking() {


}

void sleeping() {

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //pinMode(LCD_BACKLIGHT_PIN,INPUT); 

  sleepRadio();

  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  waking();


}



void napping() {

  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  //pinMode(LCD_BACKLIGHT_PIN,INPUT); 

  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  waking();

}

ISR(WDT_vect) {

  if (secondsUnixTime + 1 < secondsUnixTime) {
    secondsUpdateRequested = 0;
    secondsTimeRequested = 0;
  }

  secondsSinceStartup++;
  secondsUnixTime++;

}



void wakeRadio() {
//  SPI.begin();
  digitalWrite(PIN_NRF,HIGH);
//  delay(200); // Power back up those bypass caps
  SetupMirfClient();

}


void sleepRadio() {

  Mirf.powerDown(); 
  digitalWrite(PIN_NRF,LOW);
//  pinMode(11,INPUT);
//  pinMode(12,INPUT);
//  pinMode(13,INPUT);
  //  pinMode(PIN_NRF,INPUT);  

}
























