/*
 * ht16k33.c
 *
 *  Created on: 13 oct. 2019
 *      Author: JLH
 */

#include "ht16k33.h"
#include <stdio.h>
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>
#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>


/**
 * Init the ht16k33 device using the provided device structure
 * @param dev
 * @return
 *  0 = SUCCESS,
 * -1 = I2C_IOCTL_ACQUIRE_FAILED,
 * -2 = I2C_COULDNT_WRITE,
 * -3 = I2C_NOT_CORRECT_NB_OF_BYTES_WRITTEN
 * -6 = I2C_FILE is NULL
 * -7 = COULDNT OPEN I2C FILE
 */
int8_t ht16k33_init(ht16k33_t * dev)
{
	int res;
	//----- OPEN THE I2C BUS -----
	if(dev->i2c_dev_file == NULL)
	{
		printf("i2c file name is NULL, can't init\n");
		return -6;
	}

	if ((dev->i2c_fd = open(dev->i2c_dev_file, O_RDWR)) < 0)
	{
		//ERROR HANDLING: you can check errno to see what went wrong
		printf("Failed to open the i2c bus %s\n",dev->i2c_dev_file);
		return -7;
	}

	//memset(dev->ram_data,0x00,HT16K33_RAM_SIZE);

	res=ht16k33_systemStandby(dev, HT16K33_CMD_SYSTEM_SETUP_NORMAL);
	if(res) return res;

	res=ht16k33_writeCommand(dev, HT16K33_CMD_ROWINT_OUTPUT);
	if(res) return res;

	res=ht16k33_setBrightness(dev,0xF);
	if(res) return res;

	res=ht16k33_displaySetup(dev, HT16K33_DISPLAY_OFF, HT16K33_BLINK_OFF);
	if(res) return res;

	res=ht16k33_writeRAM_N(dev, HT16K33_RAM_START, HT16K33_RAM_SIZE);
	if(res) return res;

	res=ht16k33_displayOn(dev, HT16K33_DISPLAY_ON);
	if(res) return res;

	return 0;
}

/**
 * Sets the a single pixel in the local ht16k33 buffer row pin and com pin
 * @param dev
 * @param row : row between 0 and 15
 * @param column : column between 0 and 7
 * @return 0 = SUCCESS, -1 = pixel coordinates out of range
 */
int8_t ht16k33_setPixel(ht16k33_t * dev,uint8_t row, uint8_t column,uint8_t state)
{
	if(row >= 16 || column >=8)
		return -1;

	uint8_t ram_id=column*2 + (row > 7 ? 1:0);
	uint8_t ram_off=(row > 7 ? row-8 : row);

	if(state) dev->ram_data[ram_id] |= (1<<ram_off);
	else dev->ram_data[ram_id] &= ~(1<<ram_off);

	return 0;
}

/**
 * Sets the whole local ht16k33 buffer row for the given com pin
 * @param dev
 * @param row : 16 bits ROW value
 * @param com : given com
 * @return 0 = SUCCESS, -1 = COM is out of range
 */
int8_t ht16k33_setRow(ht16k33_t *dev,uint16_t row,uint8_t com)
{
	if(com >= 8)
		return -1;
	dev->ram_data[com*2 + 1]=row>>8;
	dev->ram_data[com*2]=row & 0xFF;
	return 0;
}

/**
 * Sets the HT16k33 brightness value
 * @param dev
 * @param dim : 0 = minimum brightness to 15 = maximum brightness
 * @return error_code :
 *  0 = SUCCESS,
 * -1 = I2C_IOCTL_ACQUIRE_FAILED,
 * -2 = I2C_COULDNT_WRITE,
 * -3 = I2C_NOT_CORRECT_NB_OF_BYTES_WRITTEN
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

/**
 * Sets the HT16k33 display setup, ON and BLINK settings
 * @param dev
 * @param on : true = display ON, false = display OFF
 * @param blink : HT16K33_BLINK_OFF=0, HT16K33_BLINK_2HZ=1, HT16K33_BLINK_1HZ=2, HT16K33_BLINK_05HZ=3
 * @return error_code :
 *  0 = SUCCESS,
 * -1 = I2C_IOCTL_ACQUIRE_FAILED,
 * -2 = I2C_COULDNT_WRITE,
 * -3 = I2C_NOT_CORRECT_NB_OF_BYTES_WRITTEN
 */
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

