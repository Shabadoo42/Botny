/*
 * LED.c
 */

#include <stdio.h>
#include <wiringPi.h>

int main(int argc, char **argv)
{
	wiringPiSetupGpio();	
	int redLed = 19;
	int grnLed = 13;
	int bluLed = 6;

	pinMode(redLed, OUTPUT);
	pinMode(grnLed, OUTPUT);
	pinMode(bluLed, OUTPUT);
	
	while(1)
	{
		
		digitalWrite(redLed, 1);	//off
		digitalWrite(grnLed, 1);
		digitalWrite(bluLed, 1);
		delay(500);
		digitalWrite(redLed, 0);	//red
		digitalWrite(grnLed, 1);
		digitalWrite(bluLed, 1);
		delay(500);
		digitalWrite(redLed, 1);	//green
		digitalWrite(grnLed, 0);
		digitalWrite(bluLed, 1);
		delay(500);
		digitalWrite(redLed, 1);	//blue
		digitalWrite(grnLed, 1);
		digitalWrite(bluLed, 0);
		delay(500);
		digitalWrite(redLed, 0);	//orangish
		digitalWrite(grnLed, 0);
		digitalWrite(bluLed, 1);
		delay(500);
		digitalWrite(redLed, 0);	//purple
		digitalWrite(grnLed, 1);
		digitalWrite(bluLed, 0);
		delay(500);
		digitalWrite(redLed, 1);	//cyan
		digitalWrite(grnLed, 0);
		digitalWrite(bluLed, 0);
		delay(500);
		digitalWrite(redLed, 0);	//white
		digitalWrite(grnLed, 0);
		digitalWrite(bluLed, 0);
		delay(500);
	}
	
	return 0;
}

