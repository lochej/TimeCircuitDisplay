/*
 * main.c
 *
 *  Created on: Oct 24, 2019
 *      Author: fpga
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "timecircuitdisplay.h"
#include <sys/select.h>

//Conflicts with IOCTL
#define MAX_NB_ERRORS_IOCTL 1
#define MAX_NB_ERRORS_I2C 5
#define TIMEOUT_SEC_NO_ERROR_COUNTER_RESET 10

#define REFRESH_PERIOD_MS 250
ht16k33_t hdev={0};
timecircuit_t htcd={0};
uint8_t cnt_hours=88; //set all default values to 88, ensures a first set in the while loop.
uint8_t cnt_minutes=88;
uint8_t cnt_days=88;
uint8_t cnt_months=88;
uint16_t cnt_year=8888;
uint8_t cnt_seconds=0;
uint16_t cnt_ms=0;
uint8_t brightness=0;


/**
 * Prints an error message and its result code and exiting the software with specified exit code
 * @param cause_msg
 * @param cause_result_code
 * @param exit_code
 */
void errorExit(char * cause_msg, int cause_result_code,int exit_code)
{
	if(cause_result_code)
	{
		printf("Fatal error cause exit : %s with res_code:%d\n",cause_msg,cause_result_code);
		exit(exit_code);
	}
}

/**
 * Checks and counts errors on the TCD commands. If too many I2C errors or IOCTL errors occur, then the program will exit with an error.
 * @param cause_msg
 * @param cause_result_code
 */
void check_tcd_errors(char * cause_msg,int cause_result_code)
{
	static uint8_t ioctl_error_count=0;
	static uint8_t i2c_write_error_count=0;

	static time_t last_error_time=0;
	//static uint8_t first_check=1;

	//Not robust to change in date of the system ?
#if 0
	//first call of the function, init last_error_time;
	if(first_check){
		first_check=0;
		last_error_time=time(NULL);
	}

	//If it has been more than 10 seconds without i2c errors, reset the counter.
	if(time(NULL) - last_error_time >= TIMEOUT_SEC_NO_ERROR_COUNTER_RESET)
	{
		last_error_time=time(NULL);
		i2c_write_error_count=0;
		ioctl_error_count=0;
	}
#endif

	//No errors
	if(cause_result_code==0)
		return;

	switch(cause_result_code)
	{
	case -1: //IOCTL acquire errors
		ioctl_error_count++;

		//register error our and date
		last_error_time=time(NULL);

		printf("Error %d/%d IOCTL acquire registered for %s with code=%d\n",ioctl_error_count,MAX_NB_ERRORS_IOCTL,cause_msg,cause_result_code);
		printf("Is it a /dev/i2c-* device ?\n");

		if(ioctl_error_count >= MAX_NB_ERRORS_IOCTL){
			printf("Reached max IOCTL errors\n");
			errorExit(cause_msg, cause_result_code, EXIT_FAILURE);
			return;
		}

		break;
	case -2: //I2C write failed, because no component responding ACK on the line
		i2c_write_error_count++;

		//register error our and date
		last_error_time=time(NULL);

		printf("Error %d / %d I2C write registered for %s with code=%d\n",i2c_write_error_count,MAX_NB_ERRORS_I2C,cause_msg,cause_result_code);

		if(i2c_write_error_count >= MAX_NB_ERRORS_I2C){
			printf("Reached max I2C write errors\n");
			errorExit(cause_msg, cause_result_code, EXIT_FAILURE);
			return;
		}

		break;
	case -3: //I2C not the correct amount of bytes written to I2C. Possibly a programming error.
		printf("Error I2C not write correct nb of bytes for %s with code=%d\n",cause_msg,cause_result_code);
		errorExit(cause_msg, cause_result_code, EXIT_FAILURE);
		break;
	}
}

//Data is to be read from stdin
int check_stdin()
{
	// if != 0, then there is data to be read on stdin
	// timeout structure passed into select
	struct timeval tv;
	// fd_set passed into select
	fd_set fds;
	// Set up the timeout.  here we can wait for 1 second
	tv.tv_sec = 0;
	tv.tv_usec = REFRESH_PERIOD_MS*1000;

	// Zero out the fd_set - make sure it's pristine
	FD_ZERO(&fds);
	// Set the FD that we want to read
	FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
	// select takes the last file descriptor value + 1 in the fdset to check,
	// the fdset for reads, writes, and errors.  We are only passing in reads.
	// the last parameter is the timeout.  select will return if an FD is ready or
	// the timeout has occurred
	if(select(STDIN_FILENO+1, &fds, NULL, NULL, &tv)==-1)
	{
		printf("Error occured in Select\n");
	}

	usleep(tv.tv_usec);
	// return 0 if STDIN is not ready to be read.
	return FD_ISSET(STDIN_FILENO, &fds);
}

