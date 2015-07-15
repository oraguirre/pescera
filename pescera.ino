//Arduino Statements
  //#include "Wire.h"
  //#include <TM1637Display.h>
  //#include <solarcalc.h>

//Spark Core Statements
#include "TM1637Display/TM1637Display.h"
#include "solarcalc/solarcalc.h"
#include "math.h"

#define DS3231_I2C_ADDRESS 0x68

const int PinCLK 	= 2;
const int PinDIO 	= 3;
const int PinWhiteLed 	= 4;
const int PinBlueLed 	= 5;
const int PinAutoLed 	= 6;
const int PinSwitch 	= 7;
const int PinSensor	= 8;
const int PinFadeWhiteLed = 9;
const int PinFadeBlueLed = 10;
const int PinSensorLed  = 13;

float latitude = -34.9290;
float longitude = 138.6010;
float zenith = 90.833;
float timezone = 9.5;

int buttonState = 0;        // current state of the button
int lastButtonState = 0;
int buttonFunction;
int AutoLedState;
boolean wLed, bLed;
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
byte hour00, hour01, midnight = 22;
long interval = 1000; // every second
byte segto;
int FadeValueWhiteLed;
int FadeValueBlueLed;


int sensorState = 0;
bool sensor_triggered = false;
int count = 0;
int sensor_timeoff = 300;

TM1637Display display(PinCLK, PinDIO);

solarcalc solardata = solarcalc(latitude, longitude, zenith, timezone);

// Convert time to a fraction number
float fractionTime(byte hour, byte minutes) {
  float result;
  result = float(hour) + (float(minutes) / 60);
  return result;
}

// Turn on or off led using time limits
boolean LedControl(float time, float time_on, float time_off) {
  boolean result;
  if (time_off > time_on) {
    if (time >= time_on && time <= time_off) {
      result = true;
    }
    else {
      result = false;
    }
  }
  else {
    if (time >= time_on || time <= time_off) {
      result = true;
    }
    else {
      result = false;
    }
  }
  return result;
}

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}

// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}


void FadeIn(int pin) {
  if (FadeValueWhiteLed < 255) {// fade in from min to max in increments of 5 points:
    for (FadeValueWhiteLed = 0; FadeValueWhiteLed <= 255; FadeValueWhiteLed += 1) {
      // sets the value (range from 0 to 255):
      analogWrite(pin, FadeValueWhiteLed);
      // wait for 10 milliseconds to see the dimming effect
      delay(10);
    }
  }
}

void FadeOut(int pin) {
  if (FadeValueWhiteLed > 0) { // fade out from max to min in increments of 5 points:
    for (FadeValueWhiteLed = 255; FadeValueWhiteLed >= 0; FadeValueWhiteLed -= 1) {
      // sets the value (range from 0 to 255):
      analogWrite(pin, FadeValueWhiteLed);
      // wait for 10 milliseconds to see the dimming effect
      delay(10);
    }
  }
}

void setup() {
  pinMode(PinWhiteLed, OUTPUT);
  pinMode(PinBlueLed, OUTPUT);
  pinMode(PinAutoLed, OUTPUT);
  pinMode(PinSwitch, INPUT);
  pinMode(PinSensor, INPUT);
  pinMode(PinSensorLed, OUTPUT);

  Wire.begin();
  Serial.begin(9600);
  // set the initial time here:
  // DS3231 seconds, minutes, hours, day, date, month, year
  // setDS3231time(00,47,21,7,27,06,15);

  FadeValueBlueLed = 0;
  FadeValueWhiteLed = 0;


}

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
                   dayOfMonth, byte month, byte year) {
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}

void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

