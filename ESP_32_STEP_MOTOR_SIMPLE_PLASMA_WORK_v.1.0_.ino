TaskHandle_t Task1;


#include <nvs_flash.h>


#define BLYNK_TEMPLATE_ID           "TMPLW84l0iFe"
#define BLYNK_DEVICE_NAME           "Quickstart Device"
#define BLYNK_AUTH_TOKEN            "Hbekmu191ZKTyJAbVE-2VOv1NiNcW2vf"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;

//char ssid[] = "Keenetic-2989";
//char pass[] = "aobUNuZS";

char ssid[] = "Fibertel WiFi514 2.4GHz";
char pass[] = "01441789845";


BlynkTimer timer;
//----------------------------------------------------------------------------------


#include <Preferences.h>

Preferences preferences;

byte message_read_value[8] = {0X5A, 0XA5, 0X05, 0X82, 0X50, 0X00, 0X00, 0X65};

byte slide_page[10] = {0X5A, 0XA5, 0X07, 0X82, 0X00, 0X84, 0X5A, 0X01, 0X00, 0X04};

byte directionR[8] = {0X5A, 0XA5, 0X05, 0X82, 0X50, 0XD0, 0X00, 0X01};
byte directionL[8] = {0X5A, 0XA5, 0X05, 0X82, 0X50, 0XD0, 0X00, 0X00};



byte inquireSTOP[7] = {0X5A, 0XA5, 0X04, 0X83, 0X50, 0XF0, 0X01};




byte slide_page_1[10] = {0X5A, 0XA5, 0X07, 0X82, 0X00, 0X84, 0X5A, 0X01, 0X00, 0X00};
byte slide_page_2[10] = {0X5A, 0XA5, 0X07, 0X82, 0X00, 0X84, 0X5A, 0X01, 0X00, 0X02};


byte Buffer[20];
byte BufferWOW[20];
byte Buffer_Len = 0;
byte Buffer_Len_WOW = 0;
bool flag = false;
bool flagTask1 = false;
bool flagStop = false;

int stepsPerMM = 2000;
const int ms1Pin = 26;
const int ms2Pin = 27;
const int ms3Pin = 14;
const int ledPin = 13;
int firstTime = 0;
int roundedWaitTime;
unsigned long timer1;
unsigned long timer2;
unsigned long difference;
int stp_distance = 0;
int current_distance = 0;
int c = 0;
int lengthOfString = 0;

float velocity_parking_inside = 10;
float velocity_parking_back_inside = 5;
int direction_inside = 0;


//----------------------------------------new-algorithm-data--------------------------------------------------------------------------
float accelerationTerm = 2000;       // acceleration factor 
float speedOf = 0.00015;             // delay beteen motor impulses on final speed, seconds (= 1 / speedFromSetup * stepsPer_mm) 
float speedOfZero = 0.00015;         // delay beteen motor impulses on final speed, seconds (= 1 / velocity_parking_inside * stepsPer_mm) 
float stepsPer_mm = 100;             // amount of steps per one millimeter (taken from setup and stored in flesh memory)
float waitTime = 0.003;              // first and last delay between motor impulses, seconds
float speedFromSetup = 10;           // speed, mm/s (taken from setup and stored in flesh memory)

String directionString = "left";     // direction (put here "right" if it isn't left)

const int stepPin = 13; 
const int dirPin = 12; 
const int zeroPositionSensor = 5;
//------------------------------------------------------------------------------------------------------------------------------------


float currentPositionInfo = 0;
int flagOfTheMovement = 0;
int countOfExtraction = 0;
int currentPositionInfo_dec = 0;


//void myTimerEvent()
//{
//  if (flagOfTheMovement == 1) {string_write(currentPositionInfo, 0XE0);}
//
//  string_write(currentPositionInfo, 0XE0);
//}


