#ifndef ODROID_Si1132_H
#define ODROID_Si1132_H

#if (ARDUINO >= 100)
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

/* COMMANDS */
#define Si1132_PARAM_QUERY	0x80
#define Si1132_PARAM_SET	0xA0
#define Si1132_NOP		0x00
#define Si1132_RESET		0x01
#define Si1132_BUSADDR		0x02
#define Si1132_GET_CAL		0x12
#define Si1132_ALS_FORCE	0x06
#define Si1132_ALS_PAUSE	0x0A
#define Si1132_ALS_AUTO		0x0E

/* Parameters */
#define Si1132_PARAM_I2CADDR	0x00
#define Si1132_PARAM_CHLIST	0x01
#define Si1132_PARAM_CHLIST_ENUV 0x80
#define Si1132_PARAM_CHLIST_ENAUX 0x40
#define Si1132_PARAM_CHLIST_ENALSIR 0x20
#define Si1132_PARAM_CHLIST_ENALSVIS 0x10

#define Si1132_PARAM_ALSENCODING 0x06

#define Si1132_PARAM_ALSIRADCMUX 0x0E
#define Si1132_PARAM_AUXADCMUX	0x0F

#define Si1132_PARAM_ALSVISADCCOUNTER 0x10
#define Si1132_PARAM_ALSVISADCGAIN 0x11
#define Si1132_PARAM_ALSVISADCMISC 0x12
#define Si1132_PARAM_ALSVISADCMISC_VISRANGE 0x20

#define Si1132_PARAM_ALSIRADCCOUNTER 0x1D
#define Si1132_PARAM_ALSIRADCGAIN 0x1E
#define Si1132_PARAM_ALSIRADCMISC 0x1F
#define Si1132_PARAM_ALSIRADCMISC_RANGE 0x20

#define Si1132_PARAM_ADCCOUNTER_511CLK 0x70

#define Si1132_PARAM_ADCMUX_SMALLIR 0x00
#define Si1132_PARAM_ADCMUX_LARGEIR 0x03

/* REGISTERS */
#define Si1132_REG_PARTID	0x00
#define Si1132_REG_REVID	0x01
#define Si1132_REG_SEQID	0x02

#define Si1132_REG_INTCFG	0x03
#define Si1132_REG_INTCFG_INTOE	0x01

#define Si1132_REG_IRQEN	0x04
#define Si1132_REG_IRQEN_ALSEVERYSAMPLE 0x01

#define Si1132_REG_IRQMODE1	0x05
#define Si1132_REG_IRQMODE2	0x06

#define Si1132_REG_HWKEY	0x07
#define Si1132_REG_MEASRATE0	0x08
#define Si1132_REG_MEASRATE1	0x09
#define Si1132_REG_UCOEF0	0x13
#define Si1132_REG_UCOEF1	0x14
#define Si1132_REG_UCOEF2	0x15
#define Si1132_REG_UCOEF3	0x16
#define Si1132_REG_PARAMWR	0x17
#define Si1132_REG_COMMAND	0x18
#define Si1132_REG_RESPONSE	0x20
#define Si1132_REG_IRQSTAT	0x21

#define Si1132_REG_ALSVISDATA0	0x22
#define Si1132_REG_ALSVISDATA1	0x23
#define Si1132_REG_ALSIRDATA0	0x24
#define Si1132_REG_ALSIRDATA1	0x25
#define Si1132_REG_UVINDEX0	0x2C
#define Si1132_REG_UVINDEX1	0x2D
#define Si1132_REG_PARAMRD	0x2E
#define Si1132_REG_CHIPSTAT	0x30

#define Si1132_ADDR 0x60

class ODROID_Si1132 {
	public :
		ODROID_Si1132(void);
		boolean begin(void);
		void reset(void);
		void configRegister(void);
		uint16_t readTemperature(void);
		uint16_t readUV(void);
		float readVisible(void);
		float readIR(void);
	private :
		uint16_t read16(uint8_t addr);
		uint8_t read8(uint8_t addr);
		void write8(uint8_t reg, uint8_t val);
		uint8_t readParam(uint8_t p);
		uint8_t writeParam(uint8_t p, uint8_t v);
		uint8_t _addr;
};

#endif
