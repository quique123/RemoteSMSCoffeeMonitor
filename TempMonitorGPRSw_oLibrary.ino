#include <Time.h>
#include <TimeLib.h>
#include <SoftwareSerial.h>
#include <TimeAlarms.h>
#include <dht.h>
//#include "SIM900z.h"

dht DHT;
#define DHT11_PIN 5
char number[]="+mynumber";
boolean started=false;
SoftwareSerial sim900(9,10);

int smsSampleCount=0; //how many samples i want in the array
typedef struct {
  int timeStamp;
  double tempSample;
  double humSample;
} samplePoint;

samplePoint finalDataArray[10];


void setup(){
  Serial.begin(9600);
  sim900.begin(9600); //get modem ready
  delay(1000); //wait for modem before 
  Alarm.timerRepeat(3600, MainAlarm); // 1hrx60=60*60=3600
  Serial.println("Alarm set!");
}

void loop(){
  Alarm.delay(10); // wait one second between clock display
}

void MainAlarm(){
  Serial.println("Main Alarm fired!");
  int chk = DHT.read11(DHT11_PIN);
  Serial.print("Temperature = ");
  double temp = DHT.temperature;
  Serial.println(DHT.temperature);
  Serial.print("Humidity = ");
  double hum = DHT.humidity;
  Serial.println(DHT.humidity);
  
  //Check every 6 hours, if smsSampleCount reached 8, store it...
  if (smsSampleCount<8) {
    finalDataArray[smsSampleCount].timeStamp = smsSampleCount;
    finalDataArray[smsSampleCount].tempSample = temp;
    finalDataArray[smsSampleCount].humSample = hum;
    smsSampleCount++;
    Serial.print("Data added with count =");
    Serial.println(smsSampleCount);
   } else { // else reset counter and send data.
    Serial.println("Reached 7, will send data!");
    smsSampleCount=0;
    //check to see if modem is ready
    sendData();
   }
}

void printDataCollectedSoFar() {
  Serial.println("Printing data");
  int x = 0;
  while (x<7) {
    Serial.println(finalDataArray[x].timeStamp);
    Serial.println(finalDataArray[x].tempSample);
    Serial.println(finalDataArray[x].humSample);  
    x=x+1;
  }
}

void sendData(){
  //Setup local vars for temp/hum strings
  static char outTempStr[15];
  static char outHumStr[15];
  static String finalString;

  int numElements = sizeof(finalDataArray) / sizeof(finalDataArray[0]);
  
  for (int i=0; i<numElements;i++){
    String tempString = dtostrf(finalDataArray[i].tempSample,5,2,outTempStr);
    String humString = dtostrf(finalDataArray[i].humSample,5,2,outHumStr);
    finalString = finalString+":"+tempString+"/"+humString+"-";
  }
  Serial.print("Parsed finalString =");
  Serial.println(finalString);
  Serial.println("Resetting array!");
  samplePoint finalDataArray[10];

  //DEBUG ONLY
  // Check if modem ok
  delay(1000);
  sim900.println("AT");
  delay(1000);
  printRead();
  
  sim900.print("\r");
  delay(1000);
  
  sim900.print("AT+CSCLK=0\r"); //NO SLEEP
  delay(1000);
  printRead();
  
  sim900.print("AT+CMGF=1\r"); //text mode
  delay(1000);
  
  sim900.print("AT+CMGS=\"+mynumber\"\r");
  delay(1000);
  
  sim900.print(finalString + "\r"); //The text for the message
  Serial.println("sent!");
  finalString = "";
  delay(1000);
  sim900.write(0x1a); //Equivalent to sending Ctrl+Z
  sim900.println(char(26));
  Serial.println("done!!");
}

void printRead() {
  char datain;
  while(sim900.available()>0){
    datain=sim900.read();
    if(datain>0){
      Serial.print(datain);
    }
  }
}
