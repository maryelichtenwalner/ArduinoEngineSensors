/*
 * Combined Program: Takes measurements from each sensor.
 *      Includes: 
 *      two pressure sensors
 *      three thermocouples with MAX31855 thermocouple amplifier breakout boards
 *      one AHT20 temperature/humidity sensor
 *      one EK1245 Gikfun obstacle avoidance infrared sensor module for shaft speed measurement
 * 
 * Written by Mary Lichtenwalner
 * 
 * Last Update: April 5, 2022
 */

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Section 0. Arduino Uno connections
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 /*
  * All sensors
  *   Connect VCC to 5V, GND to ground
  *   
  * MAX31855 Thermocouple amplifier breakout board
  *   Thermocouple 1: Compressed air, DO to pin 3, CS to pin 4, CLK to pin 5
  *   Thermocouple 2: Chamber, DO to pin 3, CS to pin 6, CLK to pin 5
  *   Thermocouple 3: Exhaust, DO to pin 3, CS to pin 7, CLK to pin 5
  *   
  * Pressure transducers
  *   Transducer 1: Ambient air, red to VCC, black to GND, yellow to pin A1
  *   Transducer 2: Compressed air, red to VCC, black to GND, yellow to pin A2
  *   
  * AHT20 temperature/humidity sensor
  *   SDA to SDA, SCL to SCL
  *   
  * Gikfun infrared sensor module
  *   red to VCC, black to GND, yellow to pin 2
  */

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Section 1. Set up sensors
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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

Adafruit_MAX31855 thermocouple_2(MAXCLK, MAXCS2, MAXDO);

// Third thermocouple (reuse DO and CLK)
#define MAXCS3  7

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

// Initialize sensor
Adafruit_AHTX0 aht;

//***********************************************************************
// Set up IR sensor
//***********************************************************************

// Variables used in rpm calculation
float value=0;
float rev=0;
int rpm;
int oldtime=0;
int time;

void isr() //interrupt service routine
{
rev++;
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Section 2. Set up program
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void setup() {
  
  // Designate two analog pins as input for the pressure transducers
  pinMode(pressurePin1, INPUT);
  pinMode(pressurePin2, INPUT);

  // Infrared sensor goes in digital pin 2
  attachInterrupt(0,isr,RISING);  //attaching the interrupt
  
  // Initialize baud rate
  Serial.begin(9600);

  delay(1000);

  // Initialize thermocouples
  thermocouple_1.begin();
  thermocouple_2.begin();
  thermocouple_3.begin();

  // Initialize temperature/humidity sensor
  aht.begin();
  delay(10);

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Section 3. Part that runs on a loop
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void loop() {

  //*********************************************************************
  // Goes through and takes a reading from each sensor
  //*********************************************************************
  
  // Read thermocouples in Fahrenheit
  String temp1 = String(thermocouple_1.readFahrenheit());
  String temp2 = String(thermocouple_2.readFahrenheit());
  String temp3 = String(thermocouple_3.readFahrenheit());

  // Read pressure transducers
  pressureReading1 = analogRead(pressurePin1);
  pressureReading2 = analogRead(pressurePin2);

  // Convert pressures to proper units, want output in psi
  // Pressure transducer 1
  float voltage1 = (pressureReading1*5.0)/1023.0;
  float calibration1 = 0.17;                                                            // Calibrate here. Adjust so that ambient pressure is correct.
  float pressure_pascal1 = (3.0*(voltage1-calibration1))*1000000.0; 
  float pressure_bar1 = pressure_pascal1/10e5;
  float pressure_psi1 = pressure_bar1*14.5038;

  // Pressure transducer 2
  float voltage2 = (pressureReading2*5.0)/1023.0;
  float calibration2 = 0.17;                                                             // Calibrate here. Adjust so that ambient pressure is correct.
  float pressure_pascal2 = (3.0*(voltage2-calibration2))*1000000.0;
  float pressure_bar2 = pressure_pascal2/10e5;
  float pressure_psi2 = pressure_bar2*14.5038;

  // Final output from both pressure transducers
  String pressure1 = String(pressure_psi1);
  String pressure2 = String(pressure_psi2);

  // Read temperature/humidity sensor
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);                                                       // Populate temp and humidity objects with fresh data

  // Read infrared shaft speed sensor
  detachInterrupt(0);           //detaches the interrupt
  time=millis()-oldtime;        //finds the time 
  rpm=(rev/time)*60000;         //calculates rpm
  oldtime=millis();             //saves the current time
  rev=0;

  //*********************************************************************
  // Send the output to the serial port
  //*********************************************************************

  // Combine all measurements into one string with spaces in between values
  String output = temp1 + " " + temp2 + " " + temp3 + " " + pressure1 + " " + pressure2 + " " + temp.temperature + " " + humidity.relative_humidity + " " + rpm;

  // Output single string containing each measurement
  Serial.println(output);

  // Uncomment following section to see error codes if thermocouples are reading NAN
//  Serial.print(thermocouple_1.readError());
//  Serial.print(thermocouple_2.readError());
//  Serial.print(thermocouple_3.readError());
//  Serial.print('\n');

  //*********************************************************************
  // Delay before next reading and restart rpm interrupt
  //*********************************************************************
  
  // Delay one second before next reading
  delay(1000);
  attachInterrupt(0,isr,RISING);

}
