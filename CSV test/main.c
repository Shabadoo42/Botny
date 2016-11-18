/* RASPBERRY PI CSV TEST PROGRAM
 *
 * Author: Logen De Bruyne
 *
 * This program is designed to test the parsing of a CSV file and use
 * the information enclosed to update a variable (blinkDelay).
 *
 * The CSV file has a field name (blinkDelay) and a value in milliseconds
 * that will be used to change the blink time of an LED connected to
 * GPIO pin 18 on the
 * Raspberry Pi.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPi.h>	// Needed to use GPIO pins

struct data {
	char field[10];
	int value;
}CSVdata;

int readCSVData();

int main() {
	wiringPiSetupGpio();				// Setup GPIO using CPU names
	const unsigned char GPIOpin = 18;	// LED connected to GPIO pin 18
	pinMode(GPIOpin, OUTPUT);			// GPIO pin 18 output

	readCSVData();

	unsigned short blinkDelay;

	if(strcmp(CSVdata.field, "blinkDelay") == 0) {
		blinkDelay = CSVdata.value;
	}
	else {
		printf("Error: CSV file incorrect\n");
		return 0;
	}

	printf("LED blinking\n");
	printf("Press ctrl + C to exit\n");

	while(1) {
		digitalWrite(GPIOpin, 1);	// LED on
		delay(blinkDelay);			// Time on
		digitalWrite(GPIOpin, 0);	// LED off
		delay(blinkDelay);			// Off time
	}

	return 0;
}

int readCSVData(void) {
	FILE *CSVfile;
	char buffer[16];
	CSVfile = fopen("/home/pi/Documents/Botny Project/CSV test/CSVtestfile.csv", "r");
	fgets(buffer, 16, CSVfile);

	char *fieldData;
	fieldData = strtok(buffer, ",");
	printf("Field data: %s\n", fieldData);
	strcpy(CSVdata.field, fieldData);

	char *valueData;
	valueData = strtok(NULL, ",");
	printf("Value Data: %s\n", valueData);

	CSVdata.value = atoi(valueData);

	return 0;
}

