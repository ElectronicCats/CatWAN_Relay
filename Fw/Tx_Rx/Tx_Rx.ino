#include <SPI.h>
#include <LoRa.h>
#define Serial SerialUSB
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
    LoRa.beginPacket();
    LoRa.write(digitalRead(0));
    LoRa.write(digitalRead(1));
    LoRa.write(digitalRead(2));
    LoRa.endPacket();
    SerialUSB.println("Dato enviado");
  }
  
void setup() {
  SerialUSB.begin(115200);

  SerialUSB.println("LoRa Sender");
  LoRa.setPins(17, 16, 4);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  pinMode(0,INPUT); 
  pinMode(1,INPUT); 
  pinMode(2,INPUT); 
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(14,OUTPUT);

  attachInterrupt(0, interrupt_0, CHANGE);
  attachInterrupt(1,interrupt_1, CHANGE);
  attachInterrupt(2,interrupt_2, CHANGE);
  
  digitalWrite(5,LOW);
  digitalWrite(6,LOW);
  digitalWrite(7,LOW);
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
    digitalWrite(5,!dato[0]);
    digitalWrite(6,!dato[1]);
    digitalWrite(7,!dato[2]);
    digitalWrite(14,LOW);
  }
}
