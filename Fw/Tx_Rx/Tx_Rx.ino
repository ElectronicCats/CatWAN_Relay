#include <SPI.h>
#include <LoRa.h>
//#define LORA
#define SERIAL

String dato="";
int serialData[3];
bool flag_g=0;

int counter = 0;
bool stat0=1;
bool stat1=1;
bool stat2=1;

void interrupt(){
  flag_g=1;
  }

void transmit(){
    int bitOne = !digitalRead(0);
    int bitTwo = !digitalRead(1);
    int bitThree = !digitalRead(2);
    #ifdef LORA
    LoRa.beginPacket();
    LoRa.write(bitOne);
    LoRa.write(bitTwo);
    LoRa.write(bitThree);
    LoRa.endPacket();
    #endif
    Serial.println(String(bitOne)+String(bitTwo)+String(bitThree));
  }
  
void setup() {
  Serial.begin(115200);
  
  #ifdef SERIAL
  while(!Serial);
  #endif
  
  #ifdef LORA
  LoRa.setPins(SS, RFM_RST, RFM_DIO0);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  #endif
  
  pinMode(0,INPUT); 
  pinMode(1,INPUT); 
  pinMode(2,INPUT); 
  pinMode(Relay_1,OUTPUT);
  pinMode(Relay_2,OUTPUT);
  pinMode(Relay_3,OUTPUT);
  pinMode(14,OUTPUT);

  attachInterrupt(0, interrupt, CHANGE);
  attachInterrupt(1,interrupt, CHANGE);
  attachInterrupt(2,interrupt, CHANGE);
  
  digitalWrite(14,LOW);
}

void loop() {
   if(flag_g){
    detachInterrupt(0);
    detachInterrupt(1);
    detachInterrupt(2);
    delay(20);
    transmit();
    attachInterrupt(0, interrupt, CHANGE);
    attachInterrupt(1,interrupt, CHANGE);
    attachInterrupt(2,interrupt, CHANGE);
    flag_g=0;
    } 

  #ifdef LORA
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    digitalWrite(14,HIGH);
    int dato[3];
    int i=0;
    if(LoRa.available()>4)return;
    while (LoRa.available()) {
      dato[i]=LoRa.read();
      Serial.println(dato[i]);
      i++;
    }
    digitalWrite(Relay_1,!dato[0]);
    digitalWrite(Relay_2,!dato[1]);
    digitalWrite(Relay_3,!dato[2]);
    digitalWrite(14,LOW);
  }
  #endif

  #ifdef SERIAL
  if(Serial.available()>0){
    int i=0;
    while(Serial.available()){
      serialData[i]=Serial.read()-48;
      i++;
    }    
    digitalWrite(Relay_1,serialData[0]);
    digitalWrite(Relay_2,serialData[1]);
    digitalWrite(Relay_3,serialData[2]);
    Serial.println("ok");
    }
  #endif 
 
}
