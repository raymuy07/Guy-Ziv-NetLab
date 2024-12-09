#define TX_PIN 5                 // Transmission pin
#define RX_PIN 4                 // Reception pin


typedef struct Frame {
uint8_t destination_address;
uint8_t source_address;
uint8_t frame_type;
uint8_t length;
uint8_t* payload;
uint32_t FCS;
} frame;







void setup() {
  
  
  
  pinMode(TX_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT);
  digitalWrite(TX_PIN, HIGH);      // Set line HIGH (idle state)
  randomSeed(analogRead(0));       // makes it truly random
  


}




void loop() {
  
  delay(100);
}
