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

//time call functions
int minCall(void);
int hourCall(void);
//ADC functions
int fd;
int asd_address = 0x48;		//note PCF8591 defaults to 0x48
int16_t val;				//keeps value
uint8_t writeBuf[3];
uint8_t readBuf[2];
float anal0;
float anal1;
const float VPS = 6.144/32768.0; 		//volts per step   was 4.096
float adc0();
float adc1();
//THS functions
#define MAXTIMINGS	85
#define DHTPIN		7 //GPIO 4
int dht11_dat[5] = { 0, 0, 0, 0, 0 };
int whileCounter = 0;
void read_dht11_dat();

//GLOBALS
int upHumid = 0;	//box humid
int upTemp = 0;		//box temp
//int downTemp; //box temp

//issues: pin control with wiringPiSetupGpio, multiple pins with THS
int main()
{
	//wiringPiSetupGpio();
	
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
	int dcon = 2;	//fogger on time
	int dcoff = 2;	//fogger off time
	int fogTracker = 0;	//tracks which minute in duty cycle
	float pHtest = adc0();	//intial ADC readings
	float waterLevel = adc1();
	
	digitalWrite(peltierPin, 0);	//starts pin at 0
	
	if ( wiringPiSetup() == -1 )	//sets up dht11 THsensor
			{exit( 1 );}
	
	while(1)
	{
		printf("time%i:%i AN0:%4.2f AN1:%4.2f humid:%i temp:%i  \n", hr, mn, pHtest, waterLevel, upHumid, upTemp);
		
			//read_dht11_dat();	//for testing only
		
		if (mn != minCall())	//if a minute has passed
		{
			mn = minCall();	//resets tracker
			
			read_dht11_dat();	//check box temp
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


//TIME CALL FUNCTIONS
int hourCall(void)
{
	
	time_t timeCall;
    char timeBuffer[7];
    struct tm* tm_info;

    time(&timeCall);
    tm_info = localtime(&timeCall);

    strftime(timeBuffer, 7, "%H%M%S", tm_info);
    //puts(timeBuffer);	//writes string
	
	//rintf("%s\n",timeBuffer);	//print string
	
	int hr10 = 10*(timeBuffer[0] - '0');
	int hr01 = timeBuffer[1] - '0';
	int hr = hr10 + hr01;	//hr represents hour

	
	//printf("%i:%i\n", hr, mn);
	
	return hr;
}

int minCall(void)
{
	
	time_t timeCall;
    char timeBuffer[7];
    struct tm* tm_info;

    time(&timeCall);
    tm_info = localtime(&timeCall);

    strftime(timeBuffer, 7, "%H%M%S", tm_info);
    //puts(timeBuffer);	//writes string
	
	//rintf("%s\n",timeBuffer);	//print string
	

	int mn10 = 10*(timeBuffer[2] - '0');
	int mn01 = timeBuffer[3] - '0';
	int mn = mn10 + mn01;	//mn represents minute
	
	//printf("%i:%i\n", hr, mn);
	
	return mn;
}


//ADC functions

//asd1115.c read TMP37 temp sensor ANC0
//operates in continuous mode
//from https://www.youtube.com/watch?v=OPC5lXCKp_w



//says connect ADR to GRD ...but doesn't seem to make a difference
//setup to use ADC0 single ended

//Things to do: switch to 12-bit, kill ending?, fix power down?

float adc0()
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
	
	anal0 = val * VPS; //convert to voltage
	
	//printf("%4.3f volts \n", anal0);
	
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
	
	return anal0;
}

float adc1()
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
	
	anal1 = val * VPS; //convert to voltage
	
	//printf("%4.3f volts \n", anal1);
	
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
	
	return anal1;
}

//THS FUNCTIONS

void read_dht11_dat()
{
	while(1)
	{
	uint8_t laststate	= HIGH;
	uint8_t counter		= 0;
	uint8_t j		= 0, i;
	
 
	dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
 
	/* pull pin down for 18 milliseconds */
	pinMode( DHTPIN, OUTPUT );
	digitalWrite( DHTPIN, LOW );
	delay( 18 );
	/* then pull it up for 40 microseconds */
	digitalWrite( DHTPIN, HIGH );
	delayMicroseconds( 40 );
	/* prepare to read the pin */
	pinMode( DHTPIN, INPUT );
 
	/* detect change and read data */
	for ( i = 0; i < MAXTIMINGS; i++ )
	{
		counter = 0;
		while ( digitalRead( DHTPIN ) == laststate )
		{
			counter++;
			delayMicroseconds( 1 );
			if ( counter == 255 )
			{
				break;
			}
		}
		laststate = digitalRead( DHTPIN );
 
		if ( counter == 255 )
			break;
 
		/* ignore first 3 transitions */
		if ( (i >= 4) && (i % 2 == 0) )
		{
			/* shove each bit into the storage bytes */
			dht11_dat[j / 8] <<= 1;
			if ( counter > 16 )
				dht11_dat[j / 8] |= 1;
			j++;
		}
	}
 
	/*
	 * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
	 * print it out if data is good
	 */
	if ( (j >= 40) &&
	     (dht11_dat[4] == ( (dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF) ) )
	{
		//UNCOMMENT TO PRINT REGULARLY
		//printf( "Hum = %d.%d %% Temp = %d.%d *C\n", dht11_dat[0], dht11_dat[1], dht11_dat[2], dht11_dat[3]);
			whileCounter++;			//gi
			upHumid = dht11_dat[0];	//captures humidity 
			upTemp = dht11_dat[2];	//captures temperature
	}else  {
		//write in code to break if too many cycles!!!!!!!!1
		printf( "Data not good, skip\n" );
		//printf("CS%i\n",j);
	}
	if (whileCounter >= 1) break;
	delay(1000);
}//while end
}

