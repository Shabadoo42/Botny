//adc
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>	//read/write usleep
#include <stdlib.h>	//exit function
#include <inttypes.h>	//uint8_t, etc
#include <linux/i2c-dev.h>	//i2c bus defintions
//ths -some
#include <wiringPi.h>
#include <stdint.h>
//timecall
#include <time.h>

#include "adc.h"
#include "ths.h"
#include "timeCall.h"


int main()
{
	wiringPiSetupGpio();
	
	int pin = 1800;	//test led
	int lightPin = 111;		//change
	int foggerPin =311;
	int fanPin = 411;
	int peltierPin = 18;
	
	pinMode(pin, OUTPUT);
	pinMode(lightPin, OUTPUT);
	pinMode(foggerPin, OUTPUT);
	pinMode(fanPin, OUTPUT);
	pinMode(peltierPin, OUTPUT);
	
	int hr = hourCall();
	int mn = minCall();
	int begLED = 6;	//LED start time
	int endLED = 23;	//LED end time
	int upTempMax = 23;	//Celsius max desired heat
	//int downTempMax = 20;	//Celsius max desired heat
	int upTemp = 666; //box temp
	//int downTemp; //box temp
	int dcon = 2;	//fogger on time
	int dcoff = 2;	//fogger off time
	int fogTracker = 0;	//tracks which minute in duty cycle
	float pHtest = adc0();	//intial ADC readings
	float waterLevel = adc1();
	
	//THS Bullshit
	//int x = thsDown();	//need to rename these in .h file <-------
	//int z = 3;
	//printf("time%i:%i AN0:%4.2f AN1:%4.2f Temp:%i %i\n", hr, mn, pHtest, waterLevel, z, x);
	
	//kill these
	digitalWrite(peltierPin, 0);	//starts pin at 0
	//upTemp = 666;
	
	while(1)
	{
		printf("time%i:%i AN0:%4.2f AN1:%4.2f Temp:%i \n", hr, mn, pHtest, waterLevel, upTemp);
		//printf("%i:%i fog:%i BoxTemp:%i\n", hr, mn, fogTracker, upTemp);
		//printf("%i:%i fog:%i \n", hr, mn, fogTracker);
		
		if (mn != minCall())	//if a minute has passed
		{
			mn = minCall();	//resets tracker
			
			
			upTemp = thsUp();	//check box temp
			//if too hot turn on fans
			if (upTempMax < upTemp)
			{digitalWrite(fanPin, 1);}	
			else
			{digitalWrite(fanPin, 0);}
			/*
			//downTemp = thsDown();	//check tub temp
			//if too hot turn on peltier
			if (downTempMax < thsDown();)
			{digitalWrite(peltierPin, 1);}	
			else
			{digitalWrite(peltierPin, 0);}
			//code to update thingspeak<-------------
			*/
			
			if (dcon > fogTracker)
			{
				digitalWrite(peltierPin, 1);	//fog on
				fogTracker++;	//increments tracker
			}
			else
			{
				digitalWrite(peltierPin, 0);	//fog off
				fogTracker++;	//increments tracker
				if (fogTracker >= (dcon + dcoff))
				{fogTracker = 0;}
			}
			//code to update thingspeak<-------------
			
		}//end min if
		
		if (hr != hourCall())	//if an hour has passed
		{
			hr = hourCall();	//resets tracker
			
			pHtest = adc0();	//read analogs
			waterLevel = adc1();
			printf("%4.3f %4.3f", pHtest, waterLevel);
			//code to update thingspeak<-------------
			
			if (begLED<endLED)	//if hours following normal rhythms
			{
				if (hr >= begLED && hr < endLED)
				{
					digitalWrite(lightPin, 1);
				}
				else
				{digitalWrite(lightPin, 0);}
			}
			else //vampire rhythms
			{
				if (hr >= begLED || hr < endLED)
				{
					digitalWrite(lightPin, 1);
				}
				else
				{digitalWrite(lightPin, 0);}
			}
			
			//code to update thingspeak<-------------
			
		}//end hr if
		delay(500);	//why the hurry?
	}//end while
	
	return 0;
}
