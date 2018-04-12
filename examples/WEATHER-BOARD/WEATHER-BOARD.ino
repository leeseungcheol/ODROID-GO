#include <odroid_go.h>
//#include "TimerOne.h"
//#include <Adafruit_GFX.h>
//#include <ODROID_Si1132.h>
//#include <ODROID_Si70xx.h>
//#include <Adafruit_TFT.h>
//#include <Adafruit_BMP085_U.h>
//#include <Adafruit_Sensor.h>
//#include <Adafruit_BME280.h>

// These are the pins used for the UNO
// for Due/Mega/Leonardo use the hardware SPI pins (which are different)
#define _sclk 13
#define _miso 12
#define _mosi 11
#define _cs 10
#define _dc 9
#define _rst 8

const char version[] = "v1.5";

uint8_t ledPin = 5;
uint8_t pwm = 255;
uint8_t textSize = 2;
uint8_t rotation = 1;

float battery = 0;
float oldBattery;
uint8_t batteryCnt;
uint8_t batteryState;
uint8_t timer;
uint8_t count;

uint16_t foregroundColor, backgroundColor;

#define SEALEVELPRESSURE_HPA        (1024.25)

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(SEALEVELPRESSURE_HPA);
Adafruit_BME280 bme; // I2C
ODROID_Si70xx si7020;
ODROID_Si1132 si1132;


float BMP180Temperature = 0;
float BMP180Pressure = 0;
float BMP180Altitude = 0;

float BME280Temperature = 0;
float BME280Pressure = 0;
float BME280Humidity = 0;
float BME280Altitude = 0;

float Si7020Temperature = 0;
float Si7020Humidity = 0;

float Si1132UVIndex = 0;
uint32_t Si1132Visible = 0;
uint32_t Si1132IR = 0;

uint8_t WB_VERSION = 2;
unsigned char errorState;

void setup()
{

        GO.begin();
        si1132.begin();
        GO.lcd.setTextFont(4);
        GO.lcd.print("312413213");
        
        Wire.begin(15, 4);
        
        // Check board version
        if (read8(BME280_ADDRESS, BME280_REGISTER_CHIPID) != 0x60)
                WB_VERSION = 1;

        if (WB_VERSION == 1) {
                bmp.begin();
                sensor_t sensor;
                bmp.getSensor(&sensor);
        } else {
                if(!bme.begin()) {
                }
        }
        // initialize the digital pins
/*
	analogReference(INTERNAL);
*/

	//displayLabel();
 
}

uint8_t read8(byte _i2caddr, byte reg)
{
	byte value;
	Wire.beginTransmission((uint8_t)_i2caddr);
	Wire.write((uint8_t)reg);
	Wire.endTransmission();
	Wire.requestFrom((uint8_t)_i2caddr, (byte)1);
	value = Wire.read();
	Wire.endTransmission();
	return value;
}

void displayLabel()
{
  
	GO.lcd.setCharCursor(0, 1);
	GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
	GO.lcd.println("Si1132");
	GO.lcd.setTextColor(TFT_MAGENTA, backgroundColor);
	GO.lcd.println("UV Index : ");
	GO.lcd.println("Visible :");
	GO.lcd.println("IR :");

	if (WB_VERSION == 1) {
		GO.lcd.setCharCursor(0, 6);
		GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
		GO.lcd.println("Si7020");
		GO.lcd.setTextColor(TFT_MAGENTA, backgroundColor);
		GO.lcd.println("Temp : ");
		GO.lcd.println("Humidity :");
    GO.lcd.setCharCursor(0, 10);
		GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
		GO.lcd.println("BMP180");
		GO.lcd.setTextColor(TFT_MAGENTA, backgroundColor);
		GO.lcd.println("Temp : ");
		GO.lcd.println("Pressure :");
		GO.lcd.println("Altitude :");
	} else {
    		GO.lcd.setCharCursor(0, 6);
		GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
		GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
		GO.lcd.println("BME280");
		GO.lcd.setTextColor(TFT_MAGENTA, backgroundColor);
		GO.lcd.println("Temp : ");
		GO.lcd.println("Pressure :");
		GO.lcd.println("Humidity :");
		GO.lcd.println("Altitude :");
	}
	GO.lcd.drawRect(240, 10, 70, 30, 870);
 
}

