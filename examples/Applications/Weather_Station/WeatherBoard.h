#include "sensors/Adafruit_BMP085_U.h"
#include "sensors/Adafruit_BME280.h"
#include "sensors/Adafruit_Sensor.h"
#include "sensors/ODROID_Si1132.h"
#include "sensors/ODROID_Si70xx.h"

#include "Arduino.h"

#define SEALEVELPRESSURE_HPA    (1024.25)

class WeatherBoard {
    public:
        Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(SEALEVELPRESSURE_HPA);
        Adafruit_BME280 bme; // I2C
        ODROID_Si70xx si7020;
        ODROID_Si1132 si1132;
        uint8_t revision = 0;
        uint8_t errorState = 0;

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

        uint8_t begin();
        uint8_t read8(byte _i2caddr, byte reg);
        void getBMP180();
        void getBME280();
        void getSi7020();
        void getSi1132();
        void errorCheck();
};
