// C++ code
//

#define BIT_wait_time 1000
#define BIT_half_wait_time 500

#define CLOCK 0
#define Tx_Data_Line 5
#define Rx_Data_Line 4
#define RX_Clock_Line 10
#define TX_Clock_Line 11



long current_time;
long ref_time; 
char data = 'a';

int current_bit , counter_rx, counter_tx;



void setup()
{

  pinMode(Tx_Data_Line, OUTPUT); 
  pinMode(Rx_Data_Line, INPUT); 
  pinMode(TX_Clock_Line, OUTPUT); 
  pinMode(RX_Clock_Line, INPUT); 

  counter_tx=0;
  counter_rx=0;
  ref_time = millis();
}


void usart_tx(){
  
  current_time = millis();

  //Sending clock
  if(current_time-ref_time >= BIT_half_wait_time){
  
    switch(CLOCK){
      
	  case(0):
	  digitalWrite(TX_Clock_Line, HIGH);
      current_bit = bitRead(data , counter);
        	if (current_bit){
         	 	digitalWrite(Tx_Data_Line, HIGH);
        	}
        	else{
           		digitalWrite(Tx_Data_Line, LOW);
        
      counter ++;
      ref_time = millis();
		break;
      
      case(1):
      digitalWrite(TX_Clock_Line, LOW);
        	if (counter_tx == 8){
        		counter_tx = 0;
        		IDLE=1;
            }
        	ref_time = millis();
		break;
  }
}
  
}


void usart_rx(){
  
  if (counter_rx == 8){
    counter_rx = 0;
  }
  
  bit_read = digitalRead(Rx_Data_Line);   // read the input pin
  Serial.print(bit_read);
  counter_rx ++ ;

}

void loop() {
  
  
  long current_time = millis();
  
  rx_clock_last = rx_clock_current;
  rx_clock_current = digitalRead(RX_Clock_Line);
  
  //indication that clock is going down
  if (rx_clock_last) && (!rx_clock_current){
    
    usart_rx(); //read data
  }
  
  
  

}

