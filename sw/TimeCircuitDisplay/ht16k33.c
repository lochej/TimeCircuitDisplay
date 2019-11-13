/*
 * ht16k33.c
 *
 *  Created on: 13 oct. 2019
 *      Author: LOCHE Jeremy
 */

#include "ht16k33.h"
#include <stdio.h>
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>
#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>

int file_i2c;

int8_t ht16k33_init(ht16k33_t * dev)
{

	//i2c_init2(400000);
	//----- OPEN THE I2C BUS -----
	char *filename = (char*)"/dev/i2c-1";
	if ((file_i2c = open(filename, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		printf("Failed to open the i2c bus");
		return -1;
	}

	int addr = HT16K33_I2C_7B_BASE_ADDR | dev->address;          //<<<<<The I2C address of the slave
	if (ioctl(file_i2c, I2C_SLAVE, addr) < 0)
	{
		printf("Failed to acquire bus access and/or talk to slave.\n");
		//ERROR HANDLING; you can check errno to see what went wrong
		return -1;
	}

	//memset(dev->ram_data,0x00,HT16K33_RAM_SIZE);

	ht16k33_systemStandby(dev, HT16K33_CMD_SYSTEM_SETUP_NORMAL);

	ht16k33_writeCommand(dev, HT16K33_CMD_ROWINT_OUTPUT);

	ht16k33_setBrightness(dev,0xF);

	ht16k33_displaySetup(dev, HT16K33_DISPLAY_OFF, HT16K33_BLINK_OFF);

	ht16k33_writeRAM_N(dev, HT16K33_RAM_START, HT16K33_RAM_SIZE);

	ht16k33_displayOn(dev, HT16K33_DISPLAY_ON);

	return 0;
}


//Pilotage haut niveau
int8_t ht16k33_setPixel(ht16k33_t * dev,uint8_t row, uint8_t column,uint8_t state)
{

	uint8_t ram_id=column*2 + (row > 7 ? 1:0);
	uint8_t ram_off=(row > 7 ? row-8 : row);

	if(state) dev->ram_data[ram_id] |= (1<<ram_off);
	else dev->ram_data[ram_id] &= ~(1<<ram_off);
	return 0;
}

int8_t ht16k33_setRow(ht16k33_t *dev,uint16_t row,uint8_t com)
{
	dev->ram_data[com*2 + 1]=row>>8;
	dev->ram_data[com*2]=row & 0xFF;
	return 0;
}
/**
 * Sets the dimming of the screen, 0 means lowest brightness 1/16 and 0xF means max brightness
 * @param dev
 * @param dim
 * @return
 */
int8_t ht16k33_setBrightness(ht16k33_t * dev,uint8_t dim)
{
	int8_t res;

	if(dim > 0xF)
		dim=0xF;

	res=ht16k33_writeCommand(dev,HT16K33_REG_DIMMING_BASE | dim);
	if(res)
		return res;

	HT16K33_STATUS_SET_BRIGHTNESS(dev->status,dim);
	return res;
}

int8_t ht16k33_displaySetup(ht16k33_t * dev,uint8_t on,uint8_t blink)
{
	int8_t res;

	blink &=0x03; //Make sure blink is only 2 LSB

	uint8_t display_setup=HT16K33_CMD_DISPLAY_SETUP(
			on ? HT16K33_DISPLAY_ON : HT16K33_DISPLAY_OFF,
					blink);

	res=ht16k33_writeCommand(dev,display_setup);

	if(res)
		return res;

	HT16K33_STATUS_SET_BLINK(dev->status,blink);
	HT16K33_STATUS_SET_ON(dev->status,on ? HT16K33_DISPLAY_ON:HT16K33_DISPLAY_OFF);

	return 0;
}

int8_t ht16k33_systemStandby(ht16k33_t * dev,uint8_t mode)
{
	int8_t res;

	if(!(mode == HT16K33_CMD_SYSTEM_SETUP_NORMAL || mode == HT16K33_CMD_SYSTEM_SETUP_STANDBY))
		return -1;

	res=ht16k33_writeCommand(dev, mode);

	if(res)
		return -2;

	HT16K33_STATUS_SET_STANDBY(dev->status,HT16K33_CMD_SYSTEM_SETUP_NORMAL ? 0:1);

	return 0;
}

//Inlined in header file
#if 0
int8_t ht16k33_displayOn(ht16k33_t * dev,uint8_t on)
{
	return ht16k33_displaySetup(dev, on, HT16K33_STATUS_GET_BLINK(dev->status));
}

int8_t ht16k33_blink(ht16k33_t * dev,uint8_t blink)
{
	return ht16k33_displaySetup(dev, HT16K33_STATUS_GET_BLINK(dev->status), blink);
}
#endif

//Pilotage bas niveau

int8_t ht16k33_writeRAM_N(ht16k33_t * dev,uint8_t from_addr, uint8_t nb)
{

	int8_t res=0;

	if(from_addr >= HT16K33_RAM_SIZE) //too far in the buffer
		return -1;

	if((from_addr + nb) > HT16K33_RAM_SIZE) //will overflow buffer
		return -2;


	res=ht16k33_writeRegs(dev, HT16K33_REG_DISPLAY_DATA_RAM_BASE+from_addr,&(dev->ram_data[from_addr]),nb);
	if(res)
		return res;

	return res;
}
int8_t ht16k33_readRAM_N(ht16k33_t * dev,uint8_t from_addr, uint8_t nb)
{
	int8_t res=0;

	if(from_addr >= HT16K33_RAM_SIZE) //too far in the buffer
		return -1;

	if((from_addr + nb) > HT16K33_RAM_SIZE) //will overflow buffer
		return -2;


	res=ht16k33_readRegs(dev, HT16K33_REG_DISPLAY_DATA_RAM_BASE+from_addr,(dev->ram_data) + from_addr,nb);
	if(res)
		return res;

	return res;
}

int8_t ht16k33_writeRAM(ht16k33_t * dev,uint8_t from_addr, uint8_t to_addr)
{
	if(from_addr >= to_addr)
		return -1;
	return ht16k33_writeRAM_N(dev, from_addr, to_addr - from_addr);
}
int8_t ht16k33_readRAM(ht16k33_t * dev,uint8_t from_addr, uint8_t to_addr)
{
	if(from_addr >= to_addr)
		return -1;
	return ht16k33_readRAM_N(dev, from_addr, to_addr - from_addr);
}


//Communication bas niveau

int8_t ht16k33_writeCommand(ht16k33_t * dev,uint8_t cmd)
{
	uint8_t i2c_addr=(HT16K33_I2C_7B_BASE_ADDR | dev->address);

	//i2c_writeReg(i2c_addr, cmd,&cmd, 0);

	//int res = alt_avalon_i2c_master_tx(hi2c,&cmd,1,0);

	if(write(file_i2c, &cmd, 1)!=1)
	{
		printf("Failed to write to the i2c bus.\n");
	}
	//printf("Write command : %d\n",cmd);



	//printf("Transmit CMD : %0X  -> code = %d\n",cmd, res);
	return 0;
}

int8_t ht16k33_writeReg(ht16k33_t * dev,uint8_t addr,uint8_t reg)
{
	return 0;
}

int8_t ht16k33_writeRegs(ht16k33_t * dev,uint8_t base_addr,uint8_t * regs,uint8_t nb)
{
	static uint8_t regbuf[17]={0};
	uint8_t i2c_addr=(HT16K33_I2C_7B_BASE_ADDR | dev->address);


	regbuf[0]=base_addr;

	for(uint8_t i=0;i<nb;i++)
	{
		regbuf[i+1]=regs[i];
	}

	//int res=alt_avalon_i2c_master_tx(hi2c,regbuf,nb+1,0);

	if(write(file_i2c, regbuf, nb+1)!=(nb+1))
	{
		printf("Failed to write to the i2c bus.\n");
	}


	//printf("Transmit RAM : %0X  -> nb=%d\n",base_addr,nb);




	//i2c_writeReg(i2c_addr, base_addr, regs, nb);
	return 0;
}

int8_t ht16k33_readReg(ht16k33_t * dev,uint8_t addr,uint8_t * reg)
{
	return 0;
}

int8_t ht16k33_readRegs(ht16k33_t * dev,uint8_t base_addr,uint8_t * reg_out,uint8_t nb)
{

	uint8_t i2c_addr=(HT16K33_I2C_7B_BASE_ADDR | dev->address);


	//i2c_readReg(i2c_addr, base_addr,reg_out,nb);
	return 0;
}
