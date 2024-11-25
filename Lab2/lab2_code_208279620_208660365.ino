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




void uart_rx(){
	
	current_time = millis();

	if(current_time-ref_time >= delta_time){
		
		
		
		
	}
	
	
    ref_time = millis();


}
//Rx function

//if it is time to read
	//enter data to buffer
	//if counter==5 
	//check switch case on buffer 

void loop() {
 
 
}
