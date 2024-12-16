#include "EthernetLab.h"



// Guy code:

uint8_t frame_type = 0x00;
uint8_t source_address = 0x1A ;
uint8_t destination_address = 0x0A ;


char payload_data[]= "Leiba & Zaidman";
uint8_t payload_length = strlen(payload_data);

uint8_t send_buffer[24];  // 4 header + 16 payload + 4 CRC



void setup() {
  
  setAddress(TX, 10);   
  Serial.println("Transmitter initialized.");
  

}



void build_packet(){
	
    
    
	memset(send_buffer, 0, sizeof(send_buffer));  // Clear buffer
	
	  send_buffer[0] = destination_address;                   // Destination address
    send_buffer[1] = source_address;                        // Source address
    send_buffer[2] = frame_type;                        	// Frame type (0)
    send_buffer[3] = payload_length;                        // Payload length (fixed 16 bytes)

    memcpy(&send_buffer[4], payload_data, payload_length);     
	
	
	unsigned long crc = calculateCRC(send_buffer[0], 20);
	memcpy(&send_buffer[4 + payload_length], &crc, 4);     // copy CRC 
	
  Serial.println("Serialized Buffer on TX:");
for (int i = 0; i < 24; i++) {
    Serial.print(send_buffer[i], HEX);
    Serial.print(" ");
}
Serial.println();

	
	
	
}



void loop() {
  
  if (sendPackage(send_buffer, sizeof(send_buffer))) {
        
		Serial.println("Frame sent successfully!");
		build_packet();
    } 
	
	else {
        Serial.println("Line busy, retrying...");
		delay(1000);
    }

}