void loop() {

  // retrieve data from DS3231
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

  // calculate sunrise sunset and current time
  solardata.time(int(year) + 2000, int(month), int(dayOfMonth), int(hour), int(minute), int(second));
  solardata.calculations();
  Serial.println("-----------------------");
  Serial.println(solardata.current_time());
  Serial.println(solardata.julian_date());
  Serial.println(solardata.solar_noon());
  Serial.println(solardata.sunrise_time());
  Serial.println(solardata.sunset_time());
  Serial.println(solardata.equation_of_time());
  Serial.println(solardata.solar_elevation_atm());
  Serial.println(solardata.solar_declination());
  Serial.println(solardata.solar_elevation_angle());

  // display current time in TM1637Display
  display.setBrightness(0x0a);
  hour00 = floor(hour / 10);
  display.showNumberDec(hour00, true, 1, 0);
  hour01 = floor(hour % 10);
  segto = 0x80  | display.encodeDigit(hour01);
  display.setSegments(&segto, 1, 1);
  display.showNumberDec(minute, true, 2, 2);

  buttonState = digitalRead(PinSwitch);
  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      buttonFunction++;
      if (buttonFunction > 3) {
        buttonFunction = 0;
      }
    }
  }
  lastButtonState = buttonState;

  sensorState = digitalRead(PinSensor);
  digitalWrite(PinSensorLed, sensorState);

  switch (buttonFunction) {
    case 0:
      wLed = LedControl(solardata.current_time(), solardata.sunset_time(), midnight);
      bLed = LedControl(solardata.current_time(), midnight, solardata.sunrise_time());

      // turn on white led and turn off blue led
      if ((wLed == true) && (bLed == false)) {
        digitalWrite(PinWhiteLed, !wLed);
        digitalWrite(PinBlueLed, !bLed);
        FadeIn(PinFadeWhiteLed);
        FadeOut(FadeValueBlueLed);
      }

      // turn on blue led and turn off white led (motion sensor dependant)
      if ((wLed == false) && (bLed == true)) {
        if (sensorState == HIGH) {
          sensor_triggered = HIGH;
          count = 0;
        }
        if (sensor_triggered == HIGH) {
          digitalWrite(PinBlueLed, bLed);
          digitalWrite(PinWhiteLed, wLed);
          FadeIn(PinFadeWhiteLed);
          FadeOut(FadeValueBlueLed);
          count++;
          if (count > sensor_timeoff) {
            count = 0;
            sensor_triggered = LOW;
          }
        }
        else {
          digitalWrite(PinWhiteLed, !wLed);
          digitalWrite(PinBlueLed, !bLed);
          FadeIn(FadeValueBlueLed);
          FadeOut(PinFadeWhiteLed);
        }
      }

      // Turn off all leds
      if ((wLed == false) && (bLed == false)) {
        digitalWrite(PinWhiteLed, !wLed);
        digitalWrite(PinBlueLed, !bLed);
        FadeOut(FadeValueBlueLed);
        FadeOut(PinFadeWhiteLed);
      }
      digitalWrite(PinAutoLed, HIGH);
      break;
    case 1:
      // White Light on
      digitalWrite(PinWhiteLed, LOW);
      digitalWrite(PinBlueLed, HIGH);
      FadeIn(PinFadeWhiteLed);
      FadeOut(FadeValueBlueLed);
      digitalWrite(PinAutoLed, LOW);
      break;
    case 2:
      // Blue Light on
      digitalWrite(PinWhiteLed, HIGH);
      digitalWrite(PinBlueLed, LOW);
      FadeIn(FadeValueBlueLed);
      FadeOut(PinFadeWhiteLed);
      digitalWrite(PinAutoLed, LOW);
      break;
    case 3:
      // Non state selected
      digitalWrite(PinWhiteLed, HIGH);
      digitalWrite(PinBlueLed, HIGH);
      FadeOut(FadeValueBlueLed);
      FadeOut(PinFadeWhiteLed);
      AutoLedState = !AutoLedState;
      digitalWrite(PinAutoLed, AutoLedState);
      count = 0;
      sensor_triggered = LOW;
      break;
    default:
      // default off
      digitalWrite(PinWhiteLed, HIGH);
      digitalWrite(PinBlueLed, HIGH);
      digitalWrite(PinAutoLed, LOW);
      analogWrite(PinFadeBlueLed, 0);
      analogWrite(PinFadeWhiteLed, 0);
  }

  delay(interval);

}
