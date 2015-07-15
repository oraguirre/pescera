//Spark Core Statements
#include "TM1637Display/TM1637Display.h"
#include "solarcalc/solarcalc.h"
#include "math.h"

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
float current_time;

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

solarcalc solardata(latitude, longitude, zenith, timezone);

float fractionTime(int hour, int minutes) {
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

  Serial.begin(9600);
  Spark.syncTime();
  Time.zone(timezone);

  FadeValueBlueLed = 0;
  FadeValueWhiteLed = 0;
}

void loop() {

  current_time=fractionTime(Time.hour(), Time.second());
  
  // calculate sunrise sunset and current time
  solardata.time(Time.year(), Time.month(), Time.day(), Time.hour(), Time.second(), Time.second());
  solardata.calculations();

  // display current time in TM1637Display
  display.setBrightness(0x0a);
  hour00 = floor(Time.hour() / 10);
  display.showNumberDec(hour00, true, 1, 0);
  hour01 = floor(Time.hour() % 10);
  segto = 0x80  | display.encodeDigit(hour01);
  display.setSegments(&segto, 1, 1);
  display.showNumberDec(Time.minute(), true, 2, 2);

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
      wLed = LedControl(current_time(), solardata.sunset_time(), midnight);
      bLed = LedControl(current_time(), midnight, solardata.sunrise_time());

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
