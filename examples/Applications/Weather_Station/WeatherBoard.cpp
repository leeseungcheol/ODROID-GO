#include "WeatherBoard.h"

uint8_t WeatherBoard::begin(void)
{
    revision = 0;
    // Check board version
    Wire.beginTransmission(96);
    if (Wire.endTransmission() == 0) {
        revision = 2;
        bme.begin();
        si1132.begin();
    }

    //Wire.beginTransmission((byte)BMP085_REGISTER_CHIPID);
    Wire.beginTransmission(119);
    if (Wire.endTransmission() == 0) {
        revision = 1;
        bmp.begin();
        sensor_t sensor;
        bmp.getSensor(&sensor);
        si1132.begin();
    }
    return revision;
}

uint8_t WeatherBoard::read8(byte _i2caddr, byte reg)
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
void WeatherBoard::getBMP180()
{
    sensors_event_t event;
    bmp.getEvent(&event);

    if (event.pressure) {
        bmp.getTemperature(&BMP180Temperature);
        BMP180Pressure = event.pressure;
        BMP180Altitude = bmp.pressureToAltitude(1025, event.pressure);
    }
}

void WeatherBoard::getSi1132()
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

void WeatherBoard::getSi7020()
{
    Si7020Temperature = si7020.readTemperature();
    Si7020Humidity = si7020.readHumidity();
}

void WeatherBoard::getBME280(void)
{
    BME280Temperature = bme.readTemperature();
    BME280Pressure = (bme.readPressure() / 100);
    BME280Humidity = bme.readHumidity();
    BME280Altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
}

void WeatherBoard::errorCheck()
{
    if ((read8(BME280_ADDRESS, BME280_REGISTER_CHIPID) != 0x60) &&
            (read8(BMP085_ADDRESS, BMP085_REGISTER_CHIPID) != 0x55)) {
        if (!errorState) {
//            GO.lcd.fillScreen(backgroundColor);
//            displayLabel();
        }
        errorState = 1;
    } else if (errorState) {
        if (read8(BME280_ADDRESS, BME280_REGISTER_CHIPID) != 0x60) {
            revision = 1;
        } else {
            revision = 2;
        }
        //si1132.begin();

        if (revision == 1) {
            bmp.begin();
            sensor_t sensor;
            bmp.getSensor(&sensor);
        } else {
            if (!bme.begin()) {
            }
        }
        //GO.lcd.fillScreen(backgroundColor);
//        displayLabel();
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

