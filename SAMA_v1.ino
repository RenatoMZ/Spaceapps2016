//This program reads the value of two FC22-MQ135 CO2 sensors, two Grove-Gas sensor O2 and a flowmeter YF S201
#include "MQ135.h"
#include <SoftwareSerial.h>

//Declare variables

//For O2 sensors
float O2_Inh = 0; // 4 bytes
float O2_Exh = 0; // 4 bytes

//For CO2 sensors
MQ135 gasSensorInh = MQ135(A2);
MQ135 gasSensorExh = MQ135(A3);
float CO2_Inh = 0; // 4 bytes
float CO2_Exh = 0; // 4 bytes

//For flowmeter

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;
float calibrationFactor = 4.5; // 4 bytes
volatile byte pulseCount;  
float flowRate; // 4 bytes
unsigned int flowMilliLitres; // Current Liquid Flowing. 2 bytes
unsigned long totalMilliLitres; // Output Liquid Quantity. 4 bytes
unsigned long oldTime; // 4 bytes

SoftwareSerial BTSerial(10, 11); // RX, TX

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  BTSerial.begin(9600);

  //For flowmeter
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;

  // The Hall-effect sensor is connected to pin 2 which uses interrupt 0.
  // Configured to trigger on a FALLING state change (transition from HIGH
  // state to LOW state)
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING); 
  Serial.write(BTSerial.read()); //Send "Client Connected" from device to Monitor Serial

}

void loop() {
  //O2 Sensor reading and conversion
  O2_Inh = analogRead(A0);
  O2_Exh = analogRead(A1);
  O2_Inh = (((O2_Inh/1024)*5)/201*10000)/7.43;  // %O2 inhaled
  O2_Exh = (((O2_Exh/1024)*5)/201*10000)/7.43;  // %O2 exhhaled  

  //CO2 Sensor reading and conversion
  CO2_Inh = gasSensorInh.getPPM();
  CO2_Exh = gasSensorExh.getPPM();
  CO2_Inh = CO2_Inh/10000;  // %CO2 inhaled
  CO2_Exh = CO2_Exh/10000;  // %CO2 exhaled  
  //Flow sensor
  if((millis() - oldTime) > 1000)    // Only process counters once per second
  { 
    // Disable the interrupt while calculating flow rate and sending the value to
    // the host
    detachInterrupt(sensorInterrupt);
        
    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    // Note the time this processing pass was executed. Note that because we've
    // disabled interrupts the millis() function won't actually be incrementing right
    // at this point, but it will still return the value it was set to just before
    // interrupts went away.
    oldTime = millis();
    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;
    
    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    
    unsigned int frac;    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print(".");             // Print the decimal point
    // Determine the fractional part. The 10 multiplier gives us 1 decimal place.
    frac = (flowRate - int(flowRate)) * 10;
    Serial.print(frac, DEC) ;      // Print the fractional part of the variable
    Serial.print("L/min");
       
    String StringflowRate = String(int(flowRate)) + String(frac,DEC);
    BTSerial.print(StringflowRate);//Send Flow Rate by BT
    
    // Print the number of litres flowed in this second
    Serial.print("  Current Liquid Flowing: ");             // Output separator
    Serial.print(flowMilliLitres);
    Serial.print("mL/Sec");
    BTSerial.write(flowMilliLitres);//Send Current Liquid Flowing by BT
    
    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");             // Output separator
    Serial.print(totalMilliLitres);
    Serial.println("mL");
    Serial.println(" ");
    BTSerial.write(totalMilliLitres);//Send Output Liquid Quantity by BT
    
    // Reset the pulse counter so we can start incrementing again
    pulseCount = 0;
    
    // Enable the interrupt again now that we've finished sending output
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }

  Serial.print("Concentration of O2 inhaled is ");
  Serial.print(O2_Inh,1);
  Serial.println("%");
  BTSerial.write(O2_Inh);//Send O2 inhaled by BT
  
  Serial.print("Concentration of CO2 inhaled is ");
  Serial.print(CO2_Inh,1);
  Serial.println("%");
  Serial.println(" ");
  BTSerial.write(CO2_Inh);//Send CO2 inhaled by BT
  
  Serial.print("Concentration of O2 exhaled is ");
  Serial.print(O2_Exh,1);
  Serial.println("%");
  BTSerial.write(O2_Exh);//Send O2 exhaled by BT
  
  Serial.print("Concentration of CO2 exhhaled is ");
  Serial.print(CO2_Exh,1);
  Serial.println("%");
  Serial.println(" ");
  BTSerial.write(CO2_Exh);//Send CO2 exhaled by BT
  Serial.println("-------------------------------------------");

  delay(1000);
}

/*
Insterrupt Service Routine
 */
void pulseCounter()
{
  // Increment the pulse counter
  pulseCount++;
}
