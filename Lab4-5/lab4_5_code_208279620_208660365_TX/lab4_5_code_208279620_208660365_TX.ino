#include "EthernetLab.h"



// Guy code:

uint8_t frame_type = 0x00;
uint8_t source_address = 0x1A ;
uint8_t destination_address = 0x0A ;
uint8_t ack_sn_data = 0x00;

char payload_data[]= "Leiba & Zaidman";
uint8_t payload_length = strlen(payload_data);


uint8_t send_buffer[25];  // 4 header + 16 payload + 4 CRC
uint8_t ack_buffer[25];



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
	
	
	///Here I need to shift the size to the left

	ack_sn_data = payload_length <<;
	ack_sn_data = ack_sn_data || sn_index;
	send_buffer[4] = ack_sn_data;	
	
	
    memcpy(&send_buffer[5], payload_data, payload_length);     
	
	
	unsigned long crc = calculateCRC(send_buffer[0], 20);
	memcpy(&send_buffer[5 + payload_length], &crc, 4);     // copy CRC 
	
  Serial.println("Serialized Buffer on TX:");
for (int i = 0; i < 24; i++) {
    Serial.print(send_buffer[i], HEX);
    Serial.print(" ");
}
Serial.println();

	
	
	
}

void is_ack(){
	
	
	recieved_ack = readPackage(ack_buffer, sizeof(ack_buffer));

	if (recieved_ack){ 
	
	//check if the ack is for good packet or for bad packet.
	//if the sn_index is different then the rn_index meaning that 
	//the reciever got a good frame and we need to continue.
	
	sn_index = ack_buffer[4];

	
	if (sn_index != rn_index){
			
			sn_index = rn_index;
			//packet_index ++;
	}
		
	//build the next packet or the same packet
	
	//build_packet(packet_index); 
	build_packet()
	ref_time = current_time;
	sendPackage(send_buffer, sizeof(send_buffer));

	
	}
	
	
	
}



//to be moved
unsigned long ref_time,current_time;

void loop() {
	
	current_time = millis();

	is_ack();
	
	
	
	int packet_index = 0;

	
	
	
	
	
	// if for timeout and sending the same data

	if (current_time - ref_time >= time_out) {
		
		
		

		build_packet(packet_index); 

		sendPackage(send_buffer, sizeof(send_buffer));
		rn_index = sn_index;
		
		ref_time = current_time;
		
		recieve_ack = 0;
		
	}
	
	
	
	current_time = millis();
	
  
  if (sendPackage(send_buffer, sizeof(send_buffer))) {
        
		Serial.println("Frame sent successfully!");
		build_packet();
    } 
	
	else {
        Serial.println("Line busy, retrying...");
		delay(1000);
    }

}
