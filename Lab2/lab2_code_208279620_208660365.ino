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
int IDLE=0;
int rx_rec_data;

char data = 'a';
int counter_tx;

void setup()
{
	//open_serial_port
	Serial.begin(9600); 
  
	pinMode(Tx_Data_Line, OUTPUT); 
	pinMode(Rx_Data_Line, INPUT); 
	
	///calculate delta_time depend on bit time
	delta_time = BIT_wait_time / (samp_time + 2);
    ref_time = millis();
	

}

//is idle

	//yes idle
		//check if random_time passed

	//no idle
		//check if data is set with all added bits
		//build new data block
		//if it time to send
		

		//check last state of clock




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



int wrapper_sample_data(int &index) {
	
	static int counter = 0;          // track number of calls
	static int sample = 0; 
	
	if (counter == 0) {
    counter = index; 	// Set counter to match the initial value of index
	}
  
	current_time = millis();
	
	if(current_time-ref_time >= delta_time){
		
		int bit = digitalRead(Rx_Data_Line);
		sample = (sample << index) | bit;   // Shift left and add the new bit
		counter++;
	}
	
	if (counter == 5){             // Check if 5 samples have been collected
	
		int result = sample & 0b1110; 
		counter = 0;                  // Reset counter for the next cycle
		sample = 0;       
		return result;                
	
	}
	
	return -1;

}

/*
void sample_data(int global_index){
	
	if (global_index > 4){
		global_index=0
	}
	
	
	current_time = millis();
	if(current_time-ref_time >= delta_time){
		
		int bit = digitalRead(Rx_Data_Line);
		sample = (sample << index) | bit;   // Shift left and add the new bit
		global_index++;
	
	
	}
	if (global_index > 4){
		global_index=0
	}
	ref_time = millis();
}

*/

void usart_rx(){
	
	
	switch (state) {
    
	case IDLE:
      rx_rec_data = digitalRead(Rx_Data_Line);
      if (rx_rec_data == 0) {
        state = START;
		global_index = 1;

      }
      break;

    case START:
      // Sample 4 more times
      sample_data(global_index);
      if (sample && 0b1110 == 0) {
        state = DATA;
      }
      break;

    case DATA:
      // Sample 5 times and then check bit
      sample_data(global_index);
      rec_bit = sample && 0b1110;
      rx_data |= rec_bit >> counter;
      counter++;
      if (counter == 7) {
        state = PARITY;
      }
      break;

    case PARITY:
      // Sample 5 times and then check bit
	  sample_data(global_index);
      parityBit = sample & 0b1110;
      // Check parity function here (implement parity check)
      state = STOP;
      break;

    case STOP:
      // Sample 5 times and then check bit
	  sample_data(global_index);
      ENDBit = sample & 0b1110;
      if (ENDBit == 1) {  // or 0 depending on your condition
        state = IDLE;
      }
      break;

    default:
      state = IDLE;  // Default case if an invalid state occurs
      break;
  }
  
}
void usart_rx(){
  
  switch(state):
    case (IDLE):  
      data=digitalread(dataPin)
      if data ==0:
        state= start;
      break;
	case (start):
      //sample 4 more times
      //sample_data(index=1)
      if sample && mask == 0:
          state = DATA
       break;
    case DATA:
        //sample 5 times
        //sample_data()
        rec_bit = sample && mask
        rx_data|= rec_bit>>counter;
        counter++
        if counter == 7:
            state = PARITY
      case PARITY:
          //sample 5 times
          parityBit=sample & mask
          //check parity func
          state = STOP
      case STOP:
          //sample 5 times
          ENDBit = sample & mask
          if ENDbit=1://or 0
              state=IDLE
    }
		
		
		
		
	}
	
	
    ref_time = millis();


}
//Rx function

//if it is time to read
	//enter data to buffer
	//if counter==5 
	//check switch case on buffer 

void loop() {
	
	uart_rx();
	uart_tx();
 
 
}





