// C++ code
//

#define BIT_wait_time 20
#define BIT_half_wait_time 10

#define Tx_Data_Line 5
#define Rx_Data_Line 4
#define RX_Clock_Line 10
#define TX_Clock_Line 11


long current_time;
long ref_time;
long random_wait_time;
char data = 'a';
int rx_data = 0;
int IDLE=0;
int tx_clock=0;
int rx_clock_last , rx_clock_current=0 ;
int bit_read, current_bit , rx_counter, counter_tx;



void setup()
{
  //for debug
  Serial.begin(9600); 
  // opens serial port, sets data rate to 9600 bps  //end debug
  
  pinMode(Tx_Data_Line, OUTPUT); 
  pinMode(Rx_Data_Line, INPUT); 
  pinMode(TX_Clock_Line, OUTPUT); 
  pinMode(RX_Clock_Line, INPUT); 

  random_wait_time = random(1000,7000);
  bit_read = 0;
  counter_tx=0;
  rx_counter=0;
  ref_time = millis();
}


void usart_tx(){
  
  current_time = millis();
  if (IDLE==1){
    
    //random wait time
    if(current_time-ref_time >= random_wait_time){
     IDLE=0; 
     ref_time = millis();
    }
    
  }
  //Sending clock
  else if(current_time-ref_time >= BIT_half_wait_time){
    switch(tx_clock){
      
	  case(0):
	  digitalWrite(TX_Clock_Line, HIGH);
      tx_clock=1;
      current_bit = bitRead(data , counter_tx);

        	if (current_bit){
         	 	digitalWrite(Tx_Data_Line, HIGH);
        	}
        	else{
           		digitalWrite(Tx_Data_Line, LOW);
            }
      counter_tx++;
      ref_time = millis();
	  break;
      
      case(1):
      digitalWrite(TX_Clock_Line, LOW);
      tx_clock=0;
        	if (counter_tx == 8){
        		counter_tx = 0;
        		IDLE=1;
              	random_wait_time = random(1000,7000);
            }
      ref_time = millis();
	  break;
  }
}
  
}


void usart_rx(){

  rx_clock_last = rx_clock_current;
  rx_clock_current = digitalRead(RX_Clock_Line);

  //indication that clock is going down
  if ((rx_clock_last) && !(rx_clock_current)){
    bit_read = digitalRead(Rx_Data_Line);   // read the input pin
    rx_data |= (bit_read<<rx_counter);
    rx_counter++;
    bit_read=0;
    if (rx_counter==8){
      Serial.println((char)rx_data);
      rx_data = 0;
      rx_counter=0;
    }

  }
}

void loop() {
  
  usart_tx();
  usart_rx(); //read data
      
 
}
