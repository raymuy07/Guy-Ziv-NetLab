// C++ code
//

#define Tx_Data_Line 5
#define Rx_Data_Line 4
#define BIT_wait_time 20

#define IDLE 0
#define START 1
#define DATA 2
#define PARITY 3
#define STOP 4


//times
long current_time;
long ref_time;
long random_wait_time;
long delta_time;

int samp_time = 3;
int rx_rec_data;

char data = 'a';
int counter_tx;

int index=0;
int state = IDLE ;
int result_data;
int rx_data =0;



void setup()
{
	//open_serial_port
	Serial.begin(9600); 
  
	pinMode(Tx_Data_Line, OUTPUT); 
	pinMode(Rx_Data_Line, INPUT); 
	
	///calculate delta_time depend on bit time
	delta_time = BIT_wait_time / (samp_time + 2);
    ref_time = millis();
	Serial.println("hey");


}


/*

void uart_tx(){
	
	current_time = millis(); 
	if (IDLE==1){
    
		//random wait time
		if(current_time-ref_time >= random_wait_time){
			IDLE=0;
			counter_tx= 0;
			
			ref_time = millis();
		
		}
	}
	else{
		
		
		
		
	}
}

*/



int wrapper_sample_data(int index = 0) {
	
	static int counter = 0;          // track number of calls
	static int sample = 0; 
	
	if (counter == 0) {
    counter = index; 	// Set counter to match the initial value of index
	}
  
	
	current_time = millis();//running over global current_time, maybe trouble timing bitTime
	if(current_time-ref_time >= delta_time){
		
		int bit = digitalRead(Rx_Data_Line);
		sample = (sample << 1) | bit;  // Shift left and add the new bit// could cause problems with real arduino
      	counter++;
	}
	
	if (counter == 5){             // Check if 5 samples have been collected
	
	  int result = sample & 0b1110;
      
      if(result == 14){
        result=1;
      }//need to loo at editing software, looks like we're reseting counter everytime (notepad)
      
      counter = 0;                  // Reset counter for the next cycle
      sample = 0;  

      ref_time = millis(); // Reset timing
      
      
      return result;                
	
	}
	
	ref_time = millis(); // Reset timing
	return -1;

}


void usart_rx(){//didnt look here yet
	
	
	switch (state) {
    
	case IDLE:
      rx_rec_data = digitalRead(Rx_Data_Line);
      if (rx_rec_data == 0) {
        state = START;
      }
      break;

    case START:
	  // Sample 4 times
	  result_data = wrapper_sample_data(index=1);
      Serial.println(result_data);

      if (result_data == 0) {
        state = DATA;
        Serial.println(state);

      }
      //state = DATA;
      break;

    
	case DATA:
	  result_data = wrapper_sample_data();
      Serial.println(result_data);

        if (result_data != -1) {
            int rec_bit = result_data;
            rx_data |= (rec_bit >> counter_tx);
            counter_tx++;
            if (counter_tx == 7) {
                state = PARITY;
                counter_tx = 0; 
            }
        }
		break;

    
	case PARITY:
        result_data = wrapper_sample_data();
        if (result_data != -1) {
            int parity_bit = result_data;
            // Add parity check logic here
            state = STOP;
        }
        break;

    case STOP:
        result_data = wrapper_sample_data();
        if (result_data != -1) {
            int end_bit = result_data;
            if (end_bit == 1) {
                state = IDLE;
                rx_data = 0; // Reset received data
            }
        }
        break;


}
  
}	
		

	


void loop() {
	
	usart_rx();
	
  //uart_tx();
 
}





