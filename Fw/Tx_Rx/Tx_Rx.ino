#include <SPI.h>
#include <LoRa.h>

#define Serial SerialUSB
#define LORA


String dato="";
bool flag_g=0;

int counter = 0;
bool stat0=1;
bool stat1=1;
bool stat2=1;

void interrupt_0(){
  flag_g=1;
  }


void interrupt_1(){
  flag_g=1;
  }

void interrupt_2(){
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
    //Serial.println(String(bitOne)+String(bitTwo)+String(bitThree));
  }
  
void setup() {
  Serial.begin(115200);
  //while(!Serial);
  //Serial.println("LoRa Sender");
  LoRa.setPins(SS, RFM_RST, RFM_DIO0);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  pinMode(0,INPUT); 
  pinMode(1,INPUT); 
  pinMode(2,INPUT); 
  pinMode(Relay_1,OUTPUT);
  pinMode(Relay_2,OUTPUT);
  pinMode(Relay_3,OUTPUT);
  pinMode(14,OUTPUT);

  attachInterrupt(0, interrupt_0, CHANGE);
  attachInterrupt(1,interrupt_1, CHANGE);
  attachInterrupt(2,interrupt_2, CHANGE);
  
  digitalWrite(14,LOW);
}

void loop() {
   if(flag_g){
    detachInterrupt(0);
    detachInterrupt(1);
    detachInterrupt(2);
    delay(20);
    transmit();
    attachInterrupt(0, interrupt_0, CHANGE);
    attachInterrupt(1,interrupt_1, CHANGE);
    attachInterrupt(2,interrupt_2, CHANGE);
    flag_g=0;
    } 
    
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    digitalWrite(14,HIGH);
    int dato[3];
    int i=0;
    if(LoRa.available()>4)return;
    while (LoRa.available()) {
      dato[i]=LoRa.read();
      SerialUSB.println(dato[i]);
      i++;
    }
    digitalWrite(Relay_1,!dato[0]);
    digitalWrite(Relay_2,!dato[1]);
    digitalWrite(Relay_3,!dato[2]);
    digitalWrite(14,LOW);
  }
}