//------------------------CORE-0-THREAD----------------------------------------------------
 /*
void Task1code( void * pvParameters ){
for(;;){


  
   if (flagOfTheMovement == 1) {
//    Serial2.write(inquireSTOP, 7);
//    delay(10);
//      Serial.println("inside");
       if (Serial2.available())
  {
    BufferWOW[Buffer_Len_WOW] = Serial2.read();
    Buffer_Len_WOW++;
    flagTask1 = true;
  } else {
        if (flagTask1){Serial.write(BufferWOW, Buffer_Len_WOW);


      if (BufferWOW[0] == 0X5A) { 
         if (BufferWOW[4] == 0X50) {
          if (BufferWOW[5] == 0XF0) {
            
 //             Serial.println("inside STOP");
  //            flagStop = true;
              }
            }
          }        
        }
        Buffer_Len_WOW = 0;
        flagTask1 = false;
  }

      } //--if-movement

     
   delay(10);
  } 
}
*/
//-----------------------------------------------------------------------------------------








void ReadSerialTry()
{
if (flagOfTheMovement == 1) {
  while (Serial2.available() > 0){BufferWOW[Buffer_Len_WOW] = Serial2.read(); Buffer_Len_WOW++; flagTask1 = true;}
     if (flagTask1 == true){ 


      if (BufferWOW[0] == 0X5A) {
         if (BufferWOW[4] == 0X50) {
          if (BufferWOW[5] == 0XF0) {Serial.write(BufferWOW, Buffer_Len_WOW);

              flagStop = true;
              }
            }
          }        
        }
        Buffer_Len_WOW = 0;
        flagTask1 = false;
    }
 }
       
 













//----------------ACCELERATION--------------------------------------------

//function to return the next delay value for positive acceleration
float positiveAcceleration(float waitTime) {
    float dVelocity = waitTime * accelerationTerm;
    waitTime = 1/(dVelocity+1/waitTime);
    if (waitTime < speedOf){
      waitTime = speedOf;
    }
    return waitTime;
}

//function to return the next delay value for negative acceleration
float negativeAcceleration(float waitTime) {
    float dVelocity = waitTime * -1 * accelerationTerm;
    waitTime = 1/(dVelocity+1/waitTime);
    if (waitTime > 0.003){
      waitTime = 0.003;
    }
    return waitTime;
}

//-----------------------------------------------------------------------------------------------------------



// -----------------MOVING-T0-THE-POINT----------------------------------------------------------------------

