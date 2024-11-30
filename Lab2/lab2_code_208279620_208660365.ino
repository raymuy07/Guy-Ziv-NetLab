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
long Rx_ref_time , Tx_ref_time;
long random_wait_time;
long delta_time;

int samp_time = 3;
int rx_rec_data;

char data = 'a';

int index=0;
int state_rx = IDLE ;
int state_tx = IDLE;
int result_data;



void setup()
{
	//open_serial_port
	Serial.begin(9600); 
  
	pinMode(Tx_Data_Line, OUTPUT); 
	pinMode(Rx_Data_Line, INPUT); 
	
	///calculate delta_time depend on bit time
	delta_time = BIT_wait_time / (samp_time + 2);
    
	
	Rx_ref_time = millis();
	Tx_ref_time = millis();
	
	Serial.println("hey");


}



void usart_tx(){
	
	current_time = millis(); 
	static int counter_tx = 0;
	static int parity_bit = 0;	
	
	if (state_tx == IDLE){
    
		//random wait time
		if(current_time - Tx_ref_time >= random_wait_time){
			
			state_tx = START;
			Tx_ref_time = millis();
		
		}
	}
	else if(current_time - Tx_ref_time >= BIT_wait_time){
	
		switch(state_tx){
		  
		  case(START):
		  parity_bit = 0;
		  counter_tx = 0;
		  digitalWrite(Tx_Data_Line, LOW);
		  state_tx = DATA;
		  Tx_ref_time = millis();
		  break;

		  case(DATA):
		  if (counter_tx < 8) { 
			  int current_bit = bitRead(data , counter_tx);
			  digitalWrite(Tx_Data_Line, current_bit);
			  parity_bit ^= current_bit;                 // Calculate parity using XOR
			  counter_tx++;
			  Tx_ref_time = millis();

		  }else {
		     state_tx = PARITY;
			 Tx_ref_time = millis();

		  }
		  break;
		  
		  
		  case(PARITY):		  
		  digitalWrite(Tx_Data_Line, parity_bit ^ 1);
		  state_tx = STOP;
		  Tx_ref_time = millis();
		  break;
		
		  case(STOP):
		  digitalWrite(Tx_Data_Line, HIGH);
		  state_tx = IDLE;
		  Tx_ref_time = millis();
		  break;

		
		
	}
}

}


int wrapper_sample_data(int &index) {
	
	static int counter = 0;          // track number of calls
	static int sample = 0; 
	
	if (counter == 0) {
    	counter = index; 	// Set counter to match the initial value of index
	}
	
	current_time = millis();
	if(current_time- Rx_ref_time >= delta_time){
		
		int bit = digitalRead(Rx_Data_Line);
		sample = (sample << 1) | bit;  // Shift left and add the new bit// could cause problems with real arduino
      	counter++;

	}
	
	if (counter == 5){             // Check if 5 samples have been collected
	
	  int result = sample & 0b1110;
      
      if(result == 14){
        result=1;
      } else if (result == 0){
        result=0;
      }
      else{
        result= -1;
      }
      
      counter = 0;                  // Reset counter for the next cycle
      sample = 0;  

      Rx_ref_time = millis(); // Reset timing

      return result;                
	
	}
	
	Rx_ref_time = millis(); // Reset timing
	return -1;

}


void usart_rx(){
	
	static int counter_rx = 0;
	static int data_parity_bit = 0;	
    static int rx_parity_bit = 0;	
    static int rx_data = 0;

	switch (state_rx) {
    
	case IDLE:
      rx_rec_data = digitalRead(Rx_Data_Line);
      if (rx_rec_data == 0) {
        state_rx = START;
      }
      break;

    case START:
	  // Sample 4 times
	  result_data = wrapper_sample_data(index=1);

      if (result_data == 0) {
        state_rx = DATA;
      }
      break;

    
	case DATA:
	
	  result_data = wrapper_sample_data(index=0);

        if (result_data != -1) {
            int rec_bit = result_data;
			data_parity_bit ^= rec_bit;                 // Calculate parity using XOR

          	Serial.println("rec");
		    Serial.println(rec_bit);

            rx_data |= (rec_bit >> counter_rx);
            counter_rx++;
            if (counter_rx == 7) {
                state_rx = PARITY;
                counter_rx = 0; 
            }
        }
		break;

    
	case PARITY:
        result_data = wrapper_sample_data(index=0);
        if (result_data != -1) {
            
            rx_parity_bit = result_data;
            state_rx = STOP;
        
        }
        break;

    case STOP:
        result_data = wrapper_sample_data(index=0);
        if (result_data != -1) {
            int end_bit = result_data;
            if (end_bit == 1) {
                
				if (rx_parity_bit == data_parity_bit){
				
					Serial.println((char)rx_data);
				}
				
				Serial.println("END");
				state_rx = IDLE;
                rx_data = 0; 
                rx_parity_bit = 0;
                data_parity_bit = 0;

            }
        }
        break;


}
  
}	
		

	


void loop() {
	
	usart_rx();
    
	Serial.println("sample");
	usart_tx();
 
  
  
}






