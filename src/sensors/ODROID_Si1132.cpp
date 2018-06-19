#include "ODROID_Si1132.h"
#include "sensors/Wire.h"

ODROID_Si1132::ODROID_Si1132()
{
	_addr = Si1132_ADDR;
}

boolean ODROID_Si1132::begin(void)
{
	//if (read8(Si1132_REG_CHIPID) != 0x60)
	//	return false;
	reset();
        configRegister();
	
	return true;
}

uint16_t ODROID_Si1132::readUV()
{
	delay(10);
	return read16(0x2c);
}

float ODROID_Si1132::readIR()
{
	delay(10);
	return ((read16(0x24) - 250)/2.44)*14.5;
}

float ODROID_Si1132::readVisible()
{
	delay(10);
	return ((read16(0x22) - 256)/0.282)*14.5;
}

void ODROID_Si1132::reset()
{
	write8(Si1132_REG_MEASRATE0, 0);
	write8(Si1132_REG_MEASRATE1, 0);
	write8(Si1132_REG_IRQEN, 0);
	write8(Si1132_REG_IRQMODE1, 0);
	write8(Si1132_REG_IRQMODE2, 0);
	write8(Si1132_REG_INTCFG, 0);
	write8(Si1132_REG_IRQSTAT, 0xFF);

	write8(Si1132_REG_COMMAND, Si1132_RESET);
	delay(10);
	write8(Si1132_REG_HWKEY, 0x17);

	delay(10);
}

void ODROID_Si1132::configRegister()
{
// enable UVindex measurement coefficients!
	write8(Si1132_REG_UCOEF0, 0x7B);
	write8(Si1132_REG_UCOEF1, 0x6B);
	write8(Si1132_REG_UCOEF2, 0x01);
	write8(Si1132_REG_UCOEF3, 0x00);

	// enable UV sensor
	writeParam(Si1132_PARAM_CHLIST, Si1132_PARAM_CHLIST_ENUV |
		Si1132_PARAM_CHLIST_ENALSIR | Si1132_PARAM_CHLIST_ENALSVIS);
	write8(Si1132_REG_INTCFG, Si1132_REG_INTCFG_INTOE);
	write8(Si1132_REG_IRQEN, Si1132_REG_IRQEN_ALSEVERYSAMPLE);

  	writeParam(Si1132_PARAM_ALSIRADCMUX, Si1132_PARAM_ADCMUX_SMALLIR);  
  // fastest clocks, clock div 1
	writeParam(Si1132_PARAM_ALSIRADCGAIN, 0);
  // take 511 clocks to measure
  	writeParam(Si1132_PARAM_ALSIRADCCOUNTER, Si1132_PARAM_ADCCOUNTER_511CLK);
  // in high range mode
	writeParam(Si1132_PARAM_ALSIRADCMISC, Si1132_PARAM_ALSIRADCMISC_RANGE);

  // fastest clocks
	writeParam(Si1132_PARAM_ALSVISADCGAIN, 0);
  // take 511 clocks to measure
  	writeParam(Si1132_PARAM_ALSVISADCCOUNTER, Si1132_PARAM_ADCCOUNTER_511CLK);
  // in high range mode (not normal signal)
	writeParam(Si1132_PARAM_ALSVISADCMISC, Si1132_PARAM_ALSVISADCMISC_VISRANGE);

	write8(Si1132_REG_MEASRATE0, 0xFF);
	write8(Si1132_REG_COMMAND, Si1132_ALS_AUTO);
}

uint8_t ODROID_Si1132::read8(uint8_t reg)
{
	uint16_t val;
	Wire.beginTransmission(_addr);
	Wire.write((uint8_t)reg);
	Wire.endTransmission();

	Wire.requestFrom((uint8_t)_addr, (uint8_t)1);
	return Wire.read();
}

uint16_t ODROID_Si1132::read16(uint8_t reg)
{
	uint16_t ret;

	Wire.beginTransmission(_addr);
	Wire.write(reg);
	Wire.endTransmission();

	Wire.requestFrom(_addr, (uint8_t)2);
	ret = Wire.read();
	ret |= (uint16_t)Wire.read() << 8;

	return ret;
}

void ODROID_Si1132::write8(uint8_t reg, uint8_t val)
{
	Wire.beginTransmission(_addr);
	Wire.write(reg);
	Wire.write(val);
	Wire.endTransmission();
}

uint8_t ODROID_Si1132::writeParam(uint8_t p, uint8_t v)
{
	write8(Si1132_REG_PARAMWR, v);
	write8(Si1132_REG_COMMAND, p | Si1132_PARAM_SET);
	return read8(Si1132_REG_PARAMRD);
}