void motingToThePoint(int stepsToMove) {



if (directionString == "left") {
if (stepsToMove >= current_distance) {digitalWrite(dirPin, LOW);} else {digitalWrite(dirPin, HIGH);}

float accelerationGainStepsAnount = 0;

while (waitTime > speedOf) {
    float dVelocity = waitTime * accelerationTerm;
    waitTime = 1/(dVelocity+1/waitTime);
    accelerationGainStepsAnount++;
    }

waitTime = 0.003;

if (abs(stepsToMove - current_distance)/2 > accelerationGainStepsAnount) {
  flagOfTheMovement = 1;
    countOfExtraction = 0; 
    currentPositionInfo = 0; 
    int roundedWaitTimeTemp = 0;
    for(int x = 0; x < (abs(stepsToMove - current_distance)-accelerationGainStepsAnount); x++) {
    waitTime = positiveAcceleration(waitTime);
    roundedWaitTime = round(waitTime*1000000);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(roundedWaitTime);
    digitalWrite(stepPin, LOW);
    currentPositionInfo++;
    countOfExtraction++;
    if (roundedWaitTime == roundedWaitTime & flagStop == true) {break;}
    roundedWaitTimeTemp = roundedWaitTime;
    if (countOfExtraction > 20) {ReadSerialTry(); countOfExtraction = 0; if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo)/stepsPer_mm, 0XE0);} else {string_write((current_distance - currentPositionInfo)/stepsPer_mm, 0XE0);}}
  } 
    flagStop = false;
    countOfExtraction = 0;
    currentPositionInfo_dec = 0;
    for(int x = 0; x < accelerationGainStepsAnount; x++) {
    waitTime = negativeAcceleration(waitTime);
    roundedWaitTime = round(waitTime*1000000);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(roundedWaitTime);
    digitalWrite(stepPin, LOW);
    currentPositionInfo_dec++;
    countOfExtraction++;
    if (countOfExtraction > 20) {countOfExtraction = 0; if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo + currentPositionInfo_dec)/stepsPer_mm, 0XE0);} else {string_write((current_distance - (currentPositionInfo + currentPositionInfo_dec))/stepsPer_mm, 0XE0);}}
}
delay(50); if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo + currentPositionInfo_dec)/stepsPer_mm, 0XE0); stp_distance = current_distance + currentPositionInfo + currentPositionInfo_dec;} else {string_write((current_distance - (currentPositionInfo + currentPositionInfo_dec))/stepsPer_mm, 0XE0); stp_distance = current_distance - (currentPositionInfo + currentPositionInfo_dec);}
 flagOfTheMovement = 0;  
} else {
    int a = abs(stepsToMove - current_distance);
    int b = abs(stepsToMove - current_distance)/2;
    
    if (b*2 != a) {c = b+1;} else {c = b;};
countOfExtraction = 0; 
currentPositionInfo = 0;   
    for(int x = 0; x < (abs(stepsToMove - current_distance)/2); x++) {
    waitTime = positiveAcceleration(waitTime);
    roundedWaitTime = round(waitTime*1000000);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(roundedWaitTime);
    digitalWrite(stepPin, LOW);
    currentPositionInfo++;
    countOfExtraction++;
    if (countOfExtraction > 20) {countOfExtraction = 0; if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo)/stepsPer_mm, 0XE0);} else {string_write((current_distance - currentPositionInfo)/stepsPer_mm, 0XE0);}}
  } 
countOfExtraction = 0;
currentPositionInfo_dec = 0;
  for(int x = 0; x < c; x++) {
    waitTime = negativeAcceleration(waitTime);
    roundedWaitTime = round(waitTime*1000000);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(roundedWaitTime);
    digitalWrite(stepPin, LOW);
    currentPositionInfo_dec++;
    countOfExtraction++;
    if (countOfExtraction > 20) {countOfExtraction = 0; if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo + currentPositionInfo_dec)/stepsPer_mm, 0XE0);} else {string_write((current_distance - (currentPositionInfo + currentPositionInfo_dec))/stepsPer_mm, 0XE0);}}
   }
   delay(50); if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo + currentPositionInfo_dec)/stepsPer_mm, 0XE0);} else {string_write((current_distance - (currentPositionInfo + currentPositionInfo_dec))/stepsPer_mm, 0XE0);}
  }
 } else {


//---------------------OTHER-WAY----------------------

  
  if (stepsToMove >= current_distance) {digitalWrite(dirPin, HIGH);} else {digitalWrite(dirPin, LOW);}

float accelerationGainStepsAnount = 0;

while (waitTime > speedOf) {
    float dVelocity = waitTime * accelerationTerm;
    waitTime = 1/(dVelocity+1/waitTime);
    accelerationGainStepsAnount++;
    }

waitTime = 0.003;

if (abs(stepsToMove - current_distance)/2 > accelerationGainStepsAnount) {
    flagOfTheMovement = 1;
    countOfExtraction = 0; 
    currentPositionInfo = 0; 
    int roundedWaitTimeTemp = 0;
    for(int x = 0; x < (abs(stepsToMove - current_distance)-accelerationGainStepsAnount); x++) {
    waitTime = positiveAcceleration(waitTime);
    roundedWaitTime = round(waitTime*1000000);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(roundedWaitTime);
    digitalWrite(stepPin, LOW);
    currentPositionInfo++;
    countOfExtraction++;
    if (roundedWaitTime == roundedWaitTime & flagStop == true) {break;}
    roundedWaitTimeTemp = roundedWaitTime;
    if (countOfExtraction > 20) {ReadSerialTry(); countOfExtraction = 0; if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo)/stepsPer_mm, 0XE0);} else {string_write((current_distance - currentPositionInfo)/stepsPer_mm, 0XE0);}}

  } 
    flagStop = false;
    countOfExtraction = 0;
    currentPositionInfo_dec = 0;
  for(int x = 0; x < accelerationGainStepsAnount; x++) {
    waitTime = negativeAcceleration(waitTime);
    roundedWaitTime = round(waitTime*1000000);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(roundedWaitTime);
    digitalWrite(stepPin, LOW);
    currentPositionInfo_dec++;
    countOfExtraction++;
    if (countOfExtraction > 20) {countOfExtraction = 0; if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo + currentPositionInfo_dec)/stepsPer_mm, 0XE0);} else {string_write((current_distance - (currentPositionInfo + currentPositionInfo_dec))/stepsPer_mm, 0XE0);}}
}
delay(50); if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo + currentPositionInfo_dec)/stepsPer_mm, 0XE0); stp_distance = current_distance + currentPositionInfo + currentPositionInfo_dec;} else {string_write((current_distance - (currentPositionInfo + currentPositionInfo_dec))/stepsPer_mm, 0XE0); stp_distance = current_distance - (currentPositionInfo + currentPositionInfo_dec);}
 flagOfTheMovement = 0; 
 
} else {
    int a = abs(stepsToMove - current_distance);
    int b = abs(stepsToMove - current_distance)/2;
    
    if (b*2 != a) {c = b+1;} else {c = b;};
countOfExtraction = 0; 
currentPositionInfo = 0;    
    for(int x = 0; x < (abs(stepsToMove - current_distance)/2); x++) {
    waitTime = positiveAcceleration(waitTime);
    roundedWaitTime = round(waitTime*1000000);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(roundedWaitTime);
    digitalWrite(stepPin, LOW);
    currentPositionInfo++;
    countOfExtraction++;
    if (countOfExtraction > 20) {countOfExtraction = 0; if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo)/stepsPer_mm, 0XE0);} else {string_write((current_distance - currentPositionInfo)/stepsPer_mm, 0XE0);}}

  } 
