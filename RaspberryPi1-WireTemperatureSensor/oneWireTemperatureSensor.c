/* Pin connections from DS18B20 to Raspberry Pi(Looking at the flat side of the DS18B20)
 * Left pin to ground
 * Middle pin to GPIO4 (Also 4.7k resistor to +3.3V)
 * Right pin to +3.3V
 *
 * MAKE SURE TO RUN THESE COMMANDS IN THE TERMINAL FIRST!
 * sudo modprobe w1-gpio
 * sudo modprobe w1-therm
 */

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int main (void) {
	DIR *dir;
	struct dirent *dirent;
	char dev[16];		// Device ID
	char devPath[128];	// Path to device
	char buf[256];		// Data from device
	char tmpData[6];	// Temperature in Celsius * 1000 reported by device
	char path[] = "/sys/bus/w1/devices";
	ssize_t numRead;
	dir = opendir (path);
	if (dir != NULL) {
		while ((dirent = readdir (dir)))	// 1-wire devices are links beginning with "28-"
		if (dirent->d_type == DT_LNK && strstr(dirent->d_name, "28-") != NULL) {
			strcpy(dev, dirent->d_name);
			printf("\nDevice: %s\n", dev);
		}
		(void) closedir (dir);
	}
	else {
		perror ("Couldn't open the w1 devices directory");
		return 1;
	}
	sprintf(devPath, "%s/%s/w1_slave", path, dev);	// Assemble path to OneWire device
	while(1) {	// Read temp continuously opening the device's file triggers new reading
		int fd = open(devPath, O_RDONLY);
		if(fd == -1) {
			perror ("Couldn't open the w1 device.");
			return 1;
		}
		while((numRead = read(fd, buf, 256)) > 0) {
			strncpy(tmpData, strstr(buf, "t=") + 2, 5);
			float tempC = strtof(tmpData, NULL);
			printf("Device: %s  - ", dev);
			printf("Temp: %.3f C  ", tempC / 1000);
			printf("%.3f F\n", (tempC / 1000) * 9 / 5 + 32);
		}
		close(fd);
	}
	// return 0;	//never called due to loop
}
