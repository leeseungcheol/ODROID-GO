#ifndef ODROID_Si70xx_H
#define ODROID_Si70xx_H

#if (ARDUINO >= 100)
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

/* Commands */
#define CMD_MEASURE_HUMIDITY_HOLD	0xE5
#define CMD_MEASURE_HUMIDITY_NO_HOLD	0xF5
#define CMD_MEASURE_TEMPERATURE_HOLD	0xE3
#define CMD_MEASURE_TEMPERATURE_NO_HOLD 0xF3
#define CMD_MEASURE_THERMISTOR_HOLD	0xEE
#define CMD_READ_PREVIOUS_TEMPERATURE	0xE0
#define CMD_RESET			0xFE
#define CMD_WRITE_REGISTER_1		0xE6
#define CMD_READ_REGISTER_1		0xE7
#define CMD_WRITE_REGISTER_2		0x50
#define CMD_READ_REGISTER_2		0x10
#define CMD_WRITE_REGISTER_3		0x51
#define CMD_READ_REGISTER_3		0x11
#define CMD_WRITE_COEFFICIENT		0xC5
#define CMD_READ_COEFFICIENT		0x84

/* User Register 1 */
#define REG1_RESOLUTION_MASK		0x81
#define REG1_RESOLUTION_H12_T14 	0x00
#define REG1_RESOLUTION_H08_T12 	0x01
#define REG1_RESOLUTION_H10_T13 	0x80
#define REG1_RESOLUTION_H11_T11 	0x81
#define REG1_LOW_VOLTAGE		0x40
#define REG1_ENABLE_HEATER		0x04

/* User Register 2 */
#define REG2_VOUT			0x01
#define REG2_VREF_VDD			0x02
#define REG2_VIN_BUFFERED		0x04
#define REG2_RESERVED			0x08
#define REG2_FAST_CONVERSION		0x10
#define REG2_MODE_CORRECTION		0x20
#define REG2_MODE_NO_HOLD		0x40

/* Device Identification */
#define ID_SAMPLE			0xFF
#define ID_SI7013			0x0D
#define ID_SI7020			0x40
#define ID_SI7021			0x15

/* Coefficients */
#define COEFFICIENT_BASE		0x82
#define COEFFICIENT_COUNT		9

class ODROID_Si70xx {
	public :
		ODROID_Si70xx();
		float readTemperature(void);
		float readHumidity(void);
	private :
		uint8_t read8(uint8_t addr);
		uint16_t read16(uint8_t addr);
		void write8(uint8_t addr);
};

#endif
