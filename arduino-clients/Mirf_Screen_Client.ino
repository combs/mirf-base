#include <SPI.h>
#include <Mirf.h>
#include <Wire.h>
#include <MirfHardwareSpiDriver.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "watchdog.h"
#include "mirfscreenconfig.h" // include this before MirfClient.h
#include "MirfClient.h" // include this after mirfscreenconfig.h
#include <EEPROM.h>

void setup();

// #include <LiquidCrystal_I2C.h>

#include <LiquidCrystal.h>






String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
volatile byte updateRequested = true;
volatile byte paintRequested = true;

volatile long secondsSinceStartup = 0;
volatile long secondsGotUpdate = 0;
volatile long secondsRequestedUpdate = 0;
volatile long secondsTurnedOn = 0;


// const byte Payload = 32;
char stringUpdate0[Payload];
char stringUpdate1[Payload];
char stringUpdateNow[Payload];
char stringUpdateLater[Payload];
char stringUpdateConditions[Payload];
// char stringBuffer[Payload];

#ifdef LCD_I2C

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 
#else
LiquidCrystal lcd(7, 6, A3, A2, A1, A0);

#endif
//LiquidCrystal_I2C_simple lcd(0x27,16,2); 



void setup()
{

  wdt_disable();
  power_adc_disable();
  power_usart0_disable();

  getNameClient();
  getNameBase();

  pinMode(2,INPUT);

  //  Serial.begin(115200);
  inputString.reserve(Payload+1);
  lcd.begin(16, 2);
  display();

  backlight();
  noBacklight();
  delay(2);
  lcd.clear();
  lcd.print("Starting radio..");

  SetupMirfClient();




#ifdef LCD_I2C
  //  inputString="BASESWCLKKStarting-i2c";
  SendToBase("Starting-i2c");
#else
  //  inputString="BASESWCLKKStarting-4bit";
  SendToBase("Starting-4bit");
#endif

  //SendMessage(inputString);

  byte status=Mirf.getStatus();
  if (status==14) {
    myLCDClear();
    delay(5);
    lcd.print(nameClient);
    lcd.print(" started.");
    lcd.setCursor(0,1); 
    lcd.print("Contacting "); 
    lcd.print(nameBase);
  } 
  else {
    lcd.clear();

    lcd.print("Radio error: ");
    lcd.setCursor(0,1);
    lcd.print(status);
  }

  setup_watchdog(WDTO_1S);

  attachInterrupt(0,flagUpdate,RISING);
  flagUpdate();

  //  inputString="BASESWCLKKStarted";
  //  SendMessage(inputString);
  SendToBase("Started");
  delay(1000); // Avoid message-spamming race condition at startup


}

void myLCDClear() {
  // The library builtin doesn't seem to work with i2c. TODO branch this out
  lcd.home();
  lcd.write("                ");
  lcd.setCursor(0,1);
  lcd.write("                ");
  lcd.home();
}

void flagUpdate() {

  updateRequested=true;
  secondsTurnedOn=secondsSinceStartup;

  paintRequested=true;

}


