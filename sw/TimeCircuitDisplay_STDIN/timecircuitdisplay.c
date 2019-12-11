/*
 * timecircuitdisplay.c
 *
 *  Created on: 15 oct. 2019
 *      Author: JLH
 */
#include "timecircuitdisplay.h"
#include "segments16_ascii.h"
#include "segments7_ascii.h"
#include <stdio.h>


static int8_t tcd_setDualDigitValue(timecircuit_t * dev,uint8_t value,uint8_t com)
{
	uint8_t asciiUnits= '0' + (value%10);
	uint8_t asciiDiz= '0' + (value/10);

	//7 Segment: DP-G-F-E-D-C-B-A
	//Ascii table starts with SPACE, so space is 32
	uint8_t segUnits= SevenSegmentASCII[asciiUnits-32];
	uint8_t segDiz= SevenSegmentASCII[asciiDiz-32];

	uint16_t bus = ((uint16_t)segUnits)<<8 | segDiz;

	//ht16k33 MSB=LED15 LSB=LED0
	ht16k33_setRow(dev->ht16k33_dev,bus,com);
	return ht16k33_refreshRow(dev->ht16k33_dev,com);
}

/**
 * Init the time circuit display
 * @param dev
 * @return same return codes as ht16k33_init
 */
int8_t tcd_init(timecircuit_t * dev)
{

	return ht16k33_init(dev->ht16k33_dev);
}

//2*7seg hours
int8_t tcd_setHours(timecircuit_t *dev,uint8_t hours)
{
	return tcd_setDualDigitValue(dev, hours, 6);
}

//2*7seg minutes
int8_t tcd_setMinutes(timecircuit_t * dev, uint8_t minutes)
{

	//Minutes on COM7
	//MSD Row [7:0] COM7
	//LSD Row [15:8] COM7
	return tcd_setDualDigitValue(dev, minutes, 7);
}

//2*7seg hours  + 2*7seg minutes
int8_t tcd_setHoursMinutes(timecircuit_t *dev,uint8_t hours,uint8_t minutes)
{
	int8_t res=0;
	res=tcd_setHours(dev, hours);
	if(res)
		return res;
	res=tcd_setMinutes(dev, minutes);
	if(res)
		return res;

	return 0;
}

//2*7seg minutes
int8_t tcd_setDays(timecircuit_t * dev, uint8_t days)
{

	//MSD Row [7:0] COM3
	//LSD Row [15:8] COM3
	return tcd_setDualDigitValue(dev, days, 3);
}

//4*7seg year
int8_t tcd_setYear(timecircuit_t * dev,uint16_t year)
{
	//MSD Row [7:0] COM5
	//MSD Row [15:8] COM5 //2 first digits XXxx
	//LSD Row [7:0] COM4
	//LSD Row [15:8] COM4 //2 low digits xxXX

	tcd_setDualDigitValue(dev, year/100,5);
	tcd_setDualDigitValue(dev, year%100,4);
	return 0;
}

//3*16seg month -> ascii version
int8_t tcd_setMonthAscii(timecircuit_t *dev,char * month)
{

	uint16_t seg1=SixteenSegmentASCII[month[0]-32]; //COM0
	uint16_t seg2=SixteenSegmentASCII[month[1]-32]; //COM1
	uint16_t seg3=SixteenSegmentASCII[month[2]-32]; //COM2

	ht16k33_setRow(dev->ht16k33_dev, seg1, 0);
	ht16k33_setRow(dev->ht16k33_dev, seg2, 1);
	ht16k33_setRow(dev->ht16k33_dev, seg3, 2);

	//ht16k33_refreshRow(dev->ht16k33_dev, 0);
	//ht16k33_refreshRow(dev->ht16k33_dev, 1);
	//ht16k33_refreshRow(dev->ht16k33_dev, 2);
	return ht16k33_writeRAM_N(dev->ht16k33_dev, 0, 2*3);
}

//3*16seg month -> numeric version
int8_t tcd_setMonthNumber(timecircuit_t *dev,uint8_t month)
{

	char tmp[4]={0};

	month/=1000;
	sprintf(tmp,"%03d",month);

	return tcd_setMonthAscii(dev, tmp);
}

//Common function to set month
int8_t tcd_setMonth(timecircuit_t *dev,uint8_t month,uint8_t ascii_numeric)
{

	if(ascii_numeric == TCD_MONTH_NUMERIC)
	{
		return tcd_setMonthNumber(dev,month);
	}

	char * monstr="   ";

	switch(month)
	{
	case 1:
		monstr="JAN";
		break;
	case 2:
		monstr="FEB";
				break;
	case 3:
		monstr="MAR";
				break;
	case 4:
		monstr="APR";
				break;
	case 5:
		monstr="MAY";
				break;
	case 6:
		monstr="JUN";
				break;
	case 7:
		monstr="JUL";
				break;
	case 8:
		monstr="AUG";
				break;
	case 9:
		monstr="SEP";
				break;
	case 10:
		monstr="OCT";
				break;
	case 11:
		monstr="NOV";
				break;
	case 12:
		monstr="DEC";
				break;
	default:
		monstr="   ";
		break;
	}

	return tcd_setMonthAscii(dev, monstr);
}

int8_t tcd_setAMPM(timecircuit_t *dev,uint8_t ampm)
{
  ht16k33_setPixel(dev->ht16k33_dev,7,6,ampm & 0x1 ? 1:0);
  ht16k33_setPixel(dev->ht16k33_dev,15,6,ampm & 0x2 ? 1:0);
  
  return ht16k33_refreshRow(dev->ht16k33_dev,6);

}

int8_t tcd_setDots(timecircuit_t *dev,uint8_t dots)
{
   ht16k33_setPixel(dev->ht16k33_dev,7,7,dots & 0x1 ? 1:0);
  ht16k33_setPixel(dev->ht16k33_dev,15,7,dots & 0x2 ? 1:0);
  
  return ht16k33_refreshRow(dev->ht16k33_dev,7);
}

int8_t tcd_refresh()
{
	return 0;
}
