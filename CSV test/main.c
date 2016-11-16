/* RASPBERRY PI CSV TEST PROGRAM
 *
 * Author: Logen De Bruyne
 *
 * This program is designed to test the parsing of a CSV file and use the information
 * enclosed to update a variable (Delay).
 *
 * The CSV file has a field name (Delay) and a value in milliseconds that will
 * be used to change the blink time of an LED connected to GPIO pin 18 on the
 * Raspberry Pi.
 */

#include <stdio.h>
#include <wiringPi.h>	// The WiringPi header is needed to use the Raspberry Pi GPIO pins

int main(void) {
	wiringPiSetupGpio();		// Setup GPIO pins using CPU naming convention
	unsigned char pin = 18;	// LED connected to GPIO pin 18
	pinMode(pin, OUTPUT);		// Set GPIO pin 18 to output

	unsigned short blinkDelay = 1000;

	printf("LED blinking\n");
	printf("Press ctrl + C to exit\n");

	while(1) {
		digitalWrite(pin, 1);	// Turn LED on
		delay(blinkDelay);			// Time on
		digitalWrite(pin, 0);	// Turn LED off
		delay(blinkDelay);			// Off time
	}

	return 0;
}
