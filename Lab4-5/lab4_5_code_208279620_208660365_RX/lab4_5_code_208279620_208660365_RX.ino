#include "EthernetLab.h"
#define TX_PIN 5                 // Transmission pin
#define RX_PIN 4                 // Reception pin

char raw_data[24]={0};
int pay_load_size=0;






typedef struct Frame {
uint8_t destination_address;
uint8_t source_address;
uint8_t frame_type;
uint8_t length;
uint8_t payload[16];
uint32_t FCS;
} frame;







void setup() {
	setAddress(RX,10);  
	randomSeed(analogRead(0));       // makes it truly random
}
void build_raw_data(){
	char string[16] ="Leiba & Zaidman";
	raw_data[0] = 0x1A;
	raw_data[1] = 0x0A;
	raw_data[2] = 0;
	raw_data[3] = 16;
	raw_data[4] = string;
	setAddress(TX,10);  
	unsigned long CRC_calc = calculateCRC(raw_data,24);
	setAddress(RX,10);
	raw_data[20] = CRC_calc;
	Serial.println(raw_data);
}



void loop() {
  if (readPackage(raw_data,pay_load_size)==0){
    /////////////////////////////////////////////////////////
    //internal testing - for Ziv
    build_raw_data();
    frame* receivedFrame = (frame*)raw_data; // Cast raw_data to a Frame pointer
    int CRC_check = calculateCRC(raw_data,24);
    if (CRC_check){
      Serial.println("payload recived correctly: ");
      Serial.println((char)receivedFrame->payload);
      
    }
    //////////////////////////////////////////////////////
  }
  else{
    frame* receivedFrame = (frame*)raw_data; // Cast raw_data to a Frame pointer
  }
	
	delay(100);	
}
  
