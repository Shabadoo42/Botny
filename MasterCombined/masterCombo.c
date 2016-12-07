//adc
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>	//read/write usleep
#include <stdlib.h>	//exit function
#include <inttypes.h>	//uint8_t, etc
#include <linux/i2c-dev.h>	//i2c bus defintions
//ths
#include <wiringPi.h>
#include <stdint.h>
//timecall
#include <time.h>
#include <sys/ioctl.h>

#include <string.h>

//time call functions
int minCall(void);
int hourCall(void);
//ADC functions
int fd;
int asd_address = 0x48;		//note PCF8591 defaults to 0x48
int16_t val;				//keeps value
uint8_t writeBuf[3];
uint8_t readBuf[2];
float anal0;				//keeps analog value
float anal1;
const float VPS = 6.144/32768.0; 		//volts per step   was 4.096
float adc0();
float adc1();
//THS functions
#define MAXTIMINGS	85
int DHTPIN	=	22; //23 for other
int dht11_dat[5] = { 0, 0, 0, 0, 0 };
void read_dht11_dat();
int Humidity = 0;		//current humid
int Temperature = 0;	//current temp

struct data {
	char field[255];
	int value;
}CSVdata;

void readCSVdata();
int checkToken(char *token, char *delimiter);

int main()
{
	//PIN VARIABLES
	int lightPin = 17;		
	int foggerPin = 4;
	int fanPin = 15;
	int peltierPin = 21;
	int pwmPin = 18;
	int circPump = 20;
	int airstone = 9;
	int redLed = 19;
	int grnLed = 13;
	int bluLed = 6;
	int doorPin =24;
	//int pwmHI = 1024; //max value 1024
	//int pwmLO = 512; //max value 1024
	
	//OUTPUTS
	pinMode(lightPin, OUTPUT);
	pinMode(foggerPin, OUTPUT);
	pinMode(fanPin, OUTPUT);
	pinMode(peltierPin, OUTPUT);
	pinMode(pwmPin, PWM_OUTPUT); // Set PWM LED as PWM output
	pinMode(circPump, OUTPUT);
	pinMode(airstone, OUTPUT);
	pinMode(redLed, OUTPUT);
	pinMode(grnLed, OUTPUT);
	pinMode(bluLed, OUTPUT);
	pinMode(doorPin, INPUT);
	
	//MAIN VARIABLES
	//int hr = hourCall();	//set time initially
	//int mn = minCall();
	int hr = 99;			//offset to check everything at beginning
	int mn = 99;			//offset to check everything at beginning
	int begLED = 6;			//LED start hour 			<--set LED start
	int endLED = 23;		//LED end hour				<--set LED end
	int boxHumid = 0;		//box humidity
	int tubHumid = 0;		//box humidity
	int boxTemp = 0;		//box temp
	int tubTemp = 0;		//box temp
	int boxTempMax = 4;		//Celsius max desired heat 	<--- set box max temp
	int tubTempMax = 25;	//Celsius max desired heat 	<--- set tub amx temp
	int DCon = 2;			//fogger on time			<--- set fog on
	int DCoff = 2;			//fogger off time			<--- set fog off
	int fogTracker = 0;		//tracks which minute in duty cycle
	float pHtest = 0;		//intial ADC readings
	float pHmin = 0;		//min desired pH			<---set pH min
	float pHmax = 9;		//max desired pH			<---set pH max
	//float waterLevel = 0;	//not needed anymore
	//float waterMin = 0.5;	//minimum water level
	int hotFlag = 0;		//flag for hottub
	int acidFlag = 0;		//flag for pH
	int doorFlag = 0;		//flag for water level
	
	//sets up dht11 sensor and pins
	if ( wiringPiSetupGpio() == -1 )
			{exit( 1 );}
			
	digitalWrite(lightPin, 0);
	digitalWrite(foggerPin, 0);
	digitalWrite(fanPin, 0);
	digitalWrite(peltierPin, 0);
	digitalWrite(circPump, 0);
	digitalWrite(airstone, 1);
	digitalWrite(redLed, 1);
	digitalWrite(grnLed, 1);
	digitalWrite(bluLed, 1);
	
	while(1)
	{
		readCSVdata();
		//TEMPERATURE SENSORS -> FANS n PELTIERS
		//checks box temp every cycle and adjusts fan
		DHTPIN = 22;			//switch to box pin
		read_dht11_dat();		//read box data
		boxHumid = Humidity;	//update box humidity values
		boxTemp = Temperature;	//update box temperature values
		//if too hot turn on fans
		if (boxTempMax < boxTemp)
		{
			digitalWrite(fanPin, 1);
			if (boxTemp - boxTempMax > 2) 
				{
					//pwmWrite(pwmPin, pwmHI);	// PWM High when 2+ degrees over
					digitalWrite(fanPin, 1);
				} 
			else
				{
					//pwmWrite(pwmPin, pwmLO); // PWM Low when 1 degree over
					digitalWrite(fanPin, 1);
				}
		}
		else
		{
			digitalWrite(fanPin, 0);
			//pwmWrite(pwmPin, 0); // PWM LED at dim setting
		}
		//checks tub temp every cycle and adjusts peltier
		DHTPIN = 23;			//switch to tub pin
		read_dht11_dat();	
		tubHumid = Humidity;
		tubTemp = Temperature;
		//if too hot turn on peltier & circ pump
		if (tubTempMax < tubTemp)
		{
			digitalWrite(peltierPin, 1);	//turns on cooler if too hot
			digitalWrite(circPump, 1);		//turns on circ pump
			if (tubTemp > (tubTempMax + 2))
				{hotFlag = 1;}				//if two degress hotter than Max, it flags LED
			else
				{hotFlag = 0;}
		}	
		else
		{
			digitalWrite(peltierPin, 0);	//turns off cooler & flag
			digitalWrite(circPump, 0);		//turns off circ pump
			hotFlag = 0;
		}	
		
		//door trigger code
		if (digitalRead(doorPin) == 1)
			{doorFlag = 1;}
		else
			{doorFlag = 0;}
			
			
			
		if (mn != minCall())	//if a minute has passed
		{
			mn = minCall();	//resets tracker
			hr = hourCall();	//resets tracker
			
			//code to update temp&humidity in thingspeak<-------------
			
			if (DCon > fogTracker)
			{
				digitalWrite(peltierPin, 1);	//fog on
				fogTracker++;	//increments tracker
			}
			else
			{
				digitalWrite(peltierPin, 0);	//fog off
				fogTracker++;	//increments tracker
				if (fogTracker >= (DCon + DCoff))
				{fogTracker = 0;}
			}
			//code to update thingspeak<-------------
			
			pHtest = adc0();	//read analogs
			if (pHtest <= pHmin || pHtest >= pHmax)
				{acidFlag = 1;}		//LED flag
			else
				{acidFlag = 0;}
			
			/*
			 * old water level sensor code
			waterLevel = adc1();
			if (pHtest <= waterMin)
				{wetFlag = 1;}		//LED flag
			else
				{wetFlag = 0;}
			*/
			
			
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
			
		}//end min if
		
		/* CAN USE IF YOU NEED SOMETHING CHECKED HOURLY
		if (hr != hourCall())	//if an hour has passed
		{
			hr = hourCall();	//resets tracker	
		}//end hr if
		*/
		if (hotFlag == 1 && doorFlag == 1)
		{
			digitalWrite(redLed, 0);	//purple
			digitalWrite(grnLed, 1);
			digitalWrite(bluLed, 0);
		}
		else if (hotFlag == 1)
		{
			digitalWrite(redLed, 0);	//red
			digitalWrite(grnLed, 1);
			digitalWrite(bluLed, 1);
		}
		else if (doorFlag == 1)
		{
			digitalWrite(redLed, 1);	//blue
			digitalWrite(grnLed, 1);
			digitalWrite(bluLed, 0);
		}
		else if (acidFlag == 1)
		{
			digitalWrite(redLed, 0);	//orangish
			digitalWrite(grnLed, 0);
			digitalWrite(bluLed, 1);
		}
		else
		{
			digitalWrite(redLed, 1);	//green means okay!
			digitalWrite(grnLed, 0);
			digitalWrite(bluLed, 1);
		}
		
		delay(2000);	//why the hurry?
		
		printf("time%i:%i pH:%4.2f boxHumid:%i boxTemp:%i tubHumid:%i tubTemp:%i \n"
			, hr, mn, pHtest, boxHumid, boxTemp, tubHumid, tubTemp);	//update serial monitor
		printf("door:%i hot:%i Acid:%i\n"
			, doorFlag, hotFlag, acidFlag);
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
	int whileCounter = 0;
 
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
			Humidity = dht11_dat[0];	//captures humidity 
			Temperature = dht11_dat[2];	//captures temperature
	}else  {
		//write in code to break if too many cycles!!!!!!!!1
		printf( "Data not good, skip\n" );	//delete after troubleshooting <-----------
		//printf("CS%i\n",j);
	}
	if (whileCounter >= 1) break;
	delay(1000);
}//while end
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
	else if(strcmp(token, "herp") == 0) {
		printf(" Field: %s\n", token);

		token = strtok(NULL, delimiter);
		int number = atoi(token);
		printf(" Value: %i\n", number);
	}
	else {
		printf(" Error: field unspecified: %s\n", token);
	}
	return 0;
}
