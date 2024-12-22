#include "EthernetLab.h"
#define window_size 3 //change window size from here


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
int dataset_index=0;
char dataset[][16] = {"Leiba & Zaidman","Zaidman & Leiba","Guy & Ziv123456","Ziv & Guy123456","123456789012345","161514131298765","ThisIs a mesage"};
char payload_data[16]= {0};
const int dataset_length = sizeof(dataset); // might need to move
char window_index_list[dataset_length]={0};
uint8_t payload_length = 17;
uint8_t ack_sn;
int next_window_index=1;
int ack_index=0;


unsigned long ref_time, current_time;
const unsigned long time_out = 2400; // average RTT about 2000ms, adding 1/5 of RTT

uint8_t send_buffer[25];  // 4 header + 16 payload + 4 CRC
uint8_t ack_buffer[25];



void setup() {
  zero_time=millis();
  setAddress(TX, 10);   
  //Serial.println("Transmitter initialized.");
  strncpy(payload_data, dataset[0], payload_length-2);
  payload_data[payload_length-2] = '\0'; // Ensure null-termination
  build_packet();
  ref_time = millis();
  for (int i=0 ; i<window_size ; i++){ // creating window_index_list, initioate with 1, not 0!!
	  window_index_list[i] = next_window_index++;
  }
  Serial.println("window index init: ");
  for (int i =0; i < window_size; i++){
  Serial.println(window_index_list[i],HEX);
  }
}



void build_packet(){
	
    
	memset(send_buffer, 0, sizeof(send_buffer));  // Clear buffer
  //Serial.println("send_buffer should be zero: ");
  /*for( int i=0; i<25 ; i++){
    Serial.println(send_buffer[i], HEX);
    Serial.println(" ");
  }*/
	send_buffer[0] = destination_address;                   // Destination address
    send_buffer[1] = source_address;                        // Source address
    send_buffer[2] = frame_type;                        	// Frame type (0)
    send_buffer[3] = payload_length;                        // Payload length (fixed 16 bytes)
	
	
	///Here I need to shift the size to the left

	//ack_sn_data = (payload_length << 1) | (sn_index & 0x01);
  ack_sn_data = sn_index; //removing the data length information, cant see how can be both ack and data length.
	send_buffer[4] = ack_sn_data;	
	
	
    memcpy(&send_buffer[5], payload_data, payload_length-1);     
	
	
	unsigned long crc = calculateCRC(&send_buffer[0], 21);//create CRC
 /* Serial.println("send_buffer calcing crc: ");
 for( int i=0; i<25 ; i++){
    Serial.println(send_buffer[i], HEX);
    Serial.println(" ");
  }*/
  //Serial.println("crc: ");
  //Serial.println(crc, HEX);
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
        int result = sendPackage(send_buffer, sizeof(send_buffer));
        if (result == 1) {
            start_RTT_measurment = millis();
            total_frames_counter++;
            Serial.println("Timeout! Resending frame...");
            Serial.println("Packet resent successfully.");
            ref_time = millis();
        }
        
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
  Serial.println("----------------------------------------------------");
}
void check_sn(){

if (ack_index = index_of(ack_sn,(int *)window_index_list) == 0) {  // ack of first packet only - Correct ACK received
     /*
      need a func that:
      -removes ack_sn from window_index list // removes only first ack index!! done
      -shifts all the rest of the nums, that way the oldest one is at window_index_list[0]// done inside of pop()
      -adds the "next num" - var not existing yet. - done
      -resets if next_num reaches limit, our choice - done
     */
     pop((char)window_index_list, ack_index);
     window_index_list[window_size-1] = next_window_index; // add the sent packet expected ack     
     next_window_index++;
     if (next_window_index == window_size*2){
      next_window_index = 0;
     }
     // make sure window_index_list actualy changes:
     Serial.println("window index list: ");
     for (int i =0; i < window_size; i++){
      Serial.println(window_index_list[i],HEX);
     }
     dataset_index++; // advance data index so next packet is sent.
      if (dataset_index == dataset_length){
        dataset_index = 0;
      }

    //Serial.println("Correct ACK received. Sending next frame...");

      strncpy(payload_data, dataset[dataset_index], payload_length-2);
      payload_data[payload_length-2] = '\0'; // Ensure null-termination
      sn_index ^= 0x01;      // Flip SN (0 -> 1, 1 -> 0)
    } 
    else{ //bad ack recived
      bad_frames_counter++;
      /*Serial.println("wrong ACK received. bad_frames_counter: ");
      Serial.println(bad_frames_counter);
      Serial.println("total_frames_counter: ");
      Serial.println(total_frames_counter);*/
    }
}


int index_of(int value, int *list) {
    int size = sizeof(list);
    for (int i = 0; i < size; i++) {
        if (list[i] == value) {
            return i;
        }
    }
    return -1; // Not found
}
/*
bool is_in_list(int var, char list){
  int list_size = sizeof(list);
  int in_list_flag=0;
  for(int i=0;i<list_size;i++){
    if (var = i){
      in_list_flag = 1;
    }
  }
  return in_list_flag;
}
*/

void pop(char* list, int index){
  for (int i=index;i < window_size; i++){
    list[i] = list[i+1];
  }
  list[window_size - 1] = '\0';
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
    ////////////
    //Serial.println(" ACK received ");
	ack_sn = ack_buffer[4];  // Extract SN bit from ACK
    /*Serial.println(" ack_sn: ");
    Serial.println(ack_sn);
    Serial.println(" sn_index: ");
    Serial.println(sn_index);*/
	  Serial.println("RTT (ms): ");
    Serial.println(RTT);
	  Serial.println("average RTT (ms): ");
    Serial.println(average_RTT);
	  Serial.println("Eror propability: ");
    Serial.println(eror_prob);
    calc_efficency();
    check_sn();

    build_packet();        // build next packet
    while (1){      
      int result = sendPackage(send_buffer, sizeof(send_buffer));
      //delay (10000);
      if (result == 0) {
          //Serial.println("Line busy. Retrying...");
      } else {
          //Serial.println("got ack, Packet sent successfully.");
		  start_RTT_measurment = millis();
		  total_frames_counter++;
          break;
      }
    }
    ref_time = millis();   // Reset timer
    received_ack = 0;
  }
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
	
	//is_time_out(); 
  //is_ack();
  //testCRC();  
  delay(3000);
  pop(window_index_list, 0);
  window_index_list[window_size-1] = next_window_index; // add the sent packet expected ack
  next_window_index++;
     if (next_window_index == window_size*2){
      next_window_index = 0;
     }
  // make sure window_index_list actualy changes:
  Serial.println("window index list after add: ");
  for (int i =0; i < window_size; i++){
  Serial.println(window_index_list[i],HEX);
  }
}
