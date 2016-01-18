//####################################################################################
/* Setup the RTC */
//####################################################################################
// Use the RTC SQW pin as timer for when to check the time and do the loop 
// and TimeLord stuff calculates sunrise and sunset times for each day

#include <Wire.h>
#include <RTClib.h>
#include <RTC_DS3231.h>
#include <TimeLord.h>

TimeLord tardis;
RTC_DS3231 RTC;

// easier to reference here...see .h file for more options
#define SQW_FREQ DS3231_SQW_FREQ_1      //  0b00000000  1Hz
//#define SQW_FREQ DS3231_SQW_FREQ_1024     //0b00001000   1024Hz
//#define SQW_FREQ DS3231_SQW_FREQ_4096  // 0b00010000   4096Hz
//#define SQW_FREQ DS3231_SQW_FREQ_8192 //0b00011000      8192Hz

#define PWM_COUNT 300  // one minute at 1024 Hz = 61440

#define RTC_SQW_IN 5     // input square wave from RTC into T1 pin (D5)
                               //WE USE TIMER1 so that it does not interfere with Arduino delay() command

float const LONGITUDE = -73.69;
float const LATITUDE = 41.34;

//----------- GLOBALS  -------------------------
  volatile bool times_up = false;

//####################################################################################
// INTERRUPT SERVICE ROUTINES
//####################################################################################
    
ISR(TIMER1_COMPA_vect) {
    times_up = true;
}    


//####################################################################################
// SETUP RGBW LED STUFF
//####################################################################################
// connect an RGBW LED to the PWM pins as indicated
const int REDPIN = 9;
const int GREENPIN = 10;
const int BLUEPIN = 11;
const int WHITEPIN = 3;
byte r = 0, g = 0, b = 0, w = 0,
     intensity = 100;  // 100 is max

unsigned long msec_tgt = 1000;  // make this higher to slow down

#define evening      0,   0,   0,  75
#define night        0,   0,  10,   0
#define morning      0,   0,   0, 100
#define white_full 255, 147,  25, 255
#define white        0,   0,   0, 255
#define white_dim    0,   0,   0,   5
#define white_deck   0,   0,   0,  75
#define pink       255,   0, 100,  10
#define red        255,   0,   0,   0
#define green        0, 255,   0,   0
#define blue         0,   0, 255,   0
#define light_blue  10,  10, 255,  40
#define rose       147,  42,  42,   0 // lots of strobe
#define purple     200,   0, 255,   0
#define brown2     139,  69,  19,   0
#define brown3     139,  80,  14,   0
#define orange     250,  40,   0,   0
#define yellow     255, 150,   0,   0
#define off          0,   0,   0,   0

//####################################################################################
// SETUP
//####################################################################################
void setup() {

//  Serial.begin(57600);

  pinMode(RTC_SQW_IN, INPUT);
  
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(WHITEPIN, OUTPUT);
  
  analogWrite(REDPIN, r);
  analogWrite(GREENPIN, g);
  analogWrite(BLUEPIN, b);
  analogWrite(WHITEPIN, w);

   //-------- sunrise sunset stuff ------

  tardis.TimeZone(-5 * 60); // tell TimeLord what timezone your RTC is synchronized to. You can ignore DST
  // as long as the RTC never changes back and forth between DST and non-DST
  tardis.Position(LATITUDE, LONGITUDE); // tell TimeLord where in the world we are
  
     //--------RTC SETUP ------------
    Wire.begin();
    RTC.begin();

    if (! RTC.isrunning()) {
      // Serial.println("RTC is NOT running!");
      // following line sets the RTC to the date & time this sketch was compiled
      RTC.adjust(DateTime(__DATE__, __TIME__));
    }
  
    DateTime now = RTC.now();
    DateTime compiled = DateTime(__DATE__, __TIME__);
    if (now.unixtime() < compiled.unixtime()) {
      //Serial.println("RTC is older than compile time!  Updating");
      RTC.adjust(DateTime(__DATE__, __TIME__));
    }
    
    RTC.enable32kHz(false);
    RTC.SQWEnable(false);//  enable output to the SQW pin, but not during battery operation (saves battery)
    RTC.BBSQWEnable(false);// enable output to the SQW pin, even during battery operation  
    RTC.SQWFrequency( SQW_FREQ );
  
    //char datastr[100];
    //RTC.getControlRegisterData( datastr[0]  );
    //Serial.print(  datastr );
 
    //--------COUNTER 1 SETUP -------
    TCCR1B = (TCCR1B & 0b11111000) | 0x03; //sets T1 (PWM pins 9 & 10) to standard internal clock 
    //setupTimer1ForCounting((int)PWM_COUNT); // sets T1 to use external clock e.g. from RTC 
//    printTimer1Info();   

}

