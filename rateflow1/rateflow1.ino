int NbTopsFan; 
#include <SD.h>
int Calc;
int sum;
int chipSelect = 53; //chipSelect pin for the SD card Reader
File mySensorData; //Data object you will write your sesnor data to
                                  //The pin location of the sensor
int hallsensor = 8;                        
typedef struct{                  //Defines the structure for multiple fans and their dividers
  char fantype;
  unsigned int fandiv;
}fanspec;

//Definitions of the fans
fanspec fanspace[3]={{0,1},{1,2},{2,8}};

char fan = 1;   //This is the varible used to select the fan and it's divider, set 1 for unipole hall effect sensor 
               //and 2 for bipole hall effect sensor 
void rpm ()      //This is the function that the interupt calls 
{ 
 NbTopsFan++; 
} 
              //This is the setup function where the serial port is initialised,
             //and the interrupt is attached
void setup() 
{ 
 pinMode(hallsensor, 2); 
 Serial.begin(9600); 
 attachInterrupt(0, rpm, RISING);
 pinMode(10, OUTPUT); //Must declare 10 an output and reserve it to keep SD card happy
  SD.begin(chipSelect); //Initialize the SD card reader
  
  if (SD.exists("rate.txt")) { //Delete old data files to start fresh
    SD.remove("rate.txt");
  } 
} 
void loop () 
{
   NbTopsFan = 0;  //Set NbTops to 0 ready for calculations
   sei();   //Enables interrupts
   delay (1000);  //Wait 1 second
   cli();   //Disable interrupts
  // Serial.print (" hellp\n");
   Calc = ((NbTopsFan * 60)/fanspace[fan].fandiv); //Times NbTopsFan (which is apprioxiamately the fequency the fan is spinning at) by 60 seconds before dividing by the fan's divider
   if (Calc!=0)   //  then the water is flowing in the sensor 
   {
    sum=sum+ Calc; // counting the number of the revolutions that we have got    //each revolution equal to (1/24 ml )                           
   }
   else if (sum!=0)// then the water is stopped and not flowing any more 
   {
    
      Serial.print (" amount:");
        Serial.print (sum/24.0); 
         Serial.println (" ml");
         mySensorData = SD.open("rate.txt", FILE_WRITE);
         mySensorData.println(sum/24.0);
          mySensorData.close();
        sum=0;
    }
}

