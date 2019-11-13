/*
 * ht16k33.h
 *
 *  Created on: 13 oct. 2019
 *      Author: LOCHE Jeremy
 */

#ifndef HT16K33_H_
#define HT16K33_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <string.h>

#define HT16K33_I2C_7B_BASE_ADDR 0x70


#define HT16K33_BLINK_OFF 0
#define HT16K33_BLINK_2HZ 1
#define HT16K33_BLINK_1HZ 2
#define HT16K33_BLINK_05HZ 3
#define HT16K33_DISPLAY_ON 1
#define HT16K33_DISPLAY_OFF 0


#define HT16K33_REG_DISPLAY_DATA_RAM_BASE 0x00
#define HT16K33_REG_KEY_DATA_RAM_BASE 0x40
#define HT16K33_REG_DIMMING_BASE 0xE0
#define HT16K33_CMD_DISPLAY_SETUP(_on_,_blink_) (0x80 | ((_on_) ? 0x01:0x00) | (((_blink_)&0x3)<<1) )
#define HT16K33_CMD_SYSTEM_SETUP_NORMAL 0x21
#define HT16K33_CMD_SYSTEM_SETUP_STANDBY 0x20
#define HT16K33_CMD_ROWINT_OUTPUT 0xA0

#define HT16K33_RAM_START (0)
#define HT16K33_RAM_SIZE (16)

#define HT16K33_STATUS_GET_ON(_status_) ((_status_) & 0x80 ? 1:0)
#define HT16K33_STATUS_GET_STANDBY(_status_) ((_status_) & 0x40 ? 1:0)
#define HT16K33_STATUS_GET_BLINK(_status_) ((_status_) & 0x03)
#define HT16K33_STATUS_GET_BRIGHTNESS(_status_) (((_status_)>>2) & 0x0F)

#define HT16K33_STATUS_SET_ON(_status_,_on_) ((_status_) = (_on_) ? ((_status_) | 0x80) : ((_status_) & 0x7F) )
#define HT16K33_STATUS_SET_STANDBY(_status_,_standby_) ((_status_) = (_standby_) ? ((_status_) | 0x40) : ((_status_) & 0xBF) )
#define HT16K33_STATUS_SET_BLINK(_status_,_blink_) ((_status_) = (((_status_)&0xFC) | (_blink_)) )
#define HT16K33_STATUS_SET_BRIGHTNESS(_status_,_dim_) ((_status_) = ((_status_) & 0xC3) | ((_dim_)<<2))



struct ht16k33_dev_struct
{
	uint8_t address; //A2 A1 A0 bits in this order
	uint8_t status;
	uint8_t ram_data[HT16K33_RAM_SIZE];
};

typedef struct ht16k33_dev_struct ht16k33_t;

int8_t ht16k33_init(ht16k33_t * dev);

//Pilotage haut niveau
/**
 * Sets the dimming of the screen, 0 means lowest brightness 1/16 and 0xF means max brightness 16/16
 * @param dev
 * @param dim
 * @return
 */
int8_t ht16k33_setBrightness(ht16k33_t * dev,uint8_t dim);
int8_t ht16k33_displaySetup(ht16k33_t * dev,uint8_t on,uint8_t blink);
int8_t ht16k33_systemStandby(ht16k33_t * dev,uint8_t mode);

//int8_t ht16k33_displayOn(ht16k33_t * dev,uint8_t on);
//int8_t ht16k33_blink(ht16k33_t * dev,uint8_t blink);
static inline int8_t ht16k33_displayOn(ht16k33_t * dev,uint8_t on)
{
	return ht16k33_displaySetup(dev, on, HT16K33_STATUS_GET_BLINK(dev->status));
}

static inline int8_t ht16k33_blink(ht16k33_t * dev,uint8_t blink)
{
	return ht16k33_displaySetup(dev, HT16K33_STATUS_GET_ON(dev->status), blink);
}



int8_t ht16k33_setPixel(ht16k33_t * dev,uint8_t row, uint8_t column,uint8_t state);
int8_t ht16k33_setRow(ht16k33_t *dev,uint16_t row,uint8_t com);

//Pilotage bas niveau

int8_t ht16k33_writeRAM_N(ht16k33_t * dev,uint8_t from_addr, uint8_t nb);
int8_t ht16k33_readRAM_N(ht16k33_t * dev,uint8_t from_addr, uint8_t nb);

int8_t ht16k33_writeRAM(ht16k33_t * dev,uint8_t from_addr, uint8_t to_addr);
int8_t ht16k33_readRAM(ht16k33_t * dev,uint8_t from_addr, uint8_t to_addr);

static inline int8_t ht16k33_refreshAll(ht16k33_t * dev)
{
	return ht16k33_writeRAM_N(dev, HT16K33_RAM_START,HT16K33_RAM_SIZE);
}

static inline int8_t ht16k33_refreshRow(ht16k33_t * dev,uint8_t com)
{
	return ht16k33_writeRAM_N(dev,com*2,2);
}

//Communication bas niveau

int8_t ht16k33_writeCommand(ht16k33_t * dev,uint8_t cmd);

int8_t ht16k33_writeReg(ht16k33_t * dev,uint8_t addr,uint8_t reg);

int8_t ht16k33_writeRegs(ht16k33_t * dev,uint8_t base_addr,uint8_t * regs,uint8_t nb);

int8_t ht16k33_readReg(ht16k33_t * dev,uint8_t addr,uint8_t * reg);

int8_t ht16k33_readRegs(ht16k33_t * dev,uint8_t base_addr,uint8_t * reg_out,uint8_t nb);

#ifdef __cplusplus
}
#endif

#endif /* HT16K33_H_ */
