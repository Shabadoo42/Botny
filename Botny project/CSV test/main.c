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
#include <wiringPi.h>

struct data {
	char field[255];
	int value;
}CSVdata;

void readCSVdata();
int checkToken(char *token, char *delimiter);

int main() {
	printf("Setting up GPIO:");
	wiringPiSetupGpio();				// Setup GPIO using CPU names
	const unsigned char GPIOpin = 6;	// LED connected to GPIO pin 18
	pinMode(GPIOpin, OUTPUT);			// GPIO pin 18 output
	digitalWrite(GPIOpin, 1);
	printf(" Done.\n");

	while(1) {
		readCSVdata();
		printf("Press ctrl + C to exit.\n");
		digitalWrite(GPIOpin, 1);	// LED on
		delay(CSVdata.value);		// Time on
		digitalWrite(GPIOpin, 0);	// LED off
		delay(CSVdata.value);		// Off time
	}
	return 0;
}

void readCSVdata(void) {
	printf("Reading CSV file:\n");
	FILE *CSVfile = fopen("/home/pi/Documents/Botny project/CSV test/CSVtestfile.csv", "r");
	char buffer[255];
	fgets(buffer, sizeof(buffer), CSVfile);
	fclose(CSVfile);

	char *delimiter = ",";
	char *token = strtok(buffer, delimiter);
	checkToken(token, delimiter);
	while(token != NULL) {
		token = strtok(NULL, delimiter);
		checkToken(token, delimiter);
	}
	printf(" Done.\n");
}

int checkToken(char *token, char *delimiter) {
	if(token == NULL) {
		return 0;
	}
	else if(strcmp(token, "blinkDelay") == 0) {
		strcpy(CSVdata.field, token);
		printf(" Field: %s\n", CSVdata.field);

		token = strtok(NULL, delimiter);
		CSVdata.value = atoi(token);
		printf(" Value: %i\n", CSVdata.value);
	}
	else {
		printf(" Error: field unspecified: %s\n", token);
	}
	return 0;
}
