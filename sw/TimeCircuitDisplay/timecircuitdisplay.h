/*
 * timecircuitdisplay.h
 *
 *  Created on: 15 oct. 2019
 *      Author: LOCHE Jeremy
 */

#ifndef TIMECIRCUITDISPLAY_H_
#define TIMECIRCUITDISPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "ht16k33.h"

#define TCD_MONTH_ASCII 1
#define TCD_MONTH_NUMERIC 0

struct timecircuit_struct
{
	uint8_t hours;
	uint8_t minutes;
	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t flags;
	ht16k33_t *ht16k33_dev;
};

typedef struct timecircuit_struct timecircuit_t;


int8_t tcd_init(timecircuit_t * dev);

//2*7seg hours
int8_t tcd_setHours(timecircuit_t *dev,uint8_t hours);

//2*7seg minutes
int8_t tcd_setMinutes(timecircuit_t * dev, uint8_t minutes);

//2*7seg hours  + 2*7seg minutes
int8_t tcd_setHoursMinutes(timecircuit_t *dev,uint8_t hours,uint8_t minutes);

//2*7seg minutes
int8_t tcd_setDays(timecircuit_t * dev, uint8_t days);

//4*7seg year
int8_t tcd_setYear(timecircuit_t * dev,uint16_t year);

//3*16seg month -> ascii version
int8_t tcd_setMonthAscii(timecircuit_t *dev,char * month);

//3*16seg month -> numeric version
int8_t tcd_setMonthNumber(timecircuit_t *dev,uint8_t month);

//Common function to set month
int8_t tcd_setMonth(timecircuit_t *dev,uint8_t month,uint8_t ascii_numeric);

int8_t tcd_setAMPM(timecircuit_t *dev,uint8_t ampm);

int8_t tcd_setDots(timecircuit_t *dev,uint8_t dots);

int8_t tcd_refresh();

#ifdef __cplusplus
}
#endif

#endif /* TIMECIRCUITDISPLAY_H_ */
