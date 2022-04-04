/*
 * Combined Program: Measures pressure, temperature, humidity
 * 
 * Written by Mary Lichtenwalner
 * 
 * March 22, 2022
 */

// Libraries to include
#include <SPI.h>
#include "Adafruit_MAX31855.h"
#include <Adafruit_AHTX0.h>

//***********************************************************************
// Set up thermocouples
//***********************************************************************

// First thermocouple
#define MAXDO   3
#define MAXCS1  4
#define MAXCLK  5

Adafruit_MAX31855 thermocouple_1(MAXCLK, MAXCS1, MAXDO);

// Second thermocouple (reuse DO and CLK)
#define MAXCS2  6
//#define MAXDO2  2
//#define MAXCLK2 9

Adafruit_MAX31855 thermocouple_2(MAXCLK, MAXCS2, MAXDO);

// Third thermocouple (reuse DO and CLK)
#define MAXCS3  7
//#define MAXDO3  8
//#define MAXCLK3 10

Adafruit_MAX31855 thermocouple_3(MAXCLK, MAXCS3, MAXDO);

//***********************************************************************
// Set up pressure transducers
//***********************************************************************

// Constants. Set pin numbers for pressure readings.
const int pressurePin1 = A1;
const int pressurePin2 = A2;

// Variables
double pressureReading1 = 0.0;
double pressureReading2 = 0.0;

//***********************************************************************
// Set up temperature/humidity sensor
//***********************************************************************

Adafruit_AHTX0 aht;

//***********************************************************************
// Set up IR sensor
//***********************************************************************
float value=0;
float rev=0;
int rpm;
int oldtime=0;
int time;

void isr() //interrupt service routine
{
rev++;
}

//***********************************************************************
// Set up program
//***********************************************************************
void setup() {
  // Set the pressure pins as input pins
  pinMode(pressurePin1, INPUT);
  pinMode(pressurePin2, INPUT);

  // Infrared sensor goes in digital pin 2
  attachInterrupt(0,isr,RISING);  //attaching the interrupt
  //pinMode(2, INPUT);
  
  // Initialize baud rate
  Serial.begin(9600);

  delay(1000);

  // Initialize thermocouples
  thermocouple_1.begin();
  thermocouple_2.begin();
  thermocouple_3.begin();

  // Initialize temperature/humidity sensor
//  if (! aht.begin()) {
//    Serial.println("Could not find AHT? Check wiring");
//    while (1) delay(10);
//  }
//  Serial.println("AHT10 or AHT20 found");

  aht.begin();
  delay(10);

}

//***********************************************************************
// Part to run on loop
//***********************************************************************
void loop() {
  // Read thermocouples in Fahrenheit
  String temp1 = String(thermocouple_1.readFahrenheit());
  String temp2 = String(thermocouple_2.readFahrenheit());
  String temp3 = String(thermocouple_3.readFahrenheit());

  // Read pressure transducers
  pressureReading1 = analogRead(pressurePin1);
  pressureReading2 = analogRead(pressurePin2);

  // Convert pressures to proper units
  float voltage1 = (pressureReading1*5.0)/1023.0;   
  //float pressure_pascal1 = (3.0*(voltage1-0.115))*1000000.0;                       //calibrate here
  float pressure_pascal1 = (3.0*(voltage1-0.135))*1000000.0;                       //calibrate here
  float pressure_bar1 = pressure_pascal1/10e5;
  float pressure_psi1 = pressure_bar1*14.5038;

  float voltage2 = (pressureReading2*5.0)/1023.0;   
  //float pressure_pascal2 = (3.0*(voltage2-0.13))*1000000.0;                       //calibrate here
  float pressure_pascal2 = (3.0*(voltage2-0.14))*1000000.0;                       //calibrate here
  float pressure_bar2 = pressure_pascal2/10e5;
  float pressure_psi2 = pressure_bar2*14.5038;

  String pressure1 = String(pressure_psi1);
  String pressure2 = String(pressure_psi2);

  // Read temperature/humidity sensor
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // Populate temp and humidity objects with fresh data

  // Read infrared shaft speed sensor
  detachInterrupt(0);           //detaches the interrupt
  time=millis()-oldtime;        //finds the time 
  rpm=(rev/time)*60000;         //calculates rpm
  oldtime=millis();             //saves the current time
  rev=0;

  // Output all values to serial monitor
  String output = temp1 + " " + temp2 + " " + temp3 + " " + pressure1 + " " + pressure2 + " " + temp.temperature + " " + humidity.relative_humidity + " " + rpm;
  //String output = temp1 + " " + temp2 + " " + temp3 + " " + pressure1 + " " + pressure2 + " " + "0" + " " + "0" + " " + rpm;

  Serial.println(output);
//  Serial.print(thermocouple_1.readError());
//  Serial.print(thermocouple_2.readError());
//  Serial.print(thermocouple_3.readError());
//  Serial.print('\n');
  
  // Delay one second before next reading
  delay(1000);
  attachInterrupt(0,isr,RISING);

}
