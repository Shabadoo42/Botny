/*
 * LED.c
 */

#include <stdio.h>
#include <wiringPi.h>

int main(int argc, char **argv)
{
	wiringPiSetupGpio();
	int pin = 18;
	pinMode(pin, OUTPUT);
	
	while(1)
	{
		digitalWrite(pin, 1);
		delay(500);
		digitalWrite(pin, 0);
		delay(500);
	}
	
	return 0;
}

