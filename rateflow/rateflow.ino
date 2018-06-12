
//Warning: filesize(): stat failed for https://www.gravitechthai.com/upload/product/file/file-141573439.ino in /home/admin/domains/gravitechthai.com/public_html/product_download.php on line 30
 /**************************************************************************************
 *****************      GravitechThai        ******************************************
 **************************************************************************************
 * 
 * This is example sketch for water flow rate sensor for both YF-S201 and YF-S401 modelTime
 * 
 * URL: YF-S201: https://gravitechthai.com/product_detail.php?d=900
 *      YF-S401: https://gravitechthai.com/product_detail.php?d=1716
 * 
 * File: Flow_Meter_YF_S201_S401.ino
 * Author: Tawan Thintawornkul
 * Last date modified: June 14, 2017
 * 
 * All code in this file is free public licence under GravitechThai. Please refer to us if you have published this code.
 * GravitechThai invests time and resource delivering best codes for you.
 * You can purchase equipments to test with this code on our online store: https://gravitechthai.com/
 * Enjoy with your water flow project!
 *
 **************************************************************************************
 */

/* -----------------------------------------------------------------------------------------
 * -----------------------(1) Please select your sensor modelTime่-------------------------------
 * -----------------------------------------------------------------------------------------
 * To select your modelTime, delTimeete  " // " in front of the modelTime you would like to select.
 * For example: select YF_S401 then,
 *    //#define YF_S201
 *    #define YF_S401
 */
//#define YF_S201
#define YF_S401


/* -----------------------------------------------------------------------------------------
 * -------------------------(2) Sensor configuration ---------------------------------------
 * -----------------------------------------------------------------------------------------
 * the pin for Interrupt (in UNO R3 / Lambda board --> interruptpin 0 at digital pin 2) 
 * (More info: https://www.arduino.cc/en/Reference/AttachInterrupt) 
 */
#define sensorInterrupt 0 //config pin;  This example use Lambda board, if 0 --> connect at digital pin 2
#define sensorPin 2


/* -----------------------------------------------------------------------------------------
 * ----------------------------(3) calibration factor  -------------------------------------
 * -----------------------------------------------------------------------------------------
 */
//Calibration Factor of sensor (Pulse/sec / L/min)
#ifdef YF_S201
   const float coeff = 7.5; //(Hz)/(L/min)  //modelTime YF_S201
#endif 
#ifdef YF_S401
   const float coeff = 98; //(Hz)/(L/min)   //modelTime YF_S401
#endif

//stepTime
#define stepTime 1000 //(ms) 
                      //the equation inside this sketch is correct and work well in range of "1000 ~ 2000" ms per a measuring time!                                                                                                                                                                                                      
 


/* -----------------------------------------------------------------------------------------
 * ------------------------------- Global Variables ----------------------------------------
 * -----------------------------------------------------------------------------------------
 */
volatile uint16_t pulseCount; //volatile kind for matching with interruption

//parameter which are needed to display
float flowRate; // (L/min)
float flowRateMLS; // (mL/sec)
float flowCumVol; // (L or mL) Cumulative water volume which flow through flow meter.

//variable for millis() timing
unsigned long previousTime;
unsigned long currentTime;
unsigned long delTime;


/* -----------------------------------------------------------------------------------------
 * --------------------------- Arduino Main Functions --------------------------------------
 * -----------------------------------------------------------------------------------------
 */
void setup() {
  Serial.begin(115200); // config baudrate
  Serial.setTimeout(0); //set serial.readstring delTimeay time = 0 sec

  //setup input pin for flow meter
  pinMode(sensorPin, INPUT);
  //digitalWrite(sensorPin, HIGH);

  Serial.println("Flow meter start"); //tell user that machine is ready.
  delay(1000); //wait for user 1 second

  //pulseCount = 0; //if place pulse count here, the initial cumulative volume ~0.2 mL (which is not exactly 0) because of processor time delTimeay
  
  //set timing value
  previousTime = 0;
  
  //set initial value for variables
  flowRate = 0;
  flowRateMLS = 0;
  flowCumVol = 0;

  pulseCount = 0; //place pulseCount here make initial cum Vol = 0.00 because the pulseCount is reset quite before processor attach Interrupt pin already.
  attachInterrupt(sensorInterrupt, pulseCounter, FALLING); //make processor interrupt when the pulse change state from high to low (FALLING) and run pulseCounter()
}