countOfExtraction = 0;
currentPositionInfo_dec = 0;
  for(int x = 0; x < c; x++) {
    waitTime = negativeAcceleration(waitTime);
    roundedWaitTime = round(waitTime*1000000);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(roundedWaitTime);
    digitalWrite(stepPin, LOW);
    currentPositionInfo_dec++;
    countOfExtraction++;
    if (countOfExtraction > 20) {countOfExtraction = 0; if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo + currentPositionInfo_dec)/stepsPer_mm, 0XE0);} else {string_write((current_distance - (currentPositionInfo + currentPositionInfo_dec))/stepsPer_mm, 0XE0);}}
   }
   delay(50); if ((stepsToMove - current_distance) >= 0) {string_write((current_distance + currentPositionInfo + currentPositionInfo_dec)/stepsPer_mm, 0XE0);} else {string_write((current_distance - (currentPositionInfo + currentPositionInfo_dec))/stepsPer_mm, 0XE0);}
  }
  
 }

}

//--------------------------------------------------------------------------------------------------------------





//----------------------FIND-ZERO-----------------------------------------------------------------------------------------------------
void findZero() {

  string_write(0, 0XE0);

  current_distance = 0;

  if (direction_inside == 1) {directionString = "left";} else {directionString = "right";}

if (directionString == "left") {digitalWrite(dirPin, HIGH);} else {digitalWrite(dirPin, LOW);}

float accelerationGainStepsAnount = 0;

speedOfZero = speedOf;
speedOf = (1/(velocity_parking_inside*stepsPer_mm));


while (waitTime > speedOf) {
    float dVelocity = waitTime * accelerationTerm;
    waitTime = 1/(dVelocity+1/waitTime);
    accelerationGainStepsAnount++;
    }

waitTime = 0.003;

int xTemp = 0;

  while (digitalRead(zeroPositionSensor) == LOW) {
    waitTime = positiveAcceleration(waitTime);
    roundedWaitTime = round(waitTime*1000000);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(roundedWaitTime);
    digitalWrite(stepPin, LOW);
    xTemp++;
  } 

  if (xTemp >= accelerationGainStepsAnount) {xTemp = accelerationGainStepsAnount;} 
  for(int x = 0; x < xTemp; x++) {
    waitTime = negativeAcceleration(waitTime);
    roundedWaitTime = round(waitTime*1000000);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(roundedWaitTime);
    digitalWrite(stepPin, LOW);
  }

waitTime = (1/(velocity_parking_back_inside*stepsPer_mm));

if (directionString == "left") {digitalWrite(dirPin, LOW);} else {digitalWrite(dirPin, HIGH);}

  while (digitalRead(zeroPositionSensor) != LOW) {
    roundedWaitTime = round(waitTime*1000000);
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(roundedWaitTime);
    digitalWrite(stepPin, LOW);
  } 
  
speedOf = speedOfZero;
waitTime = 0.003;
current_distance = 0;


}
//---------------------------------------------------------------------------------------------------------------------------