void sendToHost()
{
  /*
	Serial.print("w0");
	Serial.print(battery);
	Serial.print("\ew2");
	Serial.print(Si1132UVIndex);
	Serial.print("\ew3");
	Serial.print(Si1132Visible);
	Serial.print("\ew4");
	Serial.print(Si1132IR);
	Serial.print("\ew5");
	if (WB_VERSION == 1) {
		Serial.print(Si7020Temperature);
		Serial.print("\ew6");
		Serial.print(Si7020Humidity);
		Serial.print("\ew7");
		Serial.print(BMP180Pressure);
		Serial.print("\ew8");
		Serial.print(BMP180Altitude);
	} else {
		Serial.print(BME280Temperature);
		Serial.print("\ew6");
		Serial.print(BME280Humidity);
		Serial.print("\ew7");
		Serial.print(BME280Pressure);
		Serial.print("\ew8");
		Serial.print(BME280Altitude);
	}

	Serial.print("\e");
 */
}

void getBMP180()
{
  
        sensors_event_t event;
        bmp.getEvent(&event);
        
        if (event.pressure) {
                bmp.getTemperature(&BMP180Temperature);
                BMP180Pressure = event.pressure;
                BMP180Altitude = bmp.pressureToAltitude(1025, event.pressure);
        }
        
}

void getSi1132()
{
  
        Si1132UVIndex = 0;
        Si1132Visible = 0;
        Si1132IR = 0;
        for (int i = 0; i < 10; i++) {
                Si1132Visible += si1132.readVisible();
                Si1132IR += si1132.readIR();
                Si1132UVIndex += si1132.readUV();
        }
        Si1132UVIndex /= 10;
        Si1132UVIndex /= 100;
        Si1132Visible /= 10;
        Si1132IR /= 10;
        
}

void getSi7020()
{
  
        Si7020Temperature = si7020.readTemperature();
        Si7020Humidity = si7020.readHumidity();
        
}

void getBME280()
{
  
	BME280Temperature = bme.readTemperature();
	BME280Pressure = (bme.readPressure()/100);
	BME280Humidity = bme.readHumidity();
	BME280Altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  
}

void displayBMP180()
{
  
        GO.lcd.setTextColor(TFT_CYAN, backgroundColor);
	GO.lcd.setCharCursor(7, 11);
        GO.lcd.print(BMP180Temperature);
        GO.lcd.println(" *C  ");
        delay(20);

	GO.lcd.setCharCursor(11, 12);
        GO.lcd.print(BMP180Pressure);
        GO.lcd.println(" Pa  ");
        delay(20);

	GO.lcd.setCharCursor(11, 13);
        GO.lcd.print(BMP180Altitude);
        GO.lcd.println(" meters   ");
        delay(20);
        
}

void displaySi7020()
{
  
        GO.lcd.setTextColor(TFT_YELLOW, backgroundColor);
	GO.lcd.setCharCursor(7, 7);
        GO.lcd.print(Si7020Temperature);
        GO.lcd.println(" *C  ");
        delay(20);

	GO.lcd.setCharCursor(11, 8);
        GO.lcd.print(Si7020Humidity);
        GO.lcd.println(" %  \n");
        delay(20);
        
}

void displaySi1132()
{
  GO.lcd.print("123213213");
  /*
        GO.lcd.setTextColor(TFT_RED, backgroundColor);
	GO.lcd.setCharCursor(11, 2);
        GO.lcd.drawFloat(Si1132UVIndex, 35, 50, 4);
        GO.lcd.println("  ");
        delay(20);

	GO.lcd.setCharCursor(10, 3);
        GO.lcd.print(Si1132Visible);
        GO.lcd.println(" Lux  ");
        delay(20);

	GO.lcd.setCharCursor(5, 4);
        GO.lcd.print(Si1132IR);
        GO.lcd.println(" Lux  \n");
        delay(20);
     */   
}

void displayBME280()
{
  
        GO.lcd.setTextColor(TFT_CYAN, backgroundColor);
	GO.lcd.setCharCursor(7, 7);
        GO.lcd.print(BME280Temperature);
        GO.lcd.println(" *C  ");
        delay(20);

	GO.lcd.setCharCursor(11, 8);
        GO.lcd.print(BME280Pressure);
        GO.lcd.println(" hPa  ");
        delay(20);

	GO.lcd.setCharCursor(11, 9);
        GO.lcd.print(BME280Humidity);
        GO.lcd.println(" %   ");
        delay(20);

        GO.lcd.setCharCursor(11, 10);
        GO.lcd.print(BME280Altitude);
        GO.lcd.println(" m   ");
        delay(20);
        
}

