
#include <Arduino.h>

char nameBase[6] = "BASES";

char nameClient[6] = "WINKY";


const long secondsScreenAwakeTime=30*60;
const long secondsTimeout=10;
const long secondsMaxDataAge=60*60L;

const long secondsSleep=30;


// obsolete:


const long msOnTime=30*60*1000L;
const long msTimeout=120000;
const long msMaxDataAge=60*60*1000L;

// #define LCD_CLEAR_ON_SLEEP
#define LCD_UPDATES_EXTEND_ON_TIME

