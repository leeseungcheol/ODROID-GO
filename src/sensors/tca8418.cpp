/*
* Arduino Library for TCA8418 I2C keyboard
*
* Copyright (C) 2015 RF Designs. All rights reserved.
*
* Author: Bob Frady <rfdesigns@live.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public
* License v2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 021110-1307, USA.
*
* If you can't comply with GPLv2, alternative licensing terms may be
* arranged. Please contact RF Designs <rfdesigns@live.com> for proprietary
* alternative licensing inquiries.
*/


//- #include <tca8418.h>
//- #include <Wire.h>
#include "tca8418.h"
#include "sensors/Wire.h"

#ifdef TCA8418_INTERRUPT_SUPPORT
#include "PCint.h"
#endif


KEYS::KEYS() :
	_address(0)
#ifdef TCA8418_INTERRUPT_SUPPORT
	, _oldPIN(0), _isrIgnore(0), _pcintPin(0), _intMode(), _intCallback()
#endif
{
}

void KEYS::begin(void)
{
  _address = 0x34;
  Wire.begin();
}

void KEYS::begin(uint8_t rows, uint16_t cols, uint8_t config)
{
  _address = 0x34;
  Wire.begin();
  configureKeys(rows, cols, config);
}

uint8_t KEYS::readKeypad(void)
{
  return(getKeyEvent());
}


/*
* Configure the TCA8418 for keypad operation
*/
bool KEYS::configureKeys(uint8_t rows, uint16_t cols, uint8_t config)
{
  //Pins all default to GPIO. pinMode(x, KEYPAD); may be used for individual pins.
  writeByte(rows, REG_KP_GPIO1);
  
  uint8_t col_tmp;
  col_tmp = (uint8_t)(0xff & cols);
  writeByte(col_tmp, REG_KP_GPIO2);
  col_tmp = (uint8_t)(0x03 & (cols>>8));
  writeByte(col_tmp, REG_KP_GPIO3);

  config |= CFG_AI;
  writeByte(config, REG_CFG);

  return true;
}

void KEYS::writeByte(uint8_t data, uint8_t reg) {
  Wire.beginTransmission(_address);
  I2CWRITE((uint8_t) reg);
  
  I2CWRITE((uint8_t) data);
  Wire.endTransmission();

  return;
}

bool KEYS::readByte(uint8_t *data, uint8_t reg) {
  Wire.beginTransmission(_address);
  I2CWRITE((uint8_t) reg);
  Wire.endTransmission();
  uint8_t timeout=0;
  
  Wire.requestFrom(_address, (uint8_t) 0x01);
  while(Wire.available() < 1) {
    timeout++;
	if(timeout > I2CTIMEOUT) {
	  return(true);
	}
	delay(1);
  } 			// Experimental
  
  *data = I2CREAD();

return(false);
}

void KEYS::write3Bytes(uint32_t data, uint8_t reg) {

  union
  {
    uint8_t b[4];
    uint32_t w;
  } datau;
  
  datau.w = data;

  Wire.beginTransmission(_address);
  I2CWRITE((uint8_t) reg);
  
  I2CWRITE((uint8_t) datau.b[0]);
  I2CWRITE((uint8_t) datau.b[1]);
  I2CWRITE((uint8_t) datau.b[2]);
  
  Wire.endTransmission();
  return;
}

bool KEYS::read3Bytes(uint32_t *data, uint8_t reg) {

  union
  {
    uint8_t b[4];
    uint32_t w;
  } datau;
  
  datau.w = *data;

  Wire.beginTransmission(_address);
  I2CWRITE((uint8_t) reg);
  Wire.endTransmission();
  uint8_t timeout=0;
  
  Wire.requestFrom(_address, (uint8_t) 0x03);
  while(Wire.available() < 3) {
    timeout++;
	if(timeout > I2CTIMEOUT) {
	  return(true);
	}
	delay(1);
  } 		//Experimental
  
  datau.b[0] = I2CREAD();
  datau.b[1] = I2CREAD();
  datau.b[2] = I2CREAD();
  
  *data = datau.w;
  

return(false);
}

