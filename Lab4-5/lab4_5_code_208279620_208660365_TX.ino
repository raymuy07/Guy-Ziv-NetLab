#define TX_PIN 5                 // Transmission pin
#define RX_PIN 4                 // Reception pin


char string_data[16]= "Leiba & Zaidman";

uint8_t type = 0;


typedef struct Frame {
uint8_t destination_address;
uint8_t source_address;
uint8_t frame_type;
uint8_t length;
uint8_t* payload;
uint32_t FCS;
} frame;







void setup() {
  
  setAddress(TX,10);
  
  uint8_t source_address = 0x1A ;
  uint8_t destination_address = 0x0A ;
  
  frame.type = type;
  frame.length = checklength(string_data);


  randomSeed(analogRead(0));       // makes it truly random
  


}

/*
int checklength(char arr[]){
	
	
}
*/



void loop() {
  
  delay(100);
}
