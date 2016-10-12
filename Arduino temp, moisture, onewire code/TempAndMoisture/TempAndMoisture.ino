#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

int analogVal = A0;     // Moisture sensor Pin
int val = 0;           // variable to store the read value

void setup()
{
  /* add setup code here */
  pinMode(A0, INPUT); //A0 set as analog input
  Serial.begin(9600); //  setup serial
  
  Serial.println("Dallas Temperature IC Control Library Demo");
  
  sensors.begin();
}

void loop()
{

  
  Serial.print(" Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  Serial.print("Temperature for Device 1 is: ");
  Serial.print(sensors.getTempCByIndex(0));
  /* add main program code here */
  
  val = analogRead(analogVal); 
  
  /*Mapped out moisture reading values from the sensor I have, Analog value of 500 is completely saturated,
  where as a value of 1024 is completely dry - no moisture at all*/
  int moistureReading = map(val, 491, 1023, 100, -1); 

  delay(10000);

  Serial.println(moistureReading);
}

