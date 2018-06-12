#include <Adafruit_GPS.h>
#include <SD.h>

//Make sure to install the adafruit GPS library from https://github.com/adafruit/Adafruit-GPS-Library
#include <Adafruit_GPS.h> //Load the GPS Library. Make sure you have installed the library form the adafruit site above
#include <SoftwareSerial.h> //Load the Software Serial Library. This library in effect gives the arduino additional serial ports
SoftwareSerial mySerial(11, 12); //Initialize SoftwareSerial, and tell it you will be connecting through pins 2 and 3
Adafruit_GPS GPS(&mySerial); //Create GPS object

String NMEA1;  //We will use this variable to hold our first NMEA sentence
String NMEA2;  //We will use this variable to hold our second NMEA sentence
char c;       //Used to read the characters spewing from the GPS module
int chipSelect = 53; //chipSelect pin for the SD card Reader
File mySensorData; //Data object you will write your sesnor data to
float deg; 
float degWhole; 
float degDec;
void setup()  
{
  Serial.begin(115200);  //Turn on the Serial Monitor
  GPS.begin(9600);       //Turn GPS on at baud rate of 9600
  GPS.sendCommand("$PGCMD,33,0*6D"); // Turn Off GPS Antenna Update
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA); //Tell GPS we want only $GPRMC and $GPGGA NMEA sentences
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  delay(1000);  //Pause
  pinMode(10, OUTPUT); //Must declare 10 an output and reserve it to keep SD card happy
  SD.begin(chipSelect); //Initialize the SD card reader
  
 
  if (SD.exists("GPSData.txt")) { //Delete old data files to start fresh
    SD.remove("GPSData.txt");
  }

}


void loop()                     // run over and over again
{
readGPS();  //This is a function we define below which reads two NMEA sentences from GPS

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
  //if((int)GPS.latitude!=200&&(int)GPS.latitude!=0&&(int)GPS.longitude!=200&&(int)GPS.longitude!=0)
  //{
GPS.parse(GPS.lastNMEA());
NMEA2=GPS.lastNMEA();
  Serial.flush();
  Serial.println(NMEA1);
  Serial.println(NMEA2);
  Serial.println("////////////////////////////////////////////////");
  Serial.println(GPS.latitude,15);
  Serial.println(GPS.longitude,15);
  Serial.println("========================////////////////////////////////////////////////");
 
  degWhole=float(int(GPS.longitude/100)); //gives me the whole degree part of Longitude
  degDec = (GPS.longitude - degWhole*100)/60; //give me fractional part of longitude
  deg = degWhole + degDec;
  Serial.print(deg,6);
  Serial.print(","); //write comma to SD card
  
  degWhole=float(int(GPS.latitude/100)); //gives me the whole degree part of latitude
  degDec = (GPS.latitude - degWhole*100)/60; //give me fractional part of latitude
  deg = degWhole + degDec; //Gives complete correct decimal form of latitude degrees
  if (GPS.lat=='S') {  //If you are in Southern hemisphere latitude should be negative
    deg= (-1)*deg;
  }
  Serial.print(deg,6);






  
  Serial.println(GPS.speed);
  mySensorData = SD.open("GPSData.txt", FILE_WRITE);
  mySensorData.print(GPS.latitude,20); //Write measured latitude to file
  mySensorData.print(GPS.lat); //Which hemisphere N or S
 Serial.println("**************************************************");
 
  mySensorData.print(",");
  mySensorData.print(GPS.longitude,20); //Write measured longitude to file
  mySensorData.print(GPS.lon); //Which Hemisphere E or W
 // mySensorData.print(",");
   //mySensorData.print(GPS.speed);
   //mySensorData.print(",");
 mySensorData.println("");
  mySensorData.close();
  //}
}
void clearGPS() {  //Since between GPS reads, we still have data streaming in, we need to clear the old data by reading a few sentences, and discarding these
while(!GPS.newNMEAreceived()) {
  c=GPS.read();
  }
GPS.parse(GPS.lastNMEA());
while(!GPS.newNMEAreceived()) {
  c=GPS.read();
  }
GPS.parse(GPS.lastNMEA());

}
