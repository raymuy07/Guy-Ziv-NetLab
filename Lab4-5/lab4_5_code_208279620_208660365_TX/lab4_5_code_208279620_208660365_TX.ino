#include "EthernetLab.h"



// Guy code:

uint8_t frame_type = 0x00;
uint8_t source_address = 0x1A ;
uint8_t destination_address = 0x0A ;


uint8_t sn_index = 0;                        // Sequence number of frame
uint8_t rn_index = 0;                        // Received ACK number
uint8_t ack_sn_data = 0x00;                  // Field combining ACK/DATA and SN


char payload_data[16]= "Leiba & Zaidman";
uint8_t payload_length = 17;


unsigned long ref_time, current_time;
const unsigned long time_out = 10000;         // timeout duration in ms

uint8_t send_buffer[25];  // 4 header + 16 payload + 4 CRC
uint8_t ack_buffer[25];



void setup() {
  
  setAddress(TX, 10);   
  //Serial.println("Transmitter initialized.");
  build_packet();
  ref_time = millis();

}



void build_packet(){
	
    
	memset(send_buffer, 0, sizeof(send_buffer));  // Clear buffer
	
	send_buffer[0] = destination_address;                   // Destination address
    send_buffer[1] = source_address;                        // Source address
    send_buffer[2] = frame_type;                        	// Frame type (0)
    send_buffer[3] = payload_length;                        // Payload length (fixed 16 bytes)
	
	
	///Here I need to shift the size to the left

	ack_sn_data = (payload_length << 1) | (sn_index & 0x01);
	send_buffer[4] = ack_sn_data;	
	
	
    memcpy(&send_buffer[5], payload_data, payload_length-1);     
	
	
	unsigned long crc = calculateCRC(send_buffer[0], 21);// --------------------shoudlnt be 21?------------------
	memcpy(&send_buffer[5 + payload_length-1], &crc, 4);     // copy CRC 
	
	
	/* DEBUG
	Serial.println("Serialized Buffer on TX:");
	for (int i = 0; i < 25; i++) {
    Serial.print(send_buffer[i], HEX);
    Serial.print("/.n");
    Serial.println(" ");
	}	*/

	
}


void is_time_out() {
	
    current_time = millis();

    // timeout check
    
	if (current_time - ref_time >= time_out) {
        //build_packet();//added to see if CRC changes
        //sendPackage(send_buffer, sizeof(send_buffer));
        int result = sendPackage(send_buffer, sizeof(send_buffer));
        //delay (3000);
        if (result == 1) {
            Serial.println("Timeout! Resending frame...");
            Serial.println("Packet resent successfully.");
            ref_time = millis();
        }
        
    }
}



void is_ack() {
	
	
    bool received_ack = readPackage(ack_buffer, sizeof(ack_buffer));
    
	if (received_ack) {
    ref_time = millis();
    Serial.println(" ACK received ");
		uint8_t ack_sn = ack_buffer[4] & 0x01;  // Extract SN bit from ACK
    Serial.println(" ack_sn: ");
    Serial.println(ack_sn);
    Serial.println(" sn_index: ");
    Serial.println(sn_index);
    if (ack_sn != sn_index) {  // Correct ACK received
  
      Serial.println("Correct ACK received. Sending next frame...");
      sn_index ^= 0x01;      // Flip SN (0 -> 1, 1 -> 0)
          
    //it is the same packet but in case there will be more data...
    }
    build_packet();        // build next packet
    while (1){      
      int result = sendPackage(send_buffer, sizeof(send_buffer));
      //delay (10000);
      if (result == 0) {
          //Serial.println("Line busy. Retrying...");
      } else {
          Serial.println("got ack, Packet sent successfully.");
          break;
      }
    }
    ref_time = millis();   // Reset timer
    Serial.println("broke");
    received_ack = 0;
  }
}





void loop() {
	
	is_time_out();  
  is_ack();    

}