//---------------SCREEN-READ-WRITE-------------------------------------------------------------------------------------------

void ReadSerial()
{
  if (Serial2.available())
  {
    Buffer[Buffer_Len] = Serial2.read();
    Buffer_Len++;
    flag = true;
  }
  else
  {
    if (flag)
    {
      if (Buffer[0] == 0X5A) {
 //      Blynk.virtualWrite(V10, "Received 0X54");
         if (Buffer[4] == 0X50) {
 //       Blynk.virtualWrite(V10, "Received 0X50");
            if (Buffer[5] == 0X60) {
 //          Blynk.virtualWrite(V10, "Received distance");
            lengthOfString = 0;
            for (int x = 7; Buffer[x] != 0XFF; x++) {lengthOfString++;}
            char buf[lengthOfString];
            for (int x = 7; x-7 != lengthOfString; x++) {buf[x-7] = Buffer[x];}
            String s = String(buf);
            float distance = s.toFloat();
//            Serial.println(distance);
            distance = distance * stepsPer_mm;
            stp_distance = round(distance);

            motingToThePoint(stp_distance); //---------------------------movement------>>-----------------
            
 //           Blynk.virtualWrite(V10, "Go to motor movement");
            current_distance = stp_distance; 
            
            preferences.begin("Credentials", false);
            preferences.putInt("current", current_distance);
            preferences.end();

  //          Serial.println(current_distance);

            stp_distance = 0;
            preferences.begin("Credentials", false);
            stp_distance = preferences.getInt("current", 0);
            preferences.end();
   //         Serial.println(stp_distance);


            
            }
          if (Buffer[5] == 0X20) {
            lengthOfString = 0;
            for (int x = 7; Buffer[x] != 0XFF; x++) {lengthOfString++;}
            char buf[lengthOfString];
            for (int x = 7; x-7 != lengthOfString; x++) {buf[x-7] = Buffer[x];}
            String s = String(buf);
            float distance = s.toFloat();
   //         Serial.println(distance);
            stepsPer_mm = distance;

            preferences.begin("Credentials", false);
            preferences.putFloat("stepsPermm", stepsPer_mm);
            preferences.end();
//            Blynk.virtualWrite(V10, "Stored Steps");
//            ESP.restart();
            
            }
          if (Buffer[5] == 0X30) {
            lengthOfString = 0;
            for (int x = 7; Buffer[x] != 0XFF; x++) {lengthOfString++;}
            char buf[lengthOfString];
            for (int x = 7; x-7 != lengthOfString; x++) {buf[x-7] = Buffer[x];}
            String s = String(buf);
            float distance = s.toFloat();
            speedFromSetup = distance;
            speedOf = (1/(speedFromSetup*stepsPer_mm));   // delay beteen motor impulses on final speed, seconds (= 1 / speedFromSetup * stepsPer_mm)

            preferences.begin("Credentials", false);
            preferences.putFloat("velocity", speedFromSetup);
            preferences.end();
 //           Blynk.virtualWrite(V10, "Stored Velocity");
            
            }
          if (Buffer[5] == 0XA0) {
            lengthOfString = 0;
            for (int x = 7; Buffer[x] != 0XFF; x++) {lengthOfString++;}
            char buf[lengthOfString];
            for (int x = 7; x-7 != lengthOfString; x++) {buf[x-7] = Buffer[x];}
            String s = String(buf);
            float distance = s.toFloat();
            velocity_parking_inside = distance;
//            speedOf = (1/(speedFromSetup*stepsPer_mm));   // delay beteen motor impulses on final speed, seconds (= 1 / speedFromSetup * stepsPer_mm)

            preferences.begin("Credentials", false);
            preferences.putFloat("velocityParking", velocity_parking_inside);
            preferences.end();

            
            }
          if (Buffer[5] == 0XB0) {
            lengthOfString = 0;
            for (int x = 7; Buffer[x] != 0XFF; x++) {lengthOfString++;}
            char buf[lengthOfString];
            for (int x = 7; x-7 != lengthOfString; x++) {buf[x-7] = Buffer[x];}
            String s = String(buf);
            float distance = s.toFloat();
            velocity_parking_back_inside = distance;
//            speedOf = (1/(speedFromSetup*stepsPer_mm));   // delay beteen motor impulses on final speed, seconds (= 1 / speedFromSetup * stepsPer_mm)

            preferences.begin("Credentials", false);
            preferences.putFloat("velocityPBack", velocity_parking_back_inside);
            preferences.end();
            
            
            }  
          if (Buffer[5] == 0XC0) {
                                  
            
            preferences.begin("Credentials", false);

            if (direction_inside == 1) {direction_inside = 0; preferences.putInt("directionOne", direction_inside); Serial2.write(directionR, 8); Serial.println(direction_inside);} else {direction_inside = 1; preferences.putInt("directionOne", direction_inside); Serial2.write(directionL, 8);}
            
            preferences.end();
            
            
            }                                  
             if (Buffer[5] == 0X80) {
             findZero(); Serial2.write(slide_page_1, 10);
            }

             if (Buffer[5] == 0X48) {  //------------------------------------CHECK-PASSWORD----------------
             lengthOfString = 0;
             for (int x = 7; Buffer[x] != 0XFF; x++) {lengthOfString++;}
             char buf[lengthOfString];
             for (int x = 7; x-7 != lengthOfString; x++) {buf[x-7] = Buffer[x];}
             String s = String(buf);
             float distance = s.toFloat();
             if (distance == 7745) {Serial2.write(slide_page_2, 10);}
             
            }
            
             if (Buffer[5] == 0X90) {
             if (directionString == "left") {digitalWrite(dirPin, LOW);} else {digitalWrite(dirPin, HIGH);}
             Serial2.write(slide_page_1, 10);
            }
            
          if (Buffer[5] == 0X40) {
            lengthOfString = 0;
            for (int x = 7; Buffer[x] != 0XFF; x++) {lengthOfString++;}
            char buf[lengthOfString];
            for (int x = 7; x-7 != lengthOfString; x++) {buf[x-7] = Buffer[x];}
            String s = String(buf);
            float distance = s.toFloat();
            accelerationTerm = distance*1000;

            preferences.begin("Credentials", false);
            preferences.putFloat("acceleration", accelerationTerm);
            preferences.end();
 //           Blynk.virtualWrite(V10, "Stored Acceleration");

            }  
          }
        }
      Buffer_Len = 0;
      flag = false;
    }
  }
}


