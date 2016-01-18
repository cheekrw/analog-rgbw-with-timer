# analog-rgbw-with-timer
code to run an analog RGBW strip from arduino, including a chronodot real time clock and TimeLord library to get sunrise/sunset times for auto on/off.  This is pretty messy code.  I want to improve by using SQW from the RTC into pin5 to run T1, but this dorks with PWM on pins 9 & 10, so I gotta figure that out.

http://playground.arduino.cc/Main/TimerPWMCheatsheet

http://docs.macetech.com/doku.php/chronodot_v2.0

https://learn.adafruit.com/rgb-led-strips/overview

https://github.com/probonopd/TimeLord
