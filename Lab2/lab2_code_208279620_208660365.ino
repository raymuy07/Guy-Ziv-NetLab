// C++ code
//

#define Tx_Data_Line 5
#define Rx_Data_Line 4
#define BIT_wait_time 20

//times
long current_time;
long ref_time;
long random_wait_time;
long delta_time;

int samp_time = 3;
int IDLE=0;
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


bool are_we_starting(){
	
	
	
	
}

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

	
void usart_rx(){
  switch(state):
    case IDLE:  
      data=digitalread(dataPin)
      if data ==0:
        state= start;
      break;
   case start:
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





