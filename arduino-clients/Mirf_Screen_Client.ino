#include <SPI.h>
#include <Mirf.h>
#include <Wire.h>
#include <MirfHardwareSpiDriver.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "watchdog.h"
#include "MirfClient.h"


void setup();

// #include <LiquidCrystal_I2C.h>

#include <LiquidCrystal.h>

#include "mirfscreenconfig.h"

//#define LCD_I2C
#define LCD_PCB

#define LCD_BACKLIGHT_PIN 10

// #define MIRF_PINS_STANDARD
#define MIRF_PINS_PCB


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
volatile byte updateRequested = true;
volatile byte paintRequested = true;
volatile long msTurnedOn = 0;



// const long msScreenUpdateInterval=1000;

long msScreenUpdated=0;

volatile long msGotForecast = 0;
volatile long msRequestedForecast = 0;
// const byte Payload = 32;
char stringForecast0[Payload];
char stringForecast1[Payload];
char stringForecastNow[Payload];
char stringForecastLater[Payload];
char stringForecastConditions[Payload];
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
  
  SetupMirfClient(nameClient,nameBase);
  



#ifdef LCD_I2C
  //  inputString="BASESWCLKKStarting-i2c";
  SendToBase(nameClient,nameBase,"Starting-i2c");
#else
  //  inputString="BASESWCLKKStarting-4bit";
  SendToBase(nameClient,nameBase,"Starting-4bit");
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


  attachInterrupt(0,flagUpdate,RISING);
  flagUpdate();

  //  inputString="BASESWCLKKStarted";
  //  SendMessage(inputString);
  SendToBase(nameClient,nameBase,"Started");

  setup_watchdog(WDTO_1S);
  

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
  msTurnedOn=millis();
  //  backlight();
  //  lcd.on();
  //  paintScreen();
  paintRequested=true;

}


void loop()
{

  waking();


  if (paintRequested==true) {
    display();
    backlight();
    paintScreen();
    paintRequested=false;  
    //    inputString="BASESWCLKKpainting";
    //    SendMessage(inputString);
  }

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
  if ((millis() - msRequestedForecast > msTimeout ) && (millis() - msRequestedForecast < ( msTimeout+msTimeout ) ) && (millis() - msGotForecast > msTimeout+msTimeout ) ) {
    //    updateRequested=true;
    //flagUpdate();
    SendToBase(nameClient,nameBase,"Timeout");

    /*
    char theBuffer[Payload]="";
     ltoa(msGotForecast,theBuffer,10);
     SendToBase(nameClient,nameBase,theBuffer);
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

#ifdef LCD_UPDATES_EXTEND_ON_TIME
    msTurnedOn=millis();
#endif
    // lcd.noCursor();
    lcd.noBlink();
    Mirf.getData((byte *) &data);
    String thePayload=String((char *)data);
    char theLine=thePayload[5];
    char theMessage[Payload];
    thePayload.substring(6).toCharArray(theMessage,Payload);

    //    SendToBase(nameClient,nameBase,"Ack");
    for (byte a=0;a<Payload;a++) {
      if (theMessage[a]=='\r' || theMessage[a]=='\n'){
        theMessage[a]='\0';
        // Look for linebreaks and swap them out for end-of-string
      }
    }
    byte x=0;
    byte y=0;
    byte length=strlen(theMessage);
    switch (theLine) {
    case '0': // first line full
      msGotForecast = millis();
      strcpy(stringForecast0,theMessage);
      x=0;
      y=0;
      length=16;
      break;

    case '1': // second line full
      msGotForecast = millis();
      strcpy(stringForecast1,theMessage);
      x=0;
      y=1;
      length=16;
      break;
    case 'C': // current Conditions
      msGotForecast = millis();
      strcpy(stringForecastConditions,theMessage);
      x=0;
      y=1;
      break;
    case 'N': // temp Now
      msGotForecast = millis();
      strcpy(stringForecastNow,theMessage);
      x=0;
      y=0;
      break;
    case 'L':  // temp Later
      msGotForecast = millis();
      strcpy(stringForecastLater,theMessage);
      x=16-strlen(stringForecastLater);
      y=0;
      break;
    case 'A':  // Ack request; don't print anything
      x=30;
      y=4;
      length=0;
      theMessage[0]='\0';
      msRequestedForecast=millis();
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

    //    lcd.write("                ");

    lcd.setCursor(x,y);
    //    lcd.write(theLine);
    lcd.print( theMessage);


    //        thePayload.substring(5,1).toCharArray(theFrom,6);

    // ... and write it out to the PC
    //   Serial.println((char *) data);
  }




  delay(250);
  //  char theBuffer[Payload]="";
  //  ltoa(msTurnedOn,theBuffer,10);
  //  SendToBase(nameClient,nameBase,theBuffer);

  if (millis() - msTurnedOn > msOnTime ) {
#ifdef LCD_CLEAR_ON_SLEEP
    myLCDClear();
#endif

    noBacklight();
    noDisplay();
    sleeping();

  } 
  else {
    if ( ( ( millis() - msGotForecast ) > msMaxDataAge ) && ( ( millis() - msRequestedForecast ) > msTimeout ) ) {
      msRequestedForecast=millis();
      // ,"Refresh");
      requestUpdate();
      // screen is on, data is old, but let's not flagUpdate because that will keep screen on longer

    }

    display();
    backlight();

    //    paintScreen();

  }



}

void requestUpdate() {

  lcd.setCursor(strlen(stringForecastNow),0);
  lcd.blink();
  //  msRequestedForecast=millis();
  //      inputString="BASESWCLKKupdate";
  //      SendMessage(inputString);
  SendToBase(nameClient,nameBase,"update");
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


void paintScreen() {
#ifdef LCD_CLEAR_ON_SLEEP

  if (millis() - msGotForecast > msMaxDataAge) {
    myLCDClear();
    return;
  }
#endif

  lcd.setCursor(0,0);
  if (strlen(stringForecastNow) > 1) {
    myLCDClear();
    lcd.print( stringForecastNow);
    //    lcd.print(strlen(stringForecastNow));
    lcd.setCursor(16-(strlen(stringForecastLater)),0);
    lcd.print( stringForecastLater);
  } 
  else if (strlen(stringForecast0) > 1 ) {
    myLCDClear();
    lcd.print(stringForecast0);
  }
  lcd.setCursor(0,1);  

  if (strlen(stringForecastConditions) > 1) {

    lcd.print( stringForecastConditions); 
  }
  else if (strlen(stringForecast0) > 1 ) {

    lcd.print(stringForecast1);
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

}




 






