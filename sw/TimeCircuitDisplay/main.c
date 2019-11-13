/*
 * main.c
 *
 *  Created on: Oct 24, 2019
 *      Author: LOCHE Jeremy
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "timecircuitdisplay.h"

ht16k33_t hdev={0};
timecircuit_t htcd={0};
uint8_t cnt_hours=0;
uint8_t cnt_minutes=0;
uint8_t cnt_days=0;
uint8_t cnt_months=0;
uint16_t cnt_year=0;
uint8_t cnt_seconds=0;
uint16_t cnt_ms=0;
uint8_t brightness=0;

int main(int argc,char ** argv)
{
	printf("Hello World from Time Circuit\n");

	if(argc > 1)
	{
		brightness=(uint8_t) atoi(argv[1]);
	}
	else
		brightness=15;



	htcd.ht16k33_dev=&hdev;


	tcd_init(&htcd);

	/*
	tcd_setDays(&htcd,cnt_days);
	tcd_setMonth(&htcd,cnt_months,TCD_MONTH_ASCII);
	tcd_setYear(&htcd,cnt_year);
	tcd_setHoursMinutes(&htcd,cnt_hours,cnt_minutes);
	tcd_setDots(&htcd,cnt_seconds);
	tcd_setAMPM(&htcd,0);
	*/
	ht16k33_setBrightness(htcd.ht16k33_dev, brightness);

	printf("Config TCD Done\n");

	int i =0;

	time_t now=time(NULL);
	struct tm * time_fields;

	while(1)
	{
		now=time(NULL);
		time_fields=localtime(&now);



		if(cnt_days != time_fields->tm_mday)
		{
			cnt_days=time_fields->tm_mday;
			tcd_setDays(&htcd,cnt_days);
		}

		if(cnt_months!=time_fields->tm_mon+1)
		{
			cnt_months=time_fields->tm_mon+1;
			tcd_setMonth(&htcd,cnt_months,TCD_MONTH_ASCII);
		}

		if(cnt_year!=(time_fields->tm_year+1900))
		{
			cnt_year=time_fields->tm_year+1900;
			tcd_setYear(&htcd,cnt_year);
		}

		if(cnt_hours!=time_fields->tm_hour)
		{
			cnt_hours=time_fields->tm_hour;
			tcd_setHours(&htcd, cnt_hours);
			tcd_setAMPM(&htcd, cnt_hours > 12 ? 0x2: 0x1);
		}

		if(cnt_minutes!=time_fields->tm_min)
		{
			cnt_minutes=time_fields->tm_min;
			tcd_setMinutes(&htcd, cnt_minutes);
		}

		if(cnt_seconds != time_fields->tm_sec)
		{
			cnt_ms=0;
			cnt_seconds=time_fields->tm_sec;
			//tcd_setDots(&htcd,cnt_seconds);
			//tcd_setAMPM(&htcd,cnt_seconds);
		}

		tcd_setDots(&htcd,cnt_ms/250);

		usleep(250*1000);
		cnt_ms+=250;
	}

	return 0;
}