void loop()
{

  //  waking();

  if (paintRequested==true) {
    display();
    backlight();
    paintScreen();
    paintRequested=false;
  }


  if (secondsSinceStartup - secondsTurnedOn > secondsScreenAwakeTime ) {
#ifdef LCD_CLEAR_ON_SLEEP
    myLCDClear();
#endif

    noBacklight();
    noDisplay();
    sleeping();

  } 

  else {

    Mirf.powerUpRx();

    if ( ( ( secondsSinceStartup - secondsGotUpdate ) > secondsMaxDataAge ) && 
      ( ( secondsSinceStartup - secondsRequestedUpdate ) > secondsTimeout ) ) {
      secondsRequestedUpdate=secondsSinceStartup;
      // ,"Refresh");
      requestUpdate();
      // screen is on, data is old, but let's not flagUpdate because that will keep screen on longer

    }

    display();
    backlight();

    //    paintScreen();

    byte data[Payload];

    if (updateRequested==true) {

      updateRequested=false;

      if  ( ( ( secondsSinceStartup - secondsRequestedUpdate > 
        secondsTimeout+secondsTimeout ) || 
        (secondsGotUpdate==0 && secondsRequestedUpdate==0))  &&  
        ( ( secondsSinceStartup - secondsGotUpdate > secondsMaxDataAge ) || 
        secondsGotUpdate==0)) {

        secondsRequestedUpdate=secondsSinceStartup;
        requestUpdate();

      } 

    }
    if ((secondsSinceStartup - secondsRequestedUpdate > secondsTimeout ) && (secondsSinceStartup - secondsRequestedUpdate < ( secondsTimeout+secondsTimeout ) ) && (secondsSinceStartup - secondsGotUpdate > secondsTimeout+secondsTimeout ) ) {

      SendToBase("Timeout");
      secondsRequestedUpdate=secondsSinceStartup;
      requestUpdate();

    }

    // is there any data pending? 
    if( !Mirf.isSending() && Mirf.dataReady() )
    {

#ifdef LCD_UPDATES_EXTEND_ON_TIME
      secondsTurnedOn=secondsSinceStartup;
#endif

      lcd.noBlink();
      Mirf.getData((byte *) &data);
      String thePayload=String((char *)data);
      char theLine=thePayload[5];
      char theMessage[Payload];
      thePayload.substring(6).toCharArray(theMessage,Payload);

      for (byte a=0;a<Payload;a++) {
        if (theMessage[a]=='\r' || theMessage[a]=='\n'){
          theMessage[a]='\0';
        }
      }
      byte x=0;
      byte y=0;
      byte length=strlen(theMessage);
      switch (theLine) {

      case '$':
        myLCDClear();
        lcd.print("System update...");
        lcd.setCursor(0,1);
        lcd.print(theMessage);
        HandleBuiltinMessage(theMessage);
        delay(500);
        length=0;
        theMessage[0]='\0';

        break;

      case '0': // first line full
        secondsGotUpdate = secondsSinceStartup;
        strcpy(stringUpdate0,theMessage);
        x=0;
        y=0;
        length=16;
        break;

      case '1': // second line full
        secondsGotUpdate = secondsSinceStartup;
        strcpy(stringUpdate1,theMessage);
        x=0;
        y=1;
        length=16;
        break;
      case 'C': // current Conditions
        secondsGotUpdate = secondsSinceStartup;
        strcpy(stringUpdateConditions,theMessage);
        x=0;
        y=1;
        break;
      case 'N': // temp Now
        secondsGotUpdate = secondsSinceStartup;
        strcpy(stringUpdateNow,theMessage);
        x=0;
        y=0;
        break;
      case 'L':  // temp Later
        secondsGotUpdate = secondsSinceStartup;
        strcpy(stringUpdateLater,theMessage);
        x=16-strlen(stringUpdateLater);
        y=0;
        break;
      case 'A':  // Ack request; don't print anything
        x=30;
        y=4;
        length=0;
        theMessage[0]='\0';
        secondsRequestedUpdate=secondsSinceStartup;
        break;
      case 'u':
      case 'r':
      case 's':
        x=0;
        y=1;
        thePayload="Radio crosstalk";
        thePayload.toCharArray(theMessage,Payload);
        length=strlen(theMessage);
        break;

      default:
        break;
      }

      lcd.setCursor(x,y);


      for (int a=0;a<length;a++){
        lcd.write(" ");
      }


      lcd.setCursor(x,y);

      lcd.print( theMessage);

    }
  }

  if (secondsSinceStartup - secondsRequestedUpdate <  
    secondsTimeout+secondsTimeout ) {
    delay(50);
  } 
  else {

    napping();
  }


}

void requestUpdate() {

  lcd.setCursor(strlen(stringUpdateNow),0);
  lcd.blink();
  SendToBase("sense");

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


void paintScreen() {
#ifdef LCD_CLEAR_ON_SLEEP

  if (secondsSinceStartup - secondsGotUpdate > secondsMaxDataAge) {
    myLCDClear();
    return;
  }
#endif

  lcd.setCursor(0,0);
  if (strlen(stringUpdateNow) > 1) {
    myLCDClear();
    lcd.print( stringUpdateNow);
    lcd.setCursor(16-(strlen(stringUpdateLater)),0);
    lcd.print( stringUpdateLater);
  } 
  else if (strlen(stringUpdate0) > 1 ) {
    myLCDClear();
    lcd.print(stringUpdate0);
  }
  lcd.setCursor(0,1);  

  if (strlen(stringUpdateConditions) > 1) {

    lcd.print( stringUpdateConditions); 
  }
  else if (strlen(stringUpdate0) > 1 ) {

    lcd.print(stringUpdate1);
  }

}


void display() {

#ifdef LCD_I2C

  lcd.on();
#else
  lcd.display();
#endif

}

void noDisplay() {
#ifdef LCD_I2C

  lcd.off();
#else
  lcd.noDisplay();
#endif


}



void backlight() {
#ifdef LCD_I2C

  lcd.backlight();
#else
  pinMode(LCD_BACKLIGHT_PIN,OUTPUT);
  digitalWrite(LCD_BACKLIGHT_PIN, HIGH);
  // analogWrite(LCD_BACKLIGHT_PIN, 128);

#endif

}
void noBacklight() {
#ifdef LCD_I2C
  lcd.noBacklight();
#else

  digitalWrite(LCD_BACKLIGHT_PIN, LOW);
  //  pinMode(LCD_BACKLIGHT_PIN,INPUT);
#endif
}






void waking() {
 

}

void sleeping() {

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //pinMode(LCD_BACKLIGHT_PIN,INPUT); 

  Mirf.powerDown(); 
  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  waking();

}



void napping() {

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  //pinMode(LCD_BACKLIGHT_PIN,INPUT); 

  sleep_enable();
  sei();
  sleep_cpu();
  sleep_disable();
  waking();

}

ISR(WDT_vect) {

  if (secondsSinceStartup + 1 < secondsSinceStartup) {
    secondsTurnedOn = 0; 
    secondsGotUpdate = 0;
    secondsRequestedUpdate = 0;
  }

  secondsSinceStartup++;

}

