void KEYS::pinMode(uint32_t pin, uint8_t mode) {
  uint32_t pullUp, dbc;
  
  readGPIO();
  switch(mode) {
    case INPUT:
	  bitClear(_PORT, pin);
	  bitClear(_DDR, pin);
	  bitClear(_PKG, pin);
	  bitSet(_PUR, pin);
	  break;
	case INPUT_PULLUP:
	  bitClear(_PORT, pin);
	  bitClear(_DDR, pin);
	  bitClear(_PKG, pin);
	  bitClear(_PUR, pin); 
	  break;
    case OUTPUT:
	  bitClear(_PORT, pin);
	  bitSet(_DDR, pin);
	  bitClear(_PKG, pin);
	  bitSet(_PUR, pin);
	  break;
	case KEYPAD:
	  bitClear(_PORT, pin);
	  bitClear(_DDR, pin);
	  bitSet(_PKG, pin);
	  bitClear(_PUR, pin); 
	  break;
	case DEBOUNCE:
	  read3Bytes((uint32_t *)&dbc, REG_DEBOUNCE_DIS1);
	  bitClear(dbc, pin);
	  write3Bytes((uint32_t)dbc, REG_DEBOUNCE_DIS1);
	  break;
	case NODEBOUNCE:
	  read3Bytes((uint32_t *)&dbc, REG_DEBOUNCE_DIS1);
	  bitSet(dbc, pin);
	  write3Bytes((uint32_t)dbc, REG_DEBOUNCE_DIS1);
	  break;
	default:
	  break;
  }
  updateGPIO();
}


void KEYS::digitalWrite(uint32_t pin, uint8_t value) {

  
  if(value)
    bitSet(_PORT, pin);
  else
    bitClear(_PORT, pin);

  updateGPIO();
}

uint8_t KEYS::digitalRead(uint32_t pin) {

  readGPIO();
  
  return(_PIN & bit(pin)) ? HIGH : LOW;
}

void KEYS::write(uint32_t value) {

  _PORT = value;
  
  updateGPIO();
}

uint32_t KEYS::read(void) {

  readGPIO();
  
  return _PORT;
}

void KEYS::toggle(uint32_t pin) {

  _PORT ^= (bit(pin));
  
  updateGPIO();
}

void KEYS::blink(uint32_t pin, uint16_t count, uint32_t duration) {

  duration /= count * 2;
  
  while(count--) {
    toggle(pin);
	delay(duration);
	toggle(pin);
	delay(duration);
  }
}

#ifdef TCA8418_INTERRUPT_SUPPORT
void KEYS::enableInterrupt(uint8_t pin, void(*selfCheckFunction)(void)) {

  _pcintPin = pin;
  
#if ARDUINO >= 100
  ::pinMode(pin, INPUT_PULLUP);
#else
  ::pinMode(pin, INPUT);
  ::digitalWrite(pin, HIGH);
#endif

  PCattachInterrupt(pin, selfCheckFunction, FALLING);
}

void KEYS::disableInterrupt() {
  PCdetachInterrupt(_pcintPin);
}

void KEYS::pinInterruptMode(uint32_t pin, uint8_t mode, uint8_t level, uint8_t fifo) {
  uint32_t intSetting, levelSetting, eventmodeSetting;

	
  read3Bytes((uint32_t *)&intSetting, REG_GPIO_INT_EN1);
  read3Bytes((uint32_t *)&levelSetting, REG_GPIO_INT_LVL1);
  read3Bytes((uint32_t *)&eventmodeSetting, REG_GPI_EM1);

  switch(mode) {
    case INTERRUPT:
	  bitSet(intSetting, pin);
	  break;
	case NOINTERRUPT:
	  bitClear(intSetting, pin);
	  break;
	default:
		break;
  }
  
  switch(level) {
    case LOW:
	  bitClear(levelSetting, pin);
	  break;
	case HIGH:
	  bitSet(levelSetting, pin);
	  break;
	default:
	  break;
  }
  
  switch(fifo) {
    case FIFO:
	  bitSet(eventmodeSetting, pin);
	  break;
	case NOFIFO:
	  bitClear(eventmodeSetting, pin);
	  break;
	default:
	  break;
  }
  
  write3Bytes((uint32_t)intSetting, REG_GPIO_INT_EN1);
  write3Bytes((uint32_t)levelSetting, REG_GPIO_INT_LVL1);
  write3Bytes((uint32_t)eventmodeSetting, REG_GPI_EM1);
  
}