/**
 * Sets the HT16k33 operating mode, NORMAL or STANDBY
 * @param dev
 * @param mode : mode to set
 * @return error_code :
 *  0 = SUCCESS,
 * -1 = I2C_IOCTL_ACQUIRE_FAILED,
 * -2 = I2C_COULDNT_WRITE,
 * -3 = I2C_NOT_CORRECT_NB_OF_BYTES_WRITTEN
 * -4 = MODE is not HT16K33_CMD_SYSTEM_SETUP_NORMAL or HT16K33_CMD_SYSTEM_SETUP_STANDBY
 */
int8_t ht16k33_systemStandby(ht16k33_t * dev,uint8_t mode)
{
	int8_t res;

	if(!(mode == HT16K33_CMD_SYSTEM_SETUP_NORMAL || mode == HT16K33_CMD_SYSTEM_SETUP_STANDBY))
		return -4;

	res=ht16k33_writeCommand(dev, mode);

	if(res)
		return res;

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

/**
 * Updates the nb RAM bytes from dev struct to the HT16K33
 * @param dev :
 * @param from_addr : ram bytes address from 0 to 15
 * @param nb : number of bytes to write
 * @return error_code :
 *  0 = SUCCESS,
 * -1 = I2C_IOCTL_ACQUIRE_FAILED,
 * -2 = I2C_COULDNT_WRITE,
 * -3 = I2C_NOT_CORRECT_NB_OF_BYTES_WRITTEN
 * -4 = from_addr exceeds ram size
 * -5 = from_addr + nb will exceed ram size
 */
int8_t ht16k33_writeRAM_N(ht16k33_t * dev,uint8_t from_addr, uint8_t nb)
{

	int8_t res=0;

	if(from_addr >= HT16K33_RAM_SIZE) //too far in the buffer
		return -4;

	if((from_addr + nb) > HT16K33_RAM_SIZE) //will overflow buffer
		return -5;


	res=ht16k33_writeRegs(dev, HT16K33_REG_DISPLAY_DATA_RAM_BASE+from_addr,&(dev->ram_data[from_addr]),nb);
	if(res)
		return res;

	return res;
}

/**
 * Updates the nb RAM bytes from HT16K33 to the dev struct
 * @param dev :
 * @param from_addr : ram bytes address from 0 to 15
 * @param nb : number of bytes to write
 * @return error_code:
 *  0 = SUCCESS,
 * -1 = I2C_IOCTL_ACQUIRE_FAILED,
 * -2 = I2C_COULDNT_WRITE,
 * -3 = I2C_NOT_CORRECT_NB_OF_BYTES_WRITTEN
 * -4 = from_addr exceeds ram size
 * -5 = from_addr + nb will exceed ram size
 */
int8_t ht16k33_readRAM_N(ht16k33_t * dev,uint8_t from_addr, uint8_t nb)
{
	int8_t res=0;

	if(from_addr >= HT16K33_RAM_SIZE) //too far in the buffer
		return -4;

	if((from_addr + nb) > HT16K33_RAM_SIZE) //will overflow buffer
		return -5;


	res=ht16k33_readRegs(dev, HT16K33_REG_DISPLAY_DATA_RAM_BASE+from_addr,(dev->ram_data) + from_addr,nb);
	if(res)
		return res;

	return res;
}

/**
 * Updates the RAM bytes from_addr to to_addr dev struct to the HT16K33
 * @param dev :
 * @param from_addr :first ram address inclusive
 * @param to_addr : last ram address inclusive
 * @return error_code :
 *  0 = SUCCESS,
 * -1 = I2C_IOCTL_ACQUIRE_FAILED,
 * -2 = I2C_COULDNT_WRITE,
 * -3 = I2C_NOT_CORRECT_NB_OF_BYTES_WRITTEN
 * -4 = from_addr exceeds ram size
 * -5 = from_addr is equal or superior to to_addr
 */
int8_t ht16k33_writeRAM(ht16k33_t * dev,uint8_t from_addr, uint8_t to_addr)
{
	if(from_addr >= to_addr)
		return -5;
	return ht16k33_writeRAM_N(dev, from_addr, to_addr - from_addr);
}

/**
 * Updates the RAM bytes from HT16K33 to the dev struct from from_addr to to_addr
 * @param dev :
 * @param from_addr :first ram address inclusive
 * @param to_addr : last ram address inclusive
 * @return error_code :
 *  0 = SUCCESS,
 * -1 = I2C_IOCTL_ACQUIRE_FAILED,
 * -2 = I2C_COULDNT_WRITE,
 * -3 = I2C_NOT_CORRECT_NB_OF_BYTES_WRITTEN
 * -4 = from_addr exceeds ram size
 * -5 = from_addr + nb will exceed ram size
 */
int8_t ht16k33_readRAM(ht16k33_t * dev,uint8_t from_addr, uint8_t to_addr)
{
	if(from_addr >= to_addr)
		return -6;
	return ht16k33_readRAM_N(dev, from_addr, to_addr - from_addr);
}


//Communication bas niveau

/**
 * Send a command byte to the HT16k33
 * @param dev
 * @param cmd : command byte to send
 * @return error_code :
 *  0 = SUCCESS,
 * -1 = I2C_IOCTL_ACQUIRE_FAILED,
 * -2 = I2C_COULDNT_WRITE,
 * -3 = I2C_NOT_CORRECT_NB_OF_BYTES_WRITTEN
 */
int8_t ht16k33_writeCommand(ht16k33_t * dev,uint8_t cmd)
{
	//uint8_t i2c_addr=(HT16K33_I2C_7B_BASE_ADDR | dev->address);

	//printf("Transmit CMD : %0X  -> code = %d\n",cmd, res);

	return ht16k33_writeRegs(dev, cmd, NULL, 0);
}

/**
 * Calls ht16k33 writeRegs with only one register to write
 * @param dev
 * @param addr : addres of the register
 * @param reg : register value
 * @return
 *  0 = SUCCESS,
 * -1 = I2C_IOCTL_ACQUIRE_FAILED,
 * -2 = I2C_COULDNT_WRITE,
 * -3 = I2C_NOT_CORRECT_NB_OF_BYTES_WRITTEN
 */
int8_t ht16k33_writeReg(ht16k33_t * dev,uint8_t addr,uint8_t reg)
{
	return ht16k33_writeRegs(dev, addr, &reg, 1);
}

/**
 * Writes the registers of the HT16K33 component
 * @param dev :
 * @param base_addr :
 * @param regs : registers values buffer
 * @param nb : size of the buffer to send
 * @return
 *  0 = SUCCESS,
 * -1 = I2C_IOCTL_ACQUIRE_FAILED,
 * -2 = I2C_COULDNT_WRITE,
 * -3 = I2C_NOT_CORRECT_NB_OF_BYTES_WRITTEN
 */
int8_t ht16k33_writeRegs(ht16k33_t * dev,uint8_t base_addr,uint8_t * regs,uint8_t nb)
{
	static uint8_t regbuf[17]={0};
	uint8_t i2c_addr=(HT16K33_I2C_7B_BASE_ADDR | dev->address); //Unused in Linux version
	int8_t res=0;

	regbuf[0]=base_addr;


	for(uint8_t i=0;i<nb;i++)
	{
		regbuf[i+1]=regs[i];
	}

	// Acquire i2c bus
	res=ioctl(dev->i2c_fd, I2C_SLAVE, i2c_addr);

	if (res)
	{
		printf("Failed acquire bus to HT16K33 at with addr : 0x%02X. err:%d\n",i2c_addr,res);
		//ERROR HANDLING; you can check errno to see what went wrong
		return -1;
	}

	// Write to the i2c bus
	res=write(dev->i2c_fd, regbuf, nb+1);

	//Check for errors
	if(res < 0)
	{
		printf("Failed to write to the i2c bus %s. err:%d\n",dev->i2c_dev_file,res);
		return -2;
	}
	else if(res != (nb+1))
	{
		printf("Didn't write %d bytes to %s but %d instead\n",nb+1,dev->i2c_dev_file,res);
		return -3;
	}

	//printf("Transmit RAM : %0X  -> nb=%d\n",base_addr,nb);

	return 0;
}

int8_t ht16k33_readReg(ht16k33_t * dev,uint8_t addr,uint8_t * reg)
{
	return ht16k33_readRegs(dev,addr, reg, 1);
}

int8_t ht16k33_readRegs(ht16k33_t * dev,uint8_t base_addr,uint8_t * reg_out,uint8_t nb)
{

	//uint8_t i2c_addr=(HT16K33_I2C_7B_BASE_ADDR | dev->address);

	printf("Read Regs NOT IMPLEMENTED !\n");
	//i2c_readReg(i2c_addr, base_addr,reg_out,nb);
	return -1;
}
