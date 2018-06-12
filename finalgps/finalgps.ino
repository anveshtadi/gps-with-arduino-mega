#include <Adafruit_GPS.h> 
#include <SoftwareSerial.h> 
#include <SD.h>
#define sensorInterrupt 0 
#define sensorPin 2
const float coeff = 98; 
#define stepTime 1000 //(ms) 
volatile uint16_t pulseCount;
float flowRate; // (L/min)
float flowRateMLS; // (mL/sec)
float flowCumVol; // (L or mL) Cumulative water volume which flow through flow meter.
unsigned long previousTime;
unsigned long currentTime;
unsigned long delTime;
SoftwareSerial mySerial(11, 12);
Adafruit_GPS GPS(&mySerial);
String NMEA1;  
String NMEA2;  
char c;       
int chipSelect = 53; 
File mySensorData;
float deg; 
float degWhole; 
float degDec;
void setup()  
{
  GPS.begin(9600);       
  GPS.sendCommand("$PGCMD,33,0*6D"); 
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   
  pinMode(sensorPin, INPUT);
  delay(1000);  //Pause
  pinMode(10, OUTPUT);
  SD.begin(chipSelect); 
  if (SD.exists("GPSData.txt")) { 
    SD.remove("GPSData.txt");
  }
  previousTime = 0;
  flowRate = 0;
  flowRateMLS = 0;
  flowCumVol = 0;
  pulseCount = 0;
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
}
void loop()                     // run over and over again
{
readGPS();  

}
void readGPS(){  //This function will read and remember two NMEA sentences from GPS
  clearGPS();    //Serial port probably has old or corrupt data, so begin by clearing it all out
  while(!GPS.newNMEAreceived()) { //Keep reading characters in this loop until a good NMEA sentence is received
  c=GPS.read(); //read a character from the GPS
  }
GPS.parse(GPS.lastNMEA());  //Once you get a good NMEA, parse it
NMEA1=GPS.lastNMEA();      //Once parsed, save NMEA sentence into NMEA1
while(!GPS.newNMEAreceived()) {  //Go out and get the second NMEA sentence, should be different type than the first one read above.
  c=GPS.read();
  }
  if((int)GPS.latitude!=200&&(int)GPS.longitude!=200&&GPS.fix==1)
   {
GPS.parse(GPS.lastNMEA());
NMEA2=GPS.lastNMEA();
  mySensorData = SD.open("GPSData.txt", FILE_WRITE);
  degWhole=float(int(GPS.longitude/100)); //gives me the whole degree part of Longitude
  degDec = (GPS.longitude - degWhole*100)/60; //give me fractional part of longitude
  deg = degWhole + degDec;
  mySensorData.print(deg,8);
  mySensorData.print(","); //write comma to SD card
  degWhole=float(int(GPS.latitude/100)); //gives me the whole degree part of latitude
  degDec = (GPS.latitude - degWhole*100)/60; //give me fractional part of latitude
  deg = degWhole + degDec; //Gives complete correct decimal form of latitude degrees
  if (GPS.lat=='S') {  //If you are in Southern hemisphere latitude should be negative
    deg= (-1)*deg;
  }
  mySensorData.print(deg,8);
  mySensorData.print(",");
  currentTime = millis(); //store millis() timing in currentTime
  delTime = currentTime-previousTime;
  if(delTime >= stepTime){ //measureFlow in each stepTime. For example: each 1 second)
    measureFlow();
  }
  mySensorData.print("  Flow Rate: "); mySensorData.print(flowRateMLS, 2); mySensorData.print(" mL/s");
  mySensorData.print("  Cumulative Vol: "); mySensorData.print(flowCumVol, 1); mySensorData.print(" mL");
  mySensorData.println("");
  mySensorData.close();
  }
}
void clearGPS() {  
while(!GPS.newNMEAreceived()) {
  c=GPS.read();
  }
GPS.parse(GPS.lastNMEA());
while(!GPS.newNMEAreceived()) {
  c=GPS.read();
  }
GPS.parse(GPS.lastNMEA());

}
void measureFlow(){
    detachInterrupt(sensorInterrupt); //detatch interrupt to make processor calculate the fluid equations below
    flowRate = (1000 * pulseCount / delTime) / coeff; //see above
    flowRateMLS = flowRate*1000/60; //(mL/s)  ////this 1000 is unit conversion 1000 milli = 1 meter.
    flowCumVol += (flowRateMLS/60)*(delTime/1000); // (mL/sec)*(sec) = mL   //this 1000 is unit conversion 1000 ms = 1 sec
    previousTime = currentTime;
    pulseCount = 0;
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING); 
}
void pulseCounter(){
  pulseCount++;
}
