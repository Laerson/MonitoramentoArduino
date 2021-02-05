//******************//
//Biblioteca do GPS//
//****************//
//************************************************************//
//Baixa no link(http://arduiniana.org/libraries/tinygpsplus/)//
//Sketch -> Incluir Biblioteca -> Adicionar biblioteca .ZIP //   
//*********************************************************//

#include <TinyGPS++.h>

//********************************************************//
//Bibliotecas do acelerômetro e da interface do cartão SD//
// Necessário Importar (Ctrl+shift+I)                   //
//*****************************************************//
#include <MPU6050.h>
#include <SD.h>

//*******************************//
//Bibliotecas já inclusas no IDE//
//*****************************//

#include <SPI.h>
#include <Wire.h>

//Biblioteca para calibrar o acelerometro//
//#include "I2Cdev.h"

//************************//
//                       //
//**********************//
TinyGPSPlus gps;

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



File data_file;
int CS_pin = 53; //Pino ligado ao CS do adaptador do cartao SD. Alterar caso esteja conectado à outro pino //
bool acelCheck = false;

void setup() {
  Wire.begin();
  Serial1.begin(9600);
  Serial.begin(9600); //Setting baudrate at 9600
  pinMode(CS_pin, OUTPUT);//declaring CS pin as output pin

//----------------------------------------------------------------------------//
//-------------------------Inicializaçao do Cartao de memoria----------------//
//--------------------------------------------------------------------------//
  if (SD.begin()) {
    Serial.println("SD card is initialized and it is ready to use");
    } 
    else {
      Serial.println("SD card is not initialized");
      return;
      }


//------------------------------------------------------------------------//
//--------------------Inicializaçao do acelerometro/giroscopio-----------//
//----------------------------------------------------------------------//

    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");


//O codigo abaixo serve para mudar a precisao dos sensores
//e necessario achar os "offsets" antes, com outro programa .ino
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
    accelgyro.setXGyroOffset(220);
    accelgyro.setYGyroOffset(76);
    accelgyro.setZGyroOffset(-85);
    Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
    Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
    Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
    Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
    Serial.print("\n");
    */

}



void loop() {

//--------------------Codigo do Acelerometro/Giroscopio--------------------//
  // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // these methods (and a few others) are also available
    //accelgyro.getAcceleration(&ax, &ay, &az);
    //accelgyro.getRotation(&gx, &gy, &gz);

    #ifdef OUTPUT_READABLE_ACCELGYRO
        // display tab-separated accel/gyro x/y/z values
        Serial.print("a/g:\t");
        Serial.print(ax/16384); Serial.print("\t");
        Serial.print(ay/16384); Serial.print("\t");
        Serial.print(az/16384); Serial.print("\t");
        Serial.print(gx/131); Serial.print("\t");
        Serial.print(gy/131); Serial.print("\t");
        Serial.println(gz/131);
    #endif

    #ifdef OUTPUT_BINARY_ACCELGYRO
        Serial.write((uint8_t)(ax >> 8)); Serial.write((uint8_t)(ax & 0xFF));
        Serial.write((uint8_t)(ay >> 8)); Serial.write((uint8_t)(ay & 0xFF));
        Serial.write((uint8_t)(az >> 8)); Serial.write((uint8_t)(az & 0xFF));
        Serial.write((uint8_t)(gx >> 8)); Serial.write((uint8_t)(gx & 0xFF));
        Serial.write((uint8_t)(gy >> 8)); Serial.write((uint8_t)(gy & 0xFF));
        Serial.write((uint8_t)(gz >> 8)); Serial.write((uint8_t)(gz & 0xFF));
    #endif


//Codigo do GPS//

  // Displays information when new sentence is available.
  while (Serial1.available() > 0)
    if (gps.encode(Serial1.read()))
      displayInfo();


    if(millis() > 5000 && gps.charsProcessed() < 10) {
      Serial.println("No GPS detected");
      while(true);
    }
}


void displayInfo()
{

  if (gps.location.isValid())
  {
    Serial.print("LAT="); Serial.println(gps.location.lat(), 6); // Latitude in degrees (double)
    Serial.print("LNG="); Serial.println(gps.location.lng(), 6); // Longitude in degrees (double)
    Serial.print(gps.location.rawLat().negative ? "-" : "+");
    Serial.println(gps.location.rawLat().deg); // Raw latitude in whole degrees
    Serial.println(gps.location.rawLat().billionths);// ... and billionths (u16/u32)
    Serial.print(gps.location.rawLng().negative ? "-" : "+");
    Serial.println(gps.location.rawLng().deg); // Raw longitude in whole degrees
    Serial.println(gps.location.rawLng().billionths);// ... and billionths (u16/u32)
    Serial.print("DATA="); Serial.println(gps.date.value()); // Raw date in DDMMYY format (u32)
    Serial.print("ANO="); Serial.println(gps.date.year()); // Year (2000+) (u16)
    Serial.print("Mes="); Serial.println(gps.date.month()); // Month (1-12) (u8)
    Serial.println(gps.date.day()); // Day (1-31) (u8)
    Serial.print("Hora (UTC)="); Serial.println(gps.time.value()); // Raw time in HHMMSSCC format (u32)
    Serial.print("Hora (Horario de Brasilia)="); Serial.println(gps.time.value() - 3000000); // Raw time in HHMMSSCC format (u32)
    Serial.println(gps.time.hour()); // Hour (0-23) (u8)
    Serial.println(gps.time.minute()); // Minute (0-59) (u8)
    Serial.println(gps.time.second()); // Second (0-59) (u8)
    Serial.println(gps.time.centisecond()); // 100ths of a second (0-99) (u8)
    Serial.println(gps.speed.value()); // Raw speed in 100ths of a knot (i32)
    Serial.println(gps.speed.knots()); // Speed in knots (double)
    Serial.println(gps.speed.mph()); // Speed in miles per hour (double)
    Serial.println(gps.speed.mps()); // Speed in meters per second (double)
    Serial.println(gps.speed.kmph()); // Speed in kilometers per hour (double)
    Serial.println(gps.course.value()); // Raw course in 100ths of a degree (i32)
    Serial.println(gps.course.deg()); // Course in degrees (double)
    Serial.println(gps.altitude.value()); // Raw altitude in centimeters (i32)
    Serial.println(gps.altitude.meters()); // Altitude in meters (double)
    Serial.println(gps.altitude.miles()); // Altitude in miles (double)
    Serial.println(gps.altitude.kilometers()); // Altitude in kilometers (double)
    Serial.println(gps.altitude.feet()); // Altitude in feet (double)
    Serial.println(gps.satellites.value()); // Number of satellites in use (u32)
    Serial.println(gps.hdop.value()); // Horizontal Dim. of Precision (100ths-i32)
  }
  
  else
  { 
    Serial.print(F("INVALID"));
  }
  
  
  Serial.println();
  
  data_file = SD.open("data.txt", FILE_WRITE);
  
  if(data_file)
  {
      getData();
  }
  
  else
  {
    Serial.println("error opening data.txt");
  }
  
  delay(1000); //intervalo do loop em microsegundos
}


//------------------Funçoes auxiliares------------------------//
void getData() {
  data_file.print("teste\n");
  data_file.flush();
  data_file.close();
}
