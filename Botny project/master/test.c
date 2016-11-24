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
	
	/* basic on/off
	 * write code to get started (fog on)
	digitalWrite(pin, 1);
	digitalWrite(pin, 0);
	*/
	
	int hr = hourCall();
	int mn = minCall();

	int upTempMax = 23;	//Celsius max desired heat
	//int downTempMax = 20;	//Celsius max desired heat
	int upTemp; //box temp

	
	
	//float pHtest = adc0();	//intial ADC readings
	//float waterLevel = adc1();
	
	//THS Bullshit
	//int x = thsDown();	//need to rename these in .h file <-------
	//int z = 3;
	//printf("time%i:%i AN0:%4.2f AN1:%4.2f Temp:%i %i\n", hr, mn, pHtest, waterLevel, z, x);
	
	//kill these
	digitalWrite(peltierPin, 0);
	upTemp = 666;
	
	while(1)
	{
		printf("%i:%i  BoxTemp:%i\n", hr, mn, upTemp);
		//if (mn != minCall())	//if a minute has passed
		{
			mn = minCall();	//resets tracker
			
			upTemp = thsUp();	//check box temp
			//if too hot turn on fans
			if (upTempMax < upTemp)
			{digitalWrite(fanPin, 1);}	
			else
			{digitalWrite(fanPin, 0);}
		}
	}
	return 0;
}