void KEYS::pinInterruptMode(uint32_t pin, uint8_t mode) {
  pinInterruptMode(pin, mode, 0, 0);
}

#endif

void KEYS::readGPIO() {

#ifdef TCA8418_INTERRUPT_SUPPORT
	/* Store old _PIN value */
	_oldPIN = _PIN;
#endif
	
	read3Bytes((uint32_t *)&_PORT, REG_GPIO_DAT_OUT1);  //Read Data OUT Registers
	read3Bytes((uint32_t *)&_PIN, REG_GPIO_DAT_STAT1);	//Read Data STATUS Registers
	read3Bytes((uint32_t *)&_DDR, REG_GPIO_DIR1);		//Read Data DIRECTION Registers
	read3Bytes((uint32_t *)&_PKG, REG_KP_GPIO1);		//Read Keypad/GPIO SELECTION Registers
	read3Bytes((uint32_t *)&_PUR, REG_GPIO_PULL1);		//Read KPull-Up RESISTOR Registers
}

void KEYS::updateGPIO() {

	write3Bytes((uint32_t)_PORT, REG_GPIO_DAT_OUT1);  	//Write Data OUT Registers
	write3Bytes((uint32_t)_DDR, REG_GPIO_DIR1);			//Write Data DIRECTION Registers
	write3Bytes((uint32_t)_PKG, REG_KP_GPIO1);			//Write Keypad/GPIO SELECTION Registers
	write3Bytes((uint32_t)_PUR, REG_GPIO_PULL1);		//Write Pull-Up RESISTOR Registers
}

void KEYS::dumpreg(void) {
  uint8_t data;
  for(int x=0x01;x<0x2F;x++) {
    readByte(&data, x);
	Serial.print(x, HEX);
	Serial.print(":");
	Serial.print(data, HEX);
	Serial.print(" ");
  }
  Serial.print("\n");
}

uint8_t KEYS::getInterruptStatus(void) {
  uint8_t status;
  
  readByte(&status, REG_INT_STAT);
  return(status & 0x0F);
}

void KEYS::clearInterruptStatus(uint8_t flags) {

  flags &= 0x0F;
  writeByte(flags, REG_INT_STAT);
}

void KEYS::clearInterruptStatus(void) {
  clearInterruptStatus(0x0F);
}

uint8_t KEYS::getKeyEvent(uint8_t event) {
  uint8_t reg;
  uint8_t keycode = 0;
  
  if (event > 9)
    return 0x0;

  readByte(&reg, REG_INT_STAT);
  if(reg & INT_STAT_K_INT)
      readByte(&keycode, (REG_KEY_EVENT_A+event));
  else
      keycode = 0x0;

  return(keycode);
}

uint8_t KEYS::getKeyEvent(void) {
  return(getKeyEvent(0));
}

uint8_t KEYS::getKeyEventCount(void) {
  uint8_t count;
  
  readByte(&count, REG_KEY_LCK_EC);
  return(count & 0x0F);
}

uint32_t KEYS::getGPIOInterrupt(void) {
  uint32_t Ints;
  
  union {
    uint32_t val;
	uint8_t arr[4];
  } IntU;
  
  readByte(&IntU.arr[2], REG_GPIO_INT_STAT3);
  readByte(&IntU.arr[1], REG_GPIO_INT_STAT2);
  readByte(&IntU.arr[0], REG_GPIO_INT_STAT1);

  Ints = IntU.val;
  return(Ints);
}

bool KEYS::isKeyDown(uint8_t key) {
  if(key & 0x80)
    return true;
  else
    return false;
}

bool KEYS::getKey(uint8_t *key) {
  uint8_t tmpkey;
  
  tmpkey = readKeypad();
  *key = *key & 0x7F;
  
  return(isKeyDown(tmpkey));
}

uint8_t KEYS::getKey(void) {
  return(readKeypad() & 0x7F);
}
