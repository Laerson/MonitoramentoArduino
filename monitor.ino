/*Conexão dos módulos no arduino mega 2560 */

/*
 * módulo SD:
 * CS -> 53
 * SCK -> 52
 * MOSI -> 51
 * MISO -> 50
 */

 /*
  * Módulo GPS: 
  * Rx -> 18
  * Tx -> 19
  */

  /*
   * MPU6050 (giroscopio + acelerômetro)
   * SDA -> 20
   * SCL -> 21
   */

// I2C device class (I2Cdev) demonstration Arduino sketch for MPU6050 class
// 10/7/2011 by Jeff Rowberg <jeff@rowberg.net>
// Updates should (hopefully) always be available at https://github.com/jrowberg/i2cdevlib
//
// Changelog:
//      2013-05-08 - added multiple output formats
//                 - added seamless Fastwire support
//      2011-10-07 - initial release

/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2011 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include <MPU6050.h>
#include <SPI.h>
#include <SD.h>
#include <TinyGPS++.h>
#include "I2Cdev.h"


// The TinyGPS++ object
TinyGPSPlus gps;

const int chipSelect = 53;

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

int16_t ax, ay, az;
int16_t gx, gy, gz;



// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
#define OUTPUT_READABLE_ACCELGYRO

// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO


#define LED_PIN 13
bool blinkState = false;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial1.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  //Inicialização do cartão SD
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

  // initialize device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    // use the code below to change accel/gyro offset values
    /*
    Serial.println("Updating internal sensor offsets...");
    // -76  -2359 1688  0 0 0
    Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
    Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
    Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
    Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
    Serial.print("\n");
   
    Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
    Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
    Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
    Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
    Serial.print("\n");
    */

    //           X Accel  Y Accel  Z Accel   X Gyro   Y Gyro   Z Gyro
   //OFFSETS     -422,   -1187,    1699,     326,     -94,     -33
   
    accelgyro.setXAccelOffset(-422);
    accelgyro.setYAccelOffset(-1187);
    accelgyro.setZAccelOffset(1699);
    accelgyro.setXGyroOffset(326);
    accelgyro.setYGyroOffset(-94);
    accelgyro.setZGyroOffset(-33);

    // configure Arduino LED pin for output
    pinMode(LED_PIN, OUTPUT);
  
}

void loop() {
  // String para ser passada para o cartão SD
  String dataString = "";

  // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // these methods (and a few others) are also available
    //accelgyro.getAcceleration(&ax, &ay, &az);
    //accelgyro.getRotation(&gx, &gy, &gz);

    #ifdef OUTPUT_READABLE_ACCELGYRO
        // display tab-separated accel/gyro x/y/z values
        dataString += ("a/g:\t");
        dataString += String(ax / 16384.0); dataString += ("\t");
        dataString += String(ay / 16384.0); dataString += ("\t");
        dataString += String(az / 16384.0); dataString += ("\t");
        dataString += String(gx / 131.0); dataString += ("\t");
        dataString += String(gy / 131.0); dataString += ("\t");
        dataString += String(gz / 131.0);
        dataString += "\n";
    #endif

    #ifdef OUTPUT_BINARY_ACCELGYRO
        Serial.write((uint8_t)(ax >> 8)); Serial.write((uint8_t)(ax & 0xFF));
        Serial.write((uint8_t)(ay >> 8)); Serial.write((uint8_t)(ay & 0xFF));
        Serial.write((uint8_t)(az >> 8)); Serial.write((uint8_t)(az & 0xFF));
        Serial.write((uint8_t)(gx >> 8)); Serial.write((uint8_t)(gx & 0xFF));
        Serial.write((uint8_t)(gy >> 8)); Serial.write((uint8_t)(gy & 0xFF));
        Serial.write((uint8_t)(gz >> 8)); Serial.write((uint8_t)(gz & 0xFF));
    #endif

    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);

  // This sketch displays information every time a new sentence is correctly encoded.
  while (Serial1.available() > 0)
    if (gps.encode(Serial1.read()))
      dataString += displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }

}

String displayInfo()
{
  String dataString = "";
  
  //Serial.print(F("Location: "));
  dataString += (F("Location: "));
  if (gps.location.isValid())
  {
    //Serial.print(gps.location.lat(), 6);
    dataString += String(gps.location.lat(), 6);
    //Serial.print(F(","));
    dataString += (F(","));
    //Serial.print(gps.location.lng(), 6);
    dataString += String(gps.location.lng(), 6);
  }
  else
  {
    //Serial.print(F("INVALID"));
    dataString += (F("INVALID"));
  }

  //Serial.print(F("  Date/Time: "));
  dataString += (F("  Date/Time: "));
  if (gps.date.isValid())
  {
    int hora = gps.time.hour() - 3;
    int dia = gps.date.day();
    int mes = gps.date.month();
    int ano = gps.date.year();
    if (hora < 0) dia -= 1;
    if (dia == 0) {
      
      mes -= 1;
      if (mes == 0){
        ano -= 1;
        mes = 12;
      }
      
      switch (mes) {
        case 2:                    
          if ( ( ano % 4 == 0 && ano % 100 != 0 ) || ano % 400 == 0 ) {
            dia = 29;
          }
          else {
            dia = 28;
          }
          break;

        case 1:
        dia = 31;
        break;
        case 3:
        dia = 31;
        break;
        case 5:
        dia = 31;
        break;
        case 7:
        dia = 31;
        break;
        case 8:
        dia = 31;
        break;
        case 10:
        dia = 31;
        break;
        case 12:
        dia = 31;
        break;
        default:
        dia = 30;
        break;
      }
    }
    //Serial.print(gps.date.day());    
    dataString += String(dia);
    //Serial.print(F("/"));
    dataString += (F("/"));
    //Serial.print(gps.date.month());
    dataString += String(mes);
    //Serial.print(F("/"));
    dataString += (F("/"));
    //Serial.print(gps.date.year());
    dataString += String(ano);
  }
  else
  {
    //Serial.print(F("INVALID"));
    dataString += (F("INVALID"));
  }

  dataString += (F(" "));
  if (gps.time.isValid())
  {
    int hora = gps.time.hour() - 3;
    if (hora < 0) hora += 24;
    if (hora < 10) dataString += (F("0"));
    //Serial.print(gps.time.hour() - 3);
    dataString += String(hora);
    //Serial.print(F(":"));
    dataString += (F(":"));
    if (gps.time.minute() < 10) dataString += (F("0"));
    dataString += String(gps.time.minute());
    dataString += (F(":"));
    if (gps.time.second() < 10) dataString += (F("0"));
    dataString += String(gps.time.second());
    dataString += (F("."));
    if (gps.time.centisecond() < 10) dataString += (F("0"));
    dataString += String(gps.time.centisecond());
  }
  else
  {
    dataString += (F("INVALID"));
  }

  dataString += "\nSpeed(KM/H) : ";
  dataString += String(gps.speed.kmph());
  dataString += "\n";

  dataString += "\n";

  return dataString;
}
