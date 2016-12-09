
/*
 * LED.c
 */

#include <stdio.h>
#include <wiringPi.h>

int main(int argc, char **argv)
{
	wiringPiSetupGpio();	
	int lightPin = 17;		
	int foggerPin = 4;
	int fanPin = 15;
	int peltierPin = 21;
	//int pwmPin = 18;
	int circPump = 20;
	int airstone = 9;
	int redLed = 19;
	int grnLed = 13;
	int bluLed = 6;


	pinMode(lightPin, OUTPUT);
	pinMode(foggerPin, OUTPUT);
	pinMode(fanPin, OUTPUT);
	
	pinMode(peltierPin, OUTPUT);
	
	//pinMode(pwmPin, PWM_OUTPUT); // Set PWM LED as PWM output
	pinMode(circPump, OUTPUT);
	pinMode(airstone, OUTPUT);
	pinMode(redLed, OUTPUT);
	pinMode(grnLed, OUTPUT);
	pinMode(bluLed, OUTPUT);
	
	while(1)
	{
		
		digitalWrite(lightPin, 1);	//off
		digitalWrite(foggerPin, 1);
		digitalWrite(fanPin, 1);
		
		digitalWrite(peltierPin, 1);
		
		digitalWrite(circPump, 1);
		digitalWrite(airstone, 1);
		digitalWrite(redLed, 1);	//off
		digitalWrite(grnLed, 1);
		digitalWrite(bluLed, 1);
		
		delay(2000);
		
		digitalWrite(lightPin, 0);	//off
		digitalWrite(foggerPin, 0);
		digitalWrite(fanPin, 0);
		
		digitalWrite(peltierPin, 0);
		
		digitalWrite(circPump, 0);
		digitalWrite(airstone, 0);
		digitalWrite(redLed, 0);	//off
		digitalWrite(grnLed, 0);
		digitalWrite(bluLed, 0);
		
		delay(2000);
	}
	
	return 0;
}

