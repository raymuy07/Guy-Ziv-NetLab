#include "EthernetLab.h"


//Guy code:

uint8_t destination_address;
uint8_t source_address;
uint8_t frame_type;
uint8_t payload_length;
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
int ack_Sn=0;
int pre_ack_sn = -1 ;
int send_ack_flag = 1;
void setup() {
	
	setAddress(RX,10);  

}



//Ziv's Code with notes
void extract_raw_data(uint8_t *raw_data){


	destination_address = raw_data[0];
    source_address = raw_data[1];
    frame_type = raw_data[2];
    payload_length = raw_data[3];//SHOULD BE 17

    // Copy payload and ensure null-termination
    memset(payload_data, 0, sizeof(payload_data));  // Clear payload buffer
    memcpy(payload_data, &raw_data[4], payload_length);
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
  send_buffer[4] = ack_Sn;     
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
	  //delay(100);
    /*Serial.println("Raw Data Received:");
        for (int i = 0; i < 25; i++) {
            Serial.print(receive_buffer[i], HEX);
            Serial.print(" ");
        }*/
	
	extract_raw_data(receive_buffer);
	if (calculated_crc == received_crc) {
      // Serial.println("CRC Verification: SUCCESS");
    /*int rec_packet_num = (payload_data[0]&0x07);
		
    //this means that the sending sequence started.
    if (rec_packet_num == 0){
    
    send_ack_num = rec_packet_num+1;

    }
    // this should stop sending duplicate acks.
    if (send_ack_num == rec_packet_num){

      
    }*/
    ack_Sn = ((payload_data[0]&0x07)+1);
    if (ack_Sn == 8){
      ack_Sn = 0;
    }
    
    if (ack_Sn != pre_ack_sn){
      pre_ack_sn = ack_Sn;
    } else{
      //pre_ack_sn = ack_Sn;
      send_ack_flag = 0;
    }
    print_data();
  }
  if (send_ack_flag){
  build_ack_packet();//should be ack with earlier sn
  //delay(1000);
  

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
} send_ack_flag =1;
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



