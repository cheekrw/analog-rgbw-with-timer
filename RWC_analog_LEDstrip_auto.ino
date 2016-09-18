
//####################################################################################
/* Setup the RTC */
//####################################################################################
// Use the RTC SQW pin as timer for when to check the time and do the loop 
// and TimeLord stuff calculates sunrise and sunset times for each day

#include <Wire.h>
#include <RTClib.h>
#include <RTC_DS3231.h>
#include <TimeLord.h>
#include <RWC_RGBW.h>


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

//####################################################################################
// SETUP
//####################################################################################
void setup() {

  Serial.begin(9600);

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

  tardis.TimeZone(-4 * 60); // tell TimeLord what timezone your RTC is synchronized to. You can ignore DST
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
//  set_string(evening);
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
    int SunSetHour = today[tl_hour];
    int SunSetMinute = today[tl_minute];
    int SunSetMinuteOfDay = SunSetHour * 60 + SunSetMinute;
    tardis.SunRise(today);
    int SunRiseHour = today[tl_hour];
    int SunRiseMinute = today[tl_minute];
    int SunRiseMinuteOfDay = SunRiseHour * 60 + SunRiseMinute;

    int NowMinute = now.hour() * 60 + now.minute();

    int WakeMinute = 5 * 60 + 30; // 5:30 AM
        // Morning ends 30 minutes past sunrise, which may be before I wake.
    int MorningEndMinute = SunRiseMinuteOfDay + 120;
        // Day ends 30 minutes before sunset 
    int DayEndMinute = SunSetMinuteOfDay - 120;

// #include st_patricks.h
     
    if (NowMinute <= WakeMinute && NowMinute <= MorningEndMinute) 
        {fadeto(night);} // color in the very early morning
    else if (NowMinute >= WakeMinute && NowMinute <= MorningEndMinute)     
        {fadeto(morning);} // color I wake to, if I wake before sunrise
    else if (NowMinute >= MorningEndMinute && NowMinute <= DayEndMinute)
        {fadeto(daylight);} // color during daylight hours
    else (fadeto(evening)) // color from 1 hour before sunset to midnight
//     else (july4th()) // color from 1 hour before sunset to midnight
     ;
//*
    Serial.print("Wake Time ");
    Serial.print(int(WakeMinute / 60));
    Serial.print(":");
    Serial.println(WakeMinute % 60);
    Serial.print("Sunrise ");
    Serial.print(int(SunRiseMinuteOfDay / 60));
    Serial.print(":");
    Serial.println(SunRiseMinuteOfDay % 60);
    Serial.print("Morning End ");
    Serial.print(int(MorningEndMinute / 60));
    Serial.print(":");
    Serial.println(MorningEndMinute % 60);
    Serial.print("Sunset ");
    Serial.print(int(SunSetMinuteOfDay / 60));
    Serial.print(":");
    Serial.println(SunSetMinuteOfDay % 60);
    Serial.print("Day End ");
    Serial.print(int(DayEndMinute / 60));
    Serial.print(":");
    Serial.println(DayEndMinute % 60);
//*
    
//*  RTC.forceTempConv(true);  //DS3231 does this every 64 seconds, we are simply testing the function here
    int16_t temp_word = RTC.getTempAsWord();
    int8_t temp_hbyte = temp_word >> 8;
    
    Serial.print("current time ");
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
//*/
  }

    times_up = false;
    delay(60000);

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
  set_string(white_cool);
  }

void christmas (){
  fadeto(white_deck  );
  delay(3000);
  fadeto(red);
  delay(3000);
  fadeto(white_deck  );
  fadeto(green );
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
  fadeto(red_4th);
  delay(1500);
  fadeto(white_4th);
  delay(3000);
  fadeto(blue);
  delay(2000);
  fadeto(white_4th);
//  fadeto(evening);
  }

void rgb(){
  fadeto(white_deck);
  delay(3000);
  fadeto(red);
  delay(100);
  fadeto(white_deck);
  fadeto(green);
  delay(100);
  fadeto(white_deck);
  fadeto(blue);
  delay(100);
  }

void set_string (byte r, byte g, byte b, byte w, float intensity ){
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

