#include "EthernetLab.h"
#define window_size 3 //change window size from here

//Guy code:

uint8_t destination_address;
uint8_t source_address;
uint8_t frame_type;
uint8_t payload_length;

uint8_t ack_data;

// we know the packet is with fixed size.

uint8_t receive_buffer[25];  // 24 bytes: 4 header + 16 payload + 4 CRC


char payload_data[17];     // Buffer for payload data
char real_data[16];

char packets_buffer[250];

unsigned long received_crc;  // Received CRC
unsigned long calculated_crc; // Calculated CRC
uint8_t send_buffer[25];  // 4 header + 17 payload + 4 CRC

uint8_t ack_destination_address = 0x1A;
uint8_t ack_source_address = 0x0A; 
uint8_t ack_frame_type = 0;
uint8_t ack_payload_length = 17;

uint8_t ack_number = 0;

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
    

    memcpy(ack_data, &raw_data[4], 1);
    memcpy(real_data, &raw_data[5], payload_length-1);
    
	payload_data[payload_length] = '\0'; // Null-terminate the payload string

    // Extract CRC
    memcpy(&received_crc, &raw_data[4 + payload_length], 4);

    // Calculate CRC (should match TX logic)
    calculated_crc = calculateCRC(&raw_data[0], 21);
	
	//print_data();

	// Verify CRC
    if (calculated_crc == received_crc) {
        //Serial.println("CRC Verification: SUCCESS");
		
    } else {
        Serial.println("CRC Verification: FAILED");
        Serial.println("recived_CRC : ");
        Serial.println(received_crc,HEX);
        Serial.println("calc_CRC : ");
        Serial.println(calculated_crc,HEX);
    }
	
}

void build_ack_packet(){

  send_buffer[0] = ack_destination_address;                   // Destination address
  send_buffer[1] = ack_source_address;                        // Source address
  send_buffer[2] = ack_frame_type;                        	// Frame type (0)
  send_buffer[3] = ack_payload_length;                        // Payload length (fixed 16 bytes)
  send_buffer[4] = ack_number;     
	memcpy(&send_buffer[5], 0, payload_length-1);
	
	unsigned long crc = calculateCRC(&send_buffer[0], 21);
	memcpy(&send_buffer[4 + payload_length], &crc, 4);     // copy CRC 
	
	
	
}


void print_data(){
	
	
	/*
    Serial.println("Packet Received:");
    Serial.print("Destination Address: 0x"); Serial.println(destination_address, HEX);
    Serial.print("Source Address: 0x"); Serial.println(source_address, HEX);
    Serial.print("Frame Type: "); Serial.println(frame_type);
    Serial.print("Payload Length: "); Serial.println(payload_length,HEX);
    Serial.print("Received CRC: 0x"); Serial.println(received_crc, HEX);
    Serial.print("Calculated CRC: 0x"); Serial.println(calculated_crc, HEX);
    Serial.print("ack: "); Serial.println(payload_data[0],BIN);*/
    
	
	Serial.print("ack_sn: "); Serial.println(ack_Sn);
    Serial.print("pure data: "); Serial.println(real_data);
    
	
	
}

void loop() {

    
    // Check if a packet has been received
	if (readPackage(receive_buffer, sizeof(receive_buffer))) {
	
	extract_raw_data(receive_buffer);
	if (calculated_crc == received_crc) {
    
	// Serial.println("CRC Verification: SUCCESS");

	ack_number = (ack_data & 0x07);
    ack_number ++;
	if (ack_number == 7){
		
	ack_number = 0;
	}
	print_data();
  
  
  }

  build_ack_packet();//should be ack with earlier sn
  

  while (1){      
        int result = sendPackage(send_buffer, sizeof(send_buffer));
        //delay (10000);
        if (result == 0) {
            //Serial.println("Line busy. Retrying...");
        } else {
            //Serial.println("sent ack:");
            break;
        }
  }
  
  //Serial.print("sent ACK sn: "); Serial.println(ack_Sn, HEX);
  /*int sent = sendPackage(send_buffer, 25);
  //delay(100);
  //Serial.print("did sent? "); Serial.println(sent);
  Serial.println("ack packet sent:");
        for (int i = 0; i < 25; i++) {
            Serial.print(send_buffer[i], HEX);
            Serial.print(" ");
        }
  Serial.print("sent ACK sn: "); Serial.println(ack_Sn, HEX);*/

	}
}



