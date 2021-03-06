//--------------------
//thingspeak headers
#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <wiringPi.h>
#include <unistd.h> 
#include <sys/ioctl.h> 
#define w1_master_slaves "/sys/bus/w1/devices/w1_master_slaves" 
#define w1_master_slave_count "/sys/bus/w1/devices/onew1_bus_master_slave_count"
#define one_wire_device_p1 "/sys/bus/w1/devices/" 
#define one_wire_device_p2 "/w1_slave" 
#define IP "184.106.153.149" //thingspeak.com 
#define GET "GET /update?key=";
#define field1 "&1=" 
#define field2 "&2=" 
#define field3 "&3=" 
#define field4 "&4=" 
#define field5 "&5=" 
#define field6 "&6="
#define CRLF "\r\n" 
//--------------------

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
//ADC
#include <sys/ioctl.h>

#include <string.h>

//time call functions
int minCall(void);
int hourCall(void);

//thingspeak code to send variables to thingspeak -----

int thingSpeakFunction(int boxHumid, int tubHumid, int boxTemp, int tubTemp, float pHtest);

//---------------------------
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
	char lightStartField[255];
	int lightStartValue;
	char lightEndField[255];
	int lightEndValue;
	char foggerStartField[255];
	int foggerStartValue;
	char foggerEndField[255];
	int foggerEndValue;
	char plantTempMinField[255];
	int plantTempMinValue;
	char plantTempMaxField[255];
	int plantTempMaxValue;
	char rootTempMinField[255];
	int rootTempMinValue;
	char rootTempMaxField[255];
	int rootTempMaxValue;
	char pHMinField[255];
	int pHMinValue;
	char pHMaxField[255];
	int pHMaxValue;
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
	int begLED = 2;			//LED start hour 			<--set LED start
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
		begLED = CSVdata.lightStartValue;			//LED start hour 			<--set LED start
		endLED = CSVdata.lightEndValue;
		boxTempMax = CSVdata.plantTempMaxValue;		//Celsius max desired heat 	<--- set box max temp
		tubTempMax = CSVdata.rootTempMaxValue;
		DCon = CSVdata.foggerStartValue;			//fogger on time			<--- set fog on
		DCoff = CSVdata.foggerEndValue;
		pHmin = CSVdata.pHMinValue;		//min desired pH			<---set pH min
		pHmax = CSVdata.pHMaxValue;
		
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
			{doorFlag = 0;}
		else
			{doorFlag = 1;}
			
			
			
		if (mn != minCall())	//if a minute has passed
		{
			mn = minCall();	//resets tracker
			hr = hourCall();	//resets tracker
			
			//code to update temp&humidity in thingspeak<-------------
			
			if (DCon > fogTracker)
			{
				digitalWrite(foggerPin, 1);	//fog on
				fogTracker++;	//increments tracker
			}
			else
			{
				digitalWrite(foggerPin, 0);	//fog off
				fogTracker++;	//increments tracker
				if (fogTracker >= (DCon + DCoff))
				{fogTracker = 0;}
			}
			//code to update thingspeak<-------------
			
			pHtest = (((-17)*adc0()) + 9.1);	//read analog
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
			
	thingSpeakFunction(boxHumid, tubHumid, boxTemp, tubTemp, pHtest);
	
	}//end while
	
	return 0;
} 

