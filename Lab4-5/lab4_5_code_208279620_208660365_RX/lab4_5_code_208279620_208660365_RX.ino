#include "EthernetLab.h"



char raw_data[24]={0};
int pay_load_size=0;



//Guy code:

uint8_t destination_address = 0x1A ;
uint8_t source_address;              // sender's address 
uint8_t frame_type;                  // Frame type 
uint8_t payload_length;              // Payload length 
char payload_data[16];               // Payload data buffer
unsigned long received_crc;          // Received CRC
unsigned long calculated_crc;        // Calculated CRC



void setup() {
	
	
	setAddress(RX,10);  
	Serial.println("Reciever initialized.");

}


void extract_packet(uint8_t *receive_buffer) {
    
	
	//Extract the fields
    
	destination_address = receive_buffer[0];   // Destination address
    source_address = receive_buffer[1];        // Source address
    frame_type = receive_buffer[2];            // Frame type
    payload_length = receive_buffer[3];        // Payload length
    
    //Extract the payload
    memset(payload_data, 0, sizeof(payload_data));  // Clear payload buffer
    memcpy(payload_data, &receive_buffer[4], payload_length);
    
    //Extract the CRC
    memcpy(&received_crc, &receive_buffer[4 + payload_length], 4);
    

	//calculate CRC
    calculated_crc = calculateCRC(payload_data, payload_length);
    
    

	print_data();
	
    // Step 6: Verify CRC
    if (calculated_crc == received_crc) {
        Serial.println("CRC Verification: SUCCESS");
    } else {
        Serial.println("CRC Verification: FAILED");
    }
}


//Ziv's Code with notes
void extract_raw_data(uint8_t *raw_data){


	destination_address = raw_data[0];
	source_address = raw_data[1];
	frame_type = raw_data[2];
	payload_length = raw_data[3];
	
	memset(payload_data, 0, sizeof(payload_data));  // Clear payload buffer
    memcpy(payload_data, &receive_buffer[4], payload_length);
	
	memcpy(&received_crc, &receive_buffer[4 + payload_length], 4); //recieve the crc
    
	calculated_crc = calculateCRC(payload_data, payload_length);

	
	// Step 6: Verify CRC
    if (calculated_crc == received_crc) {
        Serial.println("CRC Verification: SUCCESS");
    } else {
        Serial.println("CRC Verification: FAILED");
    }
}


void print_data(){
	
    Serial.println("Packet Received:");
    Serial.print("Destination Address: 0x"); Serial.println(destination_address, HEX);
    Serial.print("Source Address: 0x"); Serial.println(source_address, HEX);
    Serial.print("Frame Type: "); Serial.println(frame_type);
    Serial.print("Payload Length: "); Serial.println(payload_length);
    Serial.print("Payload: "); Serial.println(payload_data);
    Serial.print("Received CRC: 0x"); Serial.println(received_crc, HEX);
    Serial.print("Calculated CRC: 0x"); Serial.println(calculated_crc, HEX);
	
	
}
void loop() {
    
	
	// we know the packet is with fixed size.
	
	uint8_t receive_buffer[24];  // 24 bytes: 4 header + 16 payload + 4 CRC
    
    // Check if a packet has been received
	if (readPackage(receive_buffer, sizeof(receive_buffer))) {
		
        extract_raw_data(receive_buffer); 
    
	} else {
        delay(100);
    }
	
	

/*

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
  
*/



