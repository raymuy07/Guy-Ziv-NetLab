#include "EthernetLab.h"



// Guy code:

uint8_t frame_type = 0x00;
uint8_t source_address = 0x1A ;
uint8_t destination_address = 0x0A ;


char payload_data[]= "Leiba & Zaidman";
uint8_t payload_length = strlen(payload_data);




void setup() {
  
  setAddress(TX, 10);   
  Serial.println("Transmitter initialized.");
  

}



void build_packet(){
	
    
    uint8_t send_buffer[24];  // 4 header + 16 payload + 4 CRC
    
	memset(send_buffer, 0, sizeof(send_buffer));  // Clear buffer
	
	send_buffer[0] = destination_address;                   // Destination address
    send_buffer[1] = source_address;                        // Source address
    send_buffer[2] = frame_type;                        	// Frame type (0)
    send_buffer[3] = payload_length;                        // Payload length (fixed 16 bytes)

    memcpy(&send_buffer[4], frame.payload, payload_length);     
	
	
	unsigned long crc = calculateCRC(payload_data, frame.length);

	memcpy(&send_buffer[4 + payload_length], &crc, 4);     // copy CRC 
	

	
	
	
}



void loop() {
  
  if (sendPackage(send_buffer, sizeof(send_buffer))) {
        
		Serial.println("Frame sent successfully!");
		build_packet();
    } 
	
	else {
        Serial.println("Line busy, retrying...");
		delay(200);
    }

}