int thingSpeakFunction(int boxHumid, int tubHumid, int boxTemp, int tubTemp, float pHtest)
{
	{
	int socket_desc;
    struct sockaddr_in server;
    char msg[80]="";
    char boxHumidString[6];
    char tubHumidString[6];
    char boxTempString[6];
    char tubTempString[6];
    char pHtestString[6];
    
    sprintf(tubTempString, "%i", tubTemp);
    sprintf(boxHumidString, "%i", boxHumid);
    sprintf(tubHumidString, "%i", tubHumid);
    sprintf(boxTempString, "%i", boxTemp);
    
    sprintf(pHtestString, "%0.2f", pHtest);
     
    printf("tubTemp:%i \n", tubTemp);
	printf("tubTempString:%s \n", tubTempString);
	
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    
    
         
    server.sin_addr.s_addr = inet_addr(IP);
    server.sin_family = AF_INET;
    server.sin_port = htons( 80 );
 
    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        printf("connect error");
        
    }
     
    printf("Connected\n");
    
    strcat(msg,"GET /update?key=XHFZ3J83YP9VTK3O");
    strcat(msg, field1);
    strcat(msg, boxHumidString);
    strcat(msg, field2);
    strcat(msg, tubHumidString);
    strcat(msg, field3);
    strcat(msg, boxTempString); 
    strcat(msg, field6);
    strcat(msg, tubTempString);
    strcat(msg, field5);
    strcat(msg, pHtestString); 
    strcat(msg, CRLF); 
    
    printf("%s\n", msg); 
     
    //Send some data
 
    if( send(socket_desc , msg , strlen(msg) , 0) < 0)
    {
        puts("Send failed");
        //return 1;
    }
    puts("Data Sent\n");
}
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
	FILE *CSVfile = fopen("/home/pi/Desktop/botny.csv", "r");
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
	else if(strcmp(token, "lightStart") == 0) {
		strcpy(CSVdata.lightStartField, token);
		//printf(" Field: %s\n", CSVdata.lightStartField);

		token = strtok(NULL, delimiter);
		CSVdata.lightStartValue = atoi(token);
		//printf(" Value: %i\n", CSVdata.lightStartValue);
	}
	else if(strcmp(token, "lightEnd") == 0) {
		strcpy(CSVdata.lightEndField, token);
		//printf(" Field: %s\n", CSVdata.lightEndField);

		token = strtok(NULL, delimiter);
		CSVdata.lightEndValue = atoi(token);
		//printf(" Value: %i\n", CSVdata.lightEndValue);
	}
	else if(strcmp(token, "foggerStart") == 0) {
		strcpy(CSVdata.foggerStartField, token);
		//printf(" Field: %s\n", CSVdata.foggerStartField);

		token = strtok(NULL, delimiter);
		CSVdata.foggerStartValue = atoi(token);
		//printf(" Value: %i\n", CSVdata.foggerStartValue);
	}
	else if(strcmp(token, "foggerEnd") == 0) {
		strcpy(CSVdata.foggerEndField, token);
		//printf(" Field: %s\n", CSVdata.foggerEndField);

		token = strtok(NULL, delimiter);
		CSVdata.foggerEndValue = atoi(token);
		//printf(" Value: %i\n", CSVdata.foggerEndValue);
	}
	else if(strcmp(token, "plantTempMin") == 0) {
		strcpy(CSVdata.plantTempMinField, token);
		//printf(" Field: %s\n", CSVdata.plantTempMinField);

		token = strtok(NULL, delimiter);
		CSVdata.plantTempMinValue = atoi(token);
		//printf(" Value: %i\n", CSVdata.plantTempMinValue);
	}
	else if(strcmp(token, "plantTempMax") == 0) {
		strcpy(CSVdata.plantTempMaxField, token);
		//printf(" Field: %s\n", CSVdata.plantTempMaxField);

		token = strtok(NULL, delimiter);
		CSVdata.plantTempMaxValue = atoi(token);
		//printf(" Value: %i\n", CSVdata.plantTempMaxValue);
	}
	else if(strcmp(token, "rootTempMin") == 0) {
		strcpy(CSVdata.rootTempMinField, token);
		//printf(" Field: %s\n", CSVdata.rootTempMinField);

		token = strtok(NULL, delimiter);
		CSVdata.rootTempMinValue = atoi(token);
		//printf(" Value: %i\n", CSVdata.rootTempMinValue);
	}
	else if(strcmp(token, "rootTempMax") == 0) {
		strcpy(CSVdata.rootTempMaxField, token);
		//printf(" Field: %s\n", CSVdata.rootTempMaxField);

		token = strtok(NULL, delimiter);
		CSVdata.rootTempMaxValue = atoi(token);
		//printf(" Value: %i\n", CSVdata.rootTempMaxValue);
	}
	else if(strcmp(token, "pHMin") == 0) {
		strcpy(CSVdata.pHMinField, token);
		//printf(" Field: %s\n", CSVdata.pHMinField);

		token = strtok(NULL, delimiter);
		CSVdata.pHMinValue = atoi(token);
		//printf(" Value: %i\n", CSVdata.pHMinValue);
	}
	else if(strcmp(token, "pHMax") == 0) {
		strcpy(CSVdata.pHMaxField, token);
		//printf(" Field: %s\n", CSVdata.pHMaxField);

		token = strtok(NULL, delimiter);
		CSVdata.pHMaxValue = atoi(token);
		//printf(" Value: %i\n", CSVdata.pHMaxValue);
	}
	else {
		//printf(" Error: field unspecified: %s\n", token);
	}
	return 0;
}