void read_brightness(){

	char buffer[10];
	int res;
	res=read(STDIN_FILENO, buffer, 10);

	if(res<0){
		//error
		printf("Error on stdin read\n");
		return;
	}
	else if(res>0)
	{
		buffer[res-1]='\0';
		int value= strtol(buffer,NULL,10);

		ht16k33_setBrightness(&hdev, value);
		printf("Read %s -> %d\n",buffer,value);
	}

}

/**
 * Time Circuit Display with arguments
 * @param argc
 * @param argv
 * @return
 */
int main(int argc,char ** argv)
{
	int res;
	printf("Time Circuit Display driving program\n");

	//Check for program arguments
	if(argc < 2){
		printf("Usage is : argv[1]=brightness optionnal argv[2]=i2c_dev file\n");
		printf("Brightness value of 0 means display OFF\n");
		printf("Brightness value of 1 (min) to 16 (max) starts the display with brightness\n");
		printf("i2c file should be provided as parameter 2, if not /dev/i2c-1 will be chosen\n");
		return -1;
	}

	//Parse brightness
	if(argc > 1){
		brightness=(uint8_t) strtol(argv[1],NULL,10);
	}
	else{
		brightness=16;
	}

	hdev.address=0; //Change to the A2:A1:A0 bits of your Time circuit
	hdev.i2c_dev_file= argc >= 3 ? argv[2] : "/dev/i2c-1";

	htcd.ht16k33_dev=&hdev;

	res = tcd_init(&htcd); //could crash because i2c file open failed, couldn't write to device,

	check_tcd_errors("tcd_init()", res);

	if(brightness == 0){
		ht16k33_displayOn(&hdev, brightness == 0 ? 0 : 1);
		printf("Turning display OFF\n");
		exit(EXIT_SUCCESS);
	}

	/*
	tcd_setDays(&htcd,cnt_days);
	tcd_setMonth(&htcd,cnt_months,TCD_MONTH_ASCII);
	tcd_setYear(&htcd,cnt_year);
	tcd_setHoursMinutes(&htcd,cnt_hours,cnt_minutes);
	tcd_setDots(&htcd,cnt_seconds);
	tcd_setAMPM(&htcd,0);
	 */
	printf("Turning display ON\n");
	printf("Setting TCD brightness to %d\n",brightness);

	res = ht16k33_setBrightness(htcd.ht16k33_dev, brightness-1);

	errorExit("ht16k33_setBrightness() failed", res, EXIT_FAILURE);


	printf("Displaying time every %d ms\n",REFRESH_PERIOD_MS);

	time_t now=time(NULL);
	struct tm * time_fields;

	while(1)
	{
		now=time(NULL);
		time_fields=localtime(&now);

		if(cnt_days != time_fields->tm_mday)
		{

			res=tcd_setDays(&htcd,time_fields->tm_mday);

			check_tcd_errors("tcd_setDays()", res);

			//Applied days correctly
			if(!res)
				cnt_days=time_fields->tm_mday;

		}

		if(cnt_months!=time_fields->tm_mon+1)
		{

			res=tcd_setMonth(&htcd,time_fields->tm_mon+1,TCD_MONTH_ASCII);

			check_tcd_errors("tcd_setMonth()",res);

			if(!res)
				cnt_months=time_fields->tm_mon+1;


		}

		if(cnt_year!=(time_fields->tm_year+1900))
		{

			res=tcd_setYear(&htcd,(time_fields->tm_year+1900));

			check_tcd_errors("tcd_setYear()",res);

			if(!res)
				cnt_year=time_fields->tm_year+1900;


		}

		if(cnt_hours!=time_fields->tm_hour)
		{

			res=tcd_setHours(&htcd, time_fields->tm_hour);

			check_tcd_errors("tcd_setHours()",res);
			if(!res)
				cnt_hours=time_fields->tm_hour;



			res=tcd_setAMPM(&htcd, cnt_hours > 12 ? 0x2: 0x1);

			check_tcd_errors("tcd_setAMPM()",res);
		}

		if(cnt_minutes!=time_fields->tm_min)
		{

			res=tcd_setMinutes(&htcd, time_fields->tm_min);

			if(!res)
				cnt_minutes=time_fields->tm_min;

			check_tcd_errors("tcd_setMinutes()",res);
		}

		if(cnt_seconds != time_fields->tm_sec)
		{
			cnt_ms=0;
			cnt_seconds=time_fields->tm_sec;
			//tcd_setDots(&htcd,cnt_seconds);
			//tcd_setAMPM(&htcd,cnt_seconds);
		}

		res=tcd_setDots(&htcd,cnt_ms/REFRESH_PERIOD_MS);

		check_tcd_errors("tcd_setDots()",res);

		//usleep(REFRESH_PERIOD_MS*1000);

		/** Read stdin to check for new birghtness value */
		if(check_stdin())
		{
			read_brightness();
		}


		cnt_ms+=REFRESH_PERIOD_MS;
	}

	return 0;
}
