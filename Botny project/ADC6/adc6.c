//asd1115.c read TMP37 temp sensor ANC0
//operates in continuous mode
//from https://www.youtube.com/watch?v=OPC5lXCKp_w

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>	//read/write usleep
#include <stdlib.h>	//exit function
#include <inttypes.h>	//uint8_t, etc
#include <linux/i2c-dev.h>	//i2c bus defintions

//says connect ADR to GRD ...but doesn't seem to make a difference
//setup to use ADC0 single ended

//Things to do: switch to 12-bit, kill ending?, fix power down?

int fd;
//note PCF8591 defaults to 0x48
int asd_address = 0x48;
int16_t val;		//keeps value
uint8_t writeBuf[3];
uint8_t readBuf[2];
float myFloat;

const float VPS = 6.144/32768.0; 		//volts per step   was 4.096

//for a 16 bit, but uses 15 because of sign

int main()
{
	//open device on /dev/i2c-1 the default on Rasp Pi B
	if ((fd = open("/dev/i2c-1", O_RDWR)) < 0)
	{
		printf("Error: couldn't open device %d\n", fd);
		exit (1);
	}
	
	//connect to ADS1115 as i2c slave
	if (ioctl(fd, I2C_SLAVE, asd_address) < 0)
	{
		printf("Error: Couldn't find device on this address \n");
		exit (1);
	}
	
	//set config register and start conversation
	//AIN0 and GND, 4.096v, 128s/s
	//refer to p19 area of spec sheet
	writeBuf[0] = 1;	//config register is 1
	writeBuf[1] = 0b11000000;	//0xC2 single shot off		was 0b11000010
	//bit 15 flag bit for single shot not used here
	//bits 14-12 input selection					0b11010000 for AN1
	//100 ANC0, 101 ANC1, 110 ANC2, 111 ANC3		0b11000000 for AN0
	//bits 11-9 amp gain. default to 010 here 001 p19
	//bit 8 operational mode of the ads1115
	// 0 : continuous conversion mode
	// 1 : power-down single-shot mode (default)
	
	writeBuf[2] = 0b10000101;	//bits 7-0 0x85
	//bits 7-5 data rate default to 100 for 128sps
	//bits 4-0 comparator functions
	
	//begin conversation
	if (write(fd, writeBuf, 3) != 3)
	{
		perror("Write to register 1");
		exit (1);
	}
	
	sleep(1);
	
	//set pointer to 0
	readBuf[0] = 0;
	if (write(fd, readBuf, 1) != 1)
	{
		perror("Write register select");
		exit(-1);
	}
	
	//read conversion register
	if (read(fd, readBuf, 2) != 2)
	{
		perror("Read conversion");
		exit(-1);
	}
	
	//could also multiply by 256 then add readBuf[1]
	val = readBuf[0] << 8 | readBuf[1];
	
	//with +- LSB sometimes generates very low neg number
	if (val < 0) val = 0;
	
	myFloat = val * VPS; //convert to voltage
	
	printf("%4.3f volts \n", myFloat);
	
	sleep(5);
	
	//power down ASD1115
	writeBuf[0] = 1;	//config register is 1
	writeBuf[1] = 0b11000011;	//bits 15-8 0xC3 single shot on
	writeBuf[2] = 0b10000101;	//bits 7-0 0x85
	if (write(fd, writeBuf, 3) != 3)
	{
		perror("Write to register 1");
		exit (1);
	}
	
	close(fd);
	
	//round2
	//open device on /dev/i2c-1 the default on Rasp Pi B
	if ((fd = open("/dev/i2c-1", O_RDWR)) < 0)
	{
		printf("Error: couldn't open device %d\n", fd);
		exit (1);
	}
	
	//connect to ADS1115 as i2c slave
	if (ioctl(fd, I2C_SLAVE, asd_address) < 0)
	{
		printf("Error: Couldn't find device on this address \n");
		exit (1);
	}
	
	//set config register and start conversation
	//AIN0 and GND, 4.096v, 128s/s
	//refer to p19 area of spec sheet
	writeBuf[0] = 1;	//config register is 1
	writeBuf[1] = 0b11010000;	//0xC2 single shot off		was 0b11000010
	//bit 15 flag bit for single shot not used here
	//bits 14-12 input selection					0b11010000 for AN1
	//100 ANC0, 101 ANC1, 110 ANC2, 111 ANC3		0b11000000 for AN0
	//bits 11-9 amp gain. default to 010 here 001 p19
	//bit 8 operational mode of the ads1115
	// 0 : continuous conversion mode
	// 1 : power-down single-shot mode (default)
	
	writeBuf[2] = 0b10000101;	//bits 7-0 0x85
	//bits 7-5 data rate default to 100 for 128sps
	//bits 4-0 comparator functions
	
	//begin conversation
	if (write(fd, writeBuf, 3) != 3)
	{
		perror("Write to register 1");
		exit (1);
	}
	
	sleep(1);
	
	//set pointer to 0
	readBuf[0] = 0;
	if (write(fd, readBuf, 1) != 1)
	{
		perror("Write register select");
		exit(-1);
	}
	
	//read conversion register
	if (read(fd, readBuf, 2) != 2)
	{
		perror("Read conversion");
		exit(-1);
	}
	
	//could also multiply by 256 then add readBuf[1]
	val = readBuf[0] << 8 | readBuf[1];
	
	//with +- LSB sometimes generates very low neg number
	if (val < 0) val = 0;
	
	myFloat = val * VPS; //convert to voltage
	
	printf("%4.3f volts \n", myFloat);
	
	sleep(5);
	
	//power down ASD1115
	writeBuf[0] = 1;	//config register is 1
	writeBuf[1] = 0b11000011;	//bits 15-8 0xC3 single shot on
	writeBuf[2] = 0b10000101;	//bits 7-0 0x85
	if (write(fd, writeBuf, 3) != 3)
	{
		perror("Write to register 1");
		exit (1);
	}
	
	close(fd);
	
	
	return 0;
}

