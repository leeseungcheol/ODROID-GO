#include "ODROID_Si70xx.h"
#include "sensors/Wire.h"

ODROID_Si70xx::ODROID_Si70xx()
{
	Wire.begin(15, 4);
	//if (read8(Si1132_REG_CHIPID) != 0x60)
	//	return false;
}

float ODROID_Si70xx::readTemperature(void)
{
	float temp;
	int32_t rawTemp;
	write8(CMD_MEASURE_TEMPERATURE_HOLD);
	
	rawTemp = read16(CMD_MEASURE_TEMPERATURE_HOLD);
	temp = (rawTemp*175.72/65536) - 46.85;

	return temp;
}

float ODROID_Si70xx::readHumidity(void)
{
	float humi;
	int32_t rawHumi;
	write8(CMD_MEASURE_HUMIDITY_HOLD);
	delay(10);
	rawHumi = read16(CMD_MEASURE_HUMIDITY_HOLD);
	delay(10);
	humi = (rawHumi*125.0/65536) - 6;

	return humi;
}

uint16_t ODROID_Si70xx::read16(uint8_t addr)
{
	uint16_t ret;
	Wire.beginTransmission(ID_SI7020);
	Wire.write(addr);
	Wire.endTransmission();

	Wire.beginTransmission(ID_SI7020);
	Wire.requestFrom(ID_SI7020, 2);
	ret = Wire.read();
	ret <<= 8;
	ret |= Wire.read();
	Wire.endTransmission();

	return ret;
}

void ODROID_Si70xx::write8(uint8_t addr)
{
	Wire.beginTransmission(ID_SI7020);
	Wire.write(addr);
	Wire.endTransmission();
}
