#include "EthernetLab.h"



// Guy code:

uint8_t frame_type = 0x00;
uint8_t source_address = 0x1A ;
uint8_t destination_address = 0x0A ;
unsigned long start_RTT_measurment=0;
unsigned long stop_RTT_measurment=0;
unsigned long RTT=0;
unsigned long average_RTT=0;

int RTT_counter=0;
float bad_frames_counter=0;
float total_frames_counter=0;
float eror_prob;
uint8_t sn_index = 0;                        // Sequence number of frame
uint8_t rn_index = 0;                        // Received ACK number
uint8_t ack_sn_data = 0x00;                  // Field combining ACK/DATA and SN
unsigned long zero_time=0;


//Dont think its neccesary in my implemintation.
int dataset_index=0;
///

char dataset[][16] = {"Leiba & Zaidman","Zaidman & Leiba","Guy & Ziv123456","Ziv & Guy123456","Ziv & Guy654321","Ziv & Guy678909","Guy123456 & Ziv", "Guy 123456& Ziv" };

int data_set_length = sizeof(dataset);

char payload_data[16]= {0};
uint8_t payload_length = 17;


unsigned long ref_time, current_time;
const unsigned long time_out = 2200;         // timeout duration in ms

uint8_t send_buffer[25];  // 4 header + 16 payload + 4 CRC
uint8_t ack_buffer[25];





//the new addition for GBN
//should be as define above
int window_size = 2;
int s_frame_index = 0; 
int i = 0;
int f_frame_index = window_size - 1; 





void setup() {
  
  zero_time=millis();
  
  setAddress(TX, 10);   
  //Serial.println("Transmitter initialized.");
  
  
  ref_time = millis();
}



void build_packet(int index){
	
    
	memset(send_buffer, 0, sizeof(send_buffer));  // Clear buffer
  
  
  
	send_buffer[0] = destination_address;                   // Destination address
    send_buffer[1] = source_address;                        // Source address
    send_buffer[2] = frame_type;                        	// Frame type (0)
    send_buffer[3] = payload_length;                        // Payload length (fixed 16 bytes)
	
	

	///Moving the payload_length three times and then anding with 7.

	
	ack_sn_data = (payload_length << 3) | (index & 0x07);
	
	send_buffer[4] = ack_sn_data;	
	
	
	
	//Thats How I build the payload data, each time the window is increasing I build different packet.
	
	strncpy(payload_data, dataset[index], payload_length-2);
	payload_data[payload_length-2] = '\0'; // Ensure null-termination
    memcpy(&send_buffer[5], payload_data, payload_length-1);     
	
	
	unsigned long crc = calculateCRC(&send_buffer[0], 21);// --------------------shoudlnt be 21?------------------
 
 
 
	memcpy(&send_buffer[5 + payload_length-1], &crc, 4);     // copy CRC 
	
	
	
}


void is_time_out() {
	
    current_time = millis();

    // timeout check
    
	if (current_time - ref_time >= time_out) {
		
		
		
		i = s_frame_index;
		build_packet(i);        // build packet in window


		while (i <= f_frame_index){
			start_RTT_measurment = millis();
			int result = sendPackage(send_buffer, sizeof(send_buffer));
        
		
			if (result == 1) {
				
				i++;
				
				build_packet(i);        // build next packet in window


				total_frames_counter++;
				Serial.println("Timeout! Resending frame...");
				Serial.println("Packet resent successfully.");
				ref_time = millis();
				
				//delay(400);//need to kick?
			}
			
		}
	}
}


void is_ack() {
	
	
    bool received_ack = readPackage(ack_buffer, sizeof(ack_buffer));
    
	if (received_ack) {
		
		ref_time = millis();
		//RTT and eror propability calculations
		
		
		stop_RTT_measurment = millis();
		RTT = (stop_RTT_measurment - start_RTT_measurment);
		average_RTT = ((average_RTT*RTT_counter)+RTT)/(RTT_counter+1);
		RTT_counter++;
		eror_prob = ((bad_frames_counter)/(total_frames_counter));
    
	
	
		uint8_t ack_sn = ack_buffer[4] & 0x07;  // Extract SN bit from ACK


	  Serial.println("ack_sn: ");
    Serial.println(ack_sn);
	  Serial.println("RTT (ms): ");
    Serial.println(RTT);
	  Serial.println("average RTT (ms): ");
    Serial.println(average_RTT);
	  Serial.println("Eror propability: ");
    Serial.println(eror_prob);
    
	
	calc_efficency();
    
	if (s_frame_index-ack_sn==7){
  
        s_frame_index = 0;
		f_frame_index = window_size - 1; 
		return;
  }
	// Correct ACK received its only one above sn_index
	if ((ack_sn - s_frame_index == 1)) {  
      
	  
	  //lets increase the indexes and the move the frame one step right.
	  
	  s_frame_index++;	
	  f_frame_index++;
	  
	  
	  if (f_frame_index == data_set_length){
		  f_frame_index = data_set_length - 1;
	  }
	  
	  //This is the reset button
	  if (s_frame_index > f_frame_index){
		  
        s_frame_index = 0;
		f_frame_index = window_size - 1; 
		return;
		
	  }


		//Now that we recieved a correct ack we want to create and 
		//Send the next packet immidaitly.
		
		
			build_packet(s_frame_index);  
			while (1){  

			
			  int result = sendPackage(send_buffer, sizeof(send_buffer));

			  if (result == 1) {
				  
				  //The sending was finished and we can go back to our routine.
				  start_RTT_measurment = millis();
				  total_frames_counter++;
				  break;
				 
				}
			}

	
	
	ref_time = millis();   // Reset timer

	//In case bad ack recived.
	}	
	else { 
		  bad_frames_counter++;
	  }
    
	
    received_ack = 0;
  
  }
}









void calc_efficency(){
	
	
  float X = total_frames_counter-bad_frames_counter;//num of successfully sent frames
  float D = 16;
  current_time = millis();
  unsigned long T = (current_time - zero_time)/1000;
  float R = 10000000;
  float efficency = (X*D)/(T*R);
  

  /*Serial.println("X: ");
  Serial.println(X);
    Serial.println("T: ");
  Serial.println(T);
    Serial.println("D: ");
  Serial.println(D);
    Serial.println("R: ");
  Serial.println(R);*/


  Serial.println("Efficency: ");
  Serial.print(efficency,9);
  Serial.println();
}




void testCRC(){
  char testString[] = {"Leiba & Zaidman"};
  char frame[10] = {0};
  for (int i =0; i<6; i++){
    frame[i] = testString[i];
  }
  unsigned long crc_test = calculateCRC(testString,15);
  Serial.println("crc_test: ");
  Serial.println(crc_test, HEX);
  delay(10000);

}



void loop() {
	
	
	is_time_out();
	is_ack();


}