void loop() {

  currentTime = millis(); //store millis() timing in currentTime
  delTime = currentTime-previousTime;

  serialReadCommand(); // check command from Serial Monitor
  
  if(delTime >= stepTime){ //measureFlow in each stepTime. For example: each 1 second)
    measureFlow();
  }
   
}


/* -----------------------------------------------------------------------------------------
 * ------------------------------------ Sub-Functions --------------------------------------
 * -----------------------------------------------------------------------------------------
 * 4 sub-function in this sketch consisted of:  
 *    measureFlow();  --------> calculate total pulseCounter to become fluid parameter and report result through serial monitor
 *    pulseCounter();  -------> count output pulse from flow meter
 *    reportResult();  -------> report parameters
 *    serialReadCommand()  ---> serial command recognizer: (Ex. type r to reset)
 */

//----------------------------measure flow function ---------------------------------------//
void measureFlow(){
    detachInterrupt(sensorInterrupt); //detatch interrupt to make processor calculate the fluid equations below
    /*
     * Flow Rate Calculation
     * Pulse count in Hz. 
     * Since currentTime - previousTime not exactly equal to stepTime.
     * Transforming pulse count in each step time become Hz unit can be done by interpolation:
     * 
     *              delTime (ms) ----count-----> pulseCount (pulse)
     *              1000 (ms) ------count------> (1000 * pulseCount / delTime)
     */
    flowRate = (1000 * pulseCount / delTime) / coeff; //see above
    flowRateMLS = flowRate*1000/60; //(mL/s)  ////this 1000 is unit conversion 1000 milli = 1 meter.
    #ifdef YF_S201
    flowCumVol += (flowRate/60)*(delTime/1000); // (L/sec)*(sec) = L  //this 1000 is unit conversion 1000 ms = 1 sec
    #endif
    #ifdef YF_S401
    flowCumVol += (flowRateMLS/60)*(delTime/1000); // (mL/sec)*(sec) = mL   //this 1000 is unit conversion 1000 ms = 1 sec
    #endif

    previousTime = currentTime;
    
    reportResult();

    pulseCount = 0;
    //make processor interrupt when the pulse change state from high to low (FALLING) and run pulseCounter()
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING); 
}


//----------------------function to report measured result ---------------------------------//
#ifdef YF_S201 //in case of YF_S201
void reportResult(){
  Serial.print("Flow Rate: "); Serial.print(flowRate, 2); Serial.print(" L/min"); //serial.print(variable, decimal)
  Serial.print("  Flow Rate: "); Serial.print(flowRateMLS, 1); Serial.print(" mL/s");
  Serial.print("  Cumulative Vol: "); Serial.print(flowCumVol, 2); Serial.print(" L");
  Serial.println("   ");
}
#endif

//----------------------function to report measured result ---------------------------------//
#ifdef YF_S401 //in case of YF_S401
void reportResult(){
  Serial.print("  Flow Rate: "); Serial.print(flowRateMLS, 2); Serial.print(" mL/s");
  Serial.print("  Cumulative Vol: "); Serial.print(flowCumVol, 1); Serial.print(" mL");
  Serial.println("   ");
}
#endif


//------------------------------function to count pulse ------------------------------------//
void pulseCounter(){
  pulseCount++;
}

//-----function to recognize user command through serials & make response to hardware-------//
void serialReadCommand(){
  String command;
  if (Serial.available()){
    command = Serial.readString();
    Serial.print("Get Command : ");
    Serial.println(command);
    switch (command[0]){
      case 'r': { //reset flow
                 Serial.println("--------RESET Cumulative Volume--------"); 
                 delay(1000); 
                 flowRate = 0; flowRateMLS = 0; flowCumVol = 0; pulseCount = 0; 
                 Serial.println("Flow meter start");
                } break;
      default : {
                 Serial.println("--------NO THIS COMMAND, PLEASE TYPE AGAIN--------");
                } break;
           }
      }
}