void errorCheck()
{
  
        if ((read8(BME280_ADDRESS, BME280_REGISTER_CHIPID) != 0x60) &&
                        (read8(BMP085_ADDRESS, BMP085_REGISTER_CHIPID) != 0x55)) {
                if (!errorState) {
                        GO.lcd.fillScreen(backgroundColor);
			displayLabel();
                }
                errorState = 1;
        } else if (errorState) {
                if (read8(BME280_ADDRESS, BME280_REGISTER_CHIPID) != 0x60) {
                        WB_VERSION = 1;
                } else {
                        WB_VERSION = 2;
                }
                si1132.begin();
                if (WB_VERSION == 1) {
                        bmp.begin();
                        sensor_t sensor;
                        bmp.getSensor(&sensor);
                } else {
                        if(!bme.begin()) {
                        }
                }
                GO.lcd.fillScreen(backgroundColor);
		displayLabel();
                errorState = 0;
        }
        if (errorState) {
                BMP180Temperature = 0;
                BMP180Pressure = 0;
                BMP180Altitude = 0;

                Si7020Temperature = 0;
                Si7020Humidity = 0;

                BME280Temperature = 0;
                BME280Pressure = 0;
                BME280Humidity = 0;
                BME280Altitude = 0;

                Si1132UVIndex = 0;
                Si1132Visible = 0;
                Si1132IR = 0;
        }
        
}

void readBattery()
{
  /*
        battery = analogRead(A2)*1.094/1024/3.9*15.9;

        if (abs(oldBattery - battery) > 0.5 || battery < 2.1)
                batteryCnt++;
        oldBattery = battery;

        if (timer > 10) {
                if (batteryCnt > 2) {
                        timer = 0;
                        batteryCnt = 0;
                        batteryState = 1;
                } else {
                        batteryState = 0;
                }
        }

        if (batteryState)
                battery = 0;

	GO.lcd.setTextColor(TFT_GREEN, backgroundColor);
        GO.lcd.setCharCursor(21, 3);
        GO.lcd.print(battery);
        if (battery > 3.95) {
                        GO.lcd.fillRect(244, 13, 14, 24, 10000);
                        GO.lcd.fillRect(260, 13, 14, 24, 10000);
                        GO.lcd.fillRect(276, 13, 14, 24, 10000);
                        GO.lcd.fillRect(292, 13, 14, 24, 10000);
        } else if (battery > 3.75 && battery <= 3.95) {
                        GO.lcd.fillRect(244, 13, 14, 24, 10000);
                        GO.lcd.fillRect(260, 13, 14, 24, 10000);
                        GO.lcd.fillRect(276, 13, 14, 24, 10000);
                        GO.lcd.fillRect(292, 13, 14, 24, 0);
        } else if (battery > 3.65 && battery <= 3.75) {
                        GO.lcd.fillRect(244, 13, 14, 24, 10000);
                        GO.lcd.fillRect(260, 13, 14, 24, 10000);
                        GO.lcd.fillRect(276, 13, 14, 24, 0);
                        GO.lcd.fillRect(292, 13, 14, 24, 0);
        } else if (battery > 3.5 && battery <= 3.65) {
                        GO.lcd.fillRect(244, 13, 14, 24, 10000);
                        GO.lcd.fillRect(260, 13, 14, 24, 0);
                        GO.lcd.fillRect(276, 13, 14, 24, 0);
                        GO.lcd.fillRect(292, 13, 14, 24, 0);
        } else if (battery <= 3.5) {
                        GO.lcd.fillRect(244, 13, 14, 24, 0);
                        GO.lcd.fillRect(260, 13, 14, 24, 0);
                        GO.lcd.fillRect(276, 13, 14, 24, 0);
                        GO.lcd.fillRect(292, 13, 14, 24, 0);
        }
        */
}

void loop(void)
{
  //displaySi1132();
  delay(1000);
  /*
  GO.update();
  
	displaySi1132();
	if (WB_VERSION == 1) {
		displayBMP180();
                displaySi7020();
        } else {
                displayBME280();
        }

        if (!errorState) {
                getSi1132();
                if (WB_VERSION == 1) {
                        getBMP180();
                        getSi7020();
                } else {
                        getBME280();
                }

		sendToHost();
        }

        if (count > 5) {
                errorCheck();
                count = 0;
        }
        //readBattery();

        count++;
    */
        
}