//####################################################################################
// MAIN
//####################################################################################
void loop() {
//  deck_light();
//  work_light();
//  rgb();
//  set_string(rose);
//  christmas();
//  halloween();
//  july4th();

 times_up = true;   // set to false if want to skip auto timing stuff
                    // need to figure out how to use an inerrupt on T1 for this

  if(times_up) {

    DateTime now = RTC.now();
    
    // store today's date (at noon) in an array for TimeLord to use
    byte today[] = {0, 0, 12, now.day(), now.month(), now.year()};

    tardis.SunSet(today);
    int evening_on_hour = today[tl_hour] - 1;
    int evening_on_minute = today[tl_minute];
    tardis.SunRise(today);
    int morning_off_hour = today[tl_hour] + 1;
    int morning_off_minute = today[tl_minute];

    if (now.hour() <= 5
        &&
        now.minute() <= 30)     
        {fadeto(night, intensity);} // color from midnight to 5:30am
    else if (now.hour() <= morning_off_hour
        &&
        now.minute() <= morning_off_minute)     
        {fadeto(morning, intensity);} // color from 5:30am to 1 hour past sunrise
    else if (now.hour() <= evening_on_hour
        &&
        now.minute() <= evening_on_minute)
        {fadeto(off, 0);} // color from 1 hour past sunrise to 1 hour before sunset
    else (fadeto(evening, intensity)) // color from 1 hour before sunset to midnight
      ;
/*
    Serial.print("Evening on ");
    Serial.print(evening_on_hour);
    Serial.print(":");
    Serial.println(evening_on_minute);
    Serial.print("Morning off ");
    Serial.print(morning_off_hour);
    Serial.print(":");
    Serial.println(morning_off_minute);
    Serial.print("current time ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.println(now.minute());
    Serial.println();
/*    
    
/*  RTC.forceTempConv(true);  //DS3231 does this every 64 seconds, we are simply testing the function here
    int16_t temp_word = RTC.getTempAsWord();
    int8_t temp_hbyte = temp_word >> 8;
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    int C_Temp = temp_hbyte;
    Serial.print("Temp: ");
    Serial.print(int(C_Temp * 1.8 + 32 ));
    Serial.println("_F");
    
    Serial.println();
    
    Serial.println();

*/
  }

    times_up = false;
    delay(300000);

}

//####################################################################################
// FUNCTIONS
//####################################################################################

//#########################################################
// TIMER/COUNTER CONTROLS 
//   Timer 0 is PD4
//   Timer 1 is PD5
//#########################################################

void setupTimer1ForCounting(int count) {
  //set WGM1[3:0] to 0b0100 for CTC mode using OCR1A. Clear Timer on Compare Match, OCR1A sets top. 
  //                            Counter is cleared when TCNT0 reaches OCR0A
  //set CS1[2:0] to 0b111 for external rising edge T1 clocking.
  //set OCR1A to count
  //set TIMSK1 to OCIE1A

  //clear it out
  TCCR1A = 0;      //nothing else to set
  TCCR1B = 0;
  TIMSK1 = 0;
 
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12);
  TCCR1B |= (1 << CS11);
  TCCR1B |= (1 << CS10);  
  
  TCNT1 = 0;
  
  OCR1A = count;      // SET COUNTER 
  
  TIMSK1 |= (1 << OCIE1A);
  }

void printTimer1Info() {
  Serial.println(" ");  
  Serial.println("----- Timer1 Information -----");
  Serial.print("TCCR1A: ");
  Serial.println(TCCR1A, BIN);
  Serial.print("TCCR1B: ");
  Serial.println(TCCR1B, BIN);
  Serial.print("TIMSK1: ");
  Serial.println(TIMSK1, BIN);
  Serial.print("OCR1A: " );
  Serial.println(OCR1A, BIN);
  Serial.println("------------------------------");
  Serial.println(" ");  
  }
  
//####################################################################################
// RGBW SETUP
//####################################################################################

void deck_light(){
  set_string(white_deck);
  }

void work_light(){
  set_string(white_full);
  }

void christmas (){
  fadeto(white_deck, intensity );
  delay(3000);
  fadeto(red, 40);
  delay(3000);
  fadeto(white_deck, intensity );
  fadeto(green, 30 );
  delay(3000);
  }

void  halloween () {
/*  fadeto(white_deck, 100);
  delay(1000);
  fadeto(orange, 100);
  delay(5000);
*/
  set_string(orange);
  }

void july4th() {
  fadeto(white_deck, 100);
  delay(3000);
  fadeto(red, 100);
  delay(1000);
  fadeto(white_full, 30);
  delay(1000);
  fadeto(blue, 100);
  delay(1000);
  }

void rgb(){
  fadeto(white_deck, intensity);
  delay(3000);
  fadeto(red, intensity);
  delay(100);
  fadeto(white_deck, intensity);
  fadeto(green, intensity);
  delay(100);
  fadeto(white_deck, intensity);
  fadeto(blue, intensity);
  delay(100);
  }

void set_string (byte r, byte g, byte b, byte w ){
  analogWrite(REDPIN, r );
  analogWrite(GREENPIN, g );
  analogWrite(BLUEPIN, b );
  analogWrite(WHITEPIN, w );
  }
  
void fadeto (byte r_new, byte g_new, byte b_new, byte w_new, float intensity ){
  intensity = intensity / 100;
  unsigned long msec_zero = millis();
  float msec_delta = 0, msec_ratio = 0;
  byte r_init = r, g_init=g, b_init = b, w_init = w;
  int r_delta = (r_new * intensity) - r,
      g_delta = (g_new * intensity) - g,
      b_delta = (b_new * intensity) - b,
      w_delta = (w_new * intensity) - w; 

  while ( msec_delta <= msec_tgt ) {
    r = r_init + (r_delta * msec_ratio);
    analogWrite(REDPIN, r );
    g = g_init + (g_delta * msec_ratio);
    analogWrite(GREENPIN, g );
    b = b_init + (b_delta * msec_ratio);
    analogWrite(BLUEPIN, b );
    w = w_init + (w_delta * msec_ratio);
    analogWrite(WHITEPIN, w );
    msec_delta = millis() - msec_zero;
    msec_ratio = msec_delta / msec_tgt;
  }
}