//--------------------------Sending data to the screen-------------------------

void string_write(float tempValue, byte dates){
            char b_bite[20];
            String tempValueString = String(tempValue, 2);
            tempValueString = tempValueString + "F";
            tempValueString.toCharArray(b_bite, 20);
            lengthOfString = 0;
            for (int x = 0; b_bite[x] != 0X46; x++) {lengthOfString++;}
            char buf[lengthOfString+8];
            buf[0] = {0X5A};
            buf[1] = {0XA5};
            buf[2] = lengthOfString+5;
            buf[3] = {0X82};
            buf[4] = {0X50};
            buf[5] = dates;
            for (int x = 0; x < lengthOfString; x++) {buf[x+6] = b_bite[x];}
            buf[6+lengthOfString] = {0XFF};
            buf[7+lengthOfString] = {0XFF};
            Serial2.write(buf, lengthOfString+8);
}
//-----------------------------------------------------------------------------


//-------------------------BLYNK-----------------------------------------------

//BLYNK_CONNECTED()
//{
//Blynk.virtualWrite(V10, "Connected");
//}

//-----------------------------------------------------------------------------










void setup() {
 /*
  preferences.begin("Credentials", false);
  preferences.putFloat("velocity", speedFromSetup); 
  preferences.putFloat("acceleration", accelerationTerm);
  preferences.putFloat("stepsPermm", stepsPer_mm);
  preferences.putInt("current", current_distance);
  preferences.putFloat("velocityParking", velocity_parking_inside);
  preferences.putFloat("velocityPBack", velocity_parking_back_inside);
  preferences.putInt("directionOne", direction_inside);
  preferences.end();
   */




  preferences.begin("Credentials", false);
  speedFromSetup = preferences.getFloat("velocity", 0); 
  accelerationTerm = preferences.getFloat("acceleration", 0);
  stepsPer_mm = preferences.getFloat("stepsPermm", 0);
  current_distance = preferences.getInt("current", 0);
  velocity_parking_inside = preferences.getFloat("velocityParking", 0);
  velocity_parking_back_inside = preferences.getFloat("velocityPBack", 0);
  direction_inside = preferences.getInt("directionOne", 0);
  preferences.end();



//xTaskCreatePinnedToCore(
//                    Task1code,   /* Task function. */
//                    "Task1",     /* name of task. */
//                    4096,       /* Stack size of task */
//                    NULL,        /* parameter of the task */
//                    0,           /* priority of the task */
//                    &Task1,      /* Task handle to keep track of created task */
//                    1);          /* pin task to core 0 */                  
//                    delay(500);



//setup the pins
  pinMode(stepPin, OUTPUT); 
  pinMode(dirPin, OUTPUT);
  pinMode(ms1Pin, OUTPUT);
  pinMode(ms2Pin, OUTPUT);
  pinMode(ms3Pin, OUTPUT);
  pinMode(ledPin, OUTPUT);  
  pinMode(zeroPositionSensor, INPUT_PULLUP);

  //change the microstepping
  digitalWrite(ms1Pin,HIGH);
  digitalWrite(ms2Pin,HIGH);
  digitalWrite(ms3Pin,HIGH);

  Serial.begin(115200);
  Serial2.begin(115200);
  Serial2.setTimeout(1);

//  Blynk.begin(auth, ssid, pass);
//timer.setInterval(500L, myTimerEvent);

}



















void loop() {

  ReadSerial();
//    Blynk.run();
//    timer.run();
  if (firstTime == 0) {
      speedOf = (1/(speedFromSetup*stepsPer_mm)); firstTime = 1;
      delay(1000); string_write(speedFromSetup, 0X30); 
      delay(500); float accelerationTermTemp = accelerationTerm/1000; string_write(accelerationTermTemp, 0X40);
      delay(500); string_write(stepsPer_mm, 0X20); string_write(velocity_parking_inside, 0XA0); string_write(velocity_parking_back_inside, 0XB0); 
      delay(500); 
      Serial2.write(slide_page, 10); 
      if (direction_inside == 1) {directionString = "left";} else {directionString = "right";}
      if (direction_inside == 1) {Serial2.write(directionL, 8);} else {Serial2.write(directionR, 8);}
      }
}
