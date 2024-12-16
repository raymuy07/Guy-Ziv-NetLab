#include "EthernetLab.h"


//Guy code:

uint8_t destination_address;
uint8_t source_address;
uint8_t frame_type;
uint8_t payload_length;

int8_t raw_data[24];      // Buffer for raw data
char payload_data[16];     // Buffer for payload data
unsigned long received_crc;  // Received CRC
unsigned long calculated_crc; // Calculated CRC



void setup() {
	
	setAddress(RX,10);  

}



//Ziv's Code with notes
void extract_raw_data(uint8_t *raw_data){


	  destination_address = raw_data[0];
    source_address = raw_data[1];
    frame_type = raw_data[2];
    payload_length = raw_data[3];

    // Copy payload and ensure null-termination
    memset(payload_data, 0, sizeof(payload_data));  // Clear payload buffer
    memcpy(payload_data, &raw_data[4], payload_length);
    payload_data[payload_length] = '\0'; // Null-terminate the payload string

    // Extract CRC
    memcpy(&received_crc, &raw_data[4 + payload_length], 4);

    // Calculate CRC (should match TX logic)
    calculated_crc = calculateCRC(&raw_data[0], 4 + payload_length);
	
	print_data();

	// Verify CRC
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


//
    Serial.println("Raw Data Received:");
        
        for (int i = 0; i < 24; i++) {
            Serial.print(receive_buffer[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
		//

        extract_raw_data(receive_buffer); 
    
	} else {
        delay(100);
    }
	
}



