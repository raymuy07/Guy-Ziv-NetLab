#include "EthernetLab.h"



// Guy code:

uint8_t frame_type = 0x00;
uint8_t source_address = 0x1A ;
uint8_t destination_address = 0x0A ;
uint8_t ack_sn_data = 0x00;
int sn_index=0;
int recieved_ack=0;
int rn_index=0;
char payload_data[][16] = {
    "Leiba & Zaidman",
    "Zaidman & Leiba",
    "Ziv & Guy121234",
    "Guy & Ziv121234"
};
uint8_t payload_length = strlen(payload_data[0]);
unsigned long ref_time,current_time;
unsigned long time_out = 10000;//0.5 sec
//int payload_data_index=0;
uint8_t send_buffer[25];  // 4 header + 16 payload + 4 CRC
uint8_t ack_buffer[25];
int packet_index = 0;



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

	ack_sn_data = payload_length << 1 ;
	ack_sn_data = ack_sn_data || sn_index;
	send_buffer[4] = ack_sn_data;	
	
	
    memcpy(&send_buffer[5], payload_data[packet_index], payload_length);     
	
	
	unsigned long crc = calculateCRC(send_buffer[0], 20);
	memcpy(&send_buffer[5 + payload_length], &crc, 4);     // copy CRC 
	
  //Serial.println("Serialized Buffer on TX:");
for (int i = 0; i < 24; i++) {
    Serial.print((char)send_buffer[i]);
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
	
	sn_index = ack_buffer[4] & 0x01;

	
	if (sn_index != rn_index){
	Serial.print(" recived rn^1: ");		
  Serial.print(sn_index);
  delay(100);
			sn_index = rn_index;
			packet_index ++;
      if (packet_index == 4 ){//reset packet_index
        packet_index = 0;
      }
      current_time = ref_time;
	}
  else{
    Serial.print(" recived rn: ");		
    Serial.print(sn_index);
    delay(100);
    current_time = ref_time;
  }
		
	//build the next packet or the same packet
	
	//build_packet(packet_index); 
	build_packet();
	ref_time = current_time;
  delay(100);
	sendPackage(send_buffer, sizeof(send_buffer));

	
	}
	
	
	
}





void loop() {
	
	current_time = millis();
  delay(100);
	is_ack();

	
	
	
	
	
	// if for timeout and sending the same data

	if (current_time - ref_time >= time_out) {

		Serial.print("time out reached");		
    Serial.print(" current_time - ref_time: ");		
    Serial.print(current_time - ref_time);		
    Serial.print(" time out: ");		
    Serial.print(time_out);		
    //Serial.print(sn_index);
		
		

		build_packet(); 
    delay(100);
		sendPackage(send_buffer, sizeof(send_buffer));
		rn_index = sn_index;
		
		ref_time = current_time;
		
		recieved_ack = 0;
		
	}
	
	
	
	current_time = millis();
	
  
  /*if (sendPackage(send_buffer, sizeof(send_buffer))) {
        
		//Serial.println("Frame sent successfully!");
		build_packet();
    } 
	
	else {
        //Serial.println("Line busy, retrying...");
		delay(1000);
    }
  */
  //delay(1000);
}
