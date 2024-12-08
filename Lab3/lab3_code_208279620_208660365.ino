#define TX_PIN 5                 // Transmission pin
#define RX_PIN 4                 // Reception pin
#define BIT_WAIT_TIME 100000      // Bit duration in microseconds (50 bps)
#define NUMBER_OF_SAMPLES 3      // Number of samples per bit
#define DELTA_TIME (BIT_WAIT_TIME / (NUMBER_OF_SAMPLES + 2)) 
#define HAM_TX_mask 0b1111		


// States
#define IDLE 0
#define START 1
#define DATA 2
#define PARITY 3
#define STOP 4
#define HAMMING 0
#define CRC 1




// Global variables for usart_rx
unsigned long rx_last_time = 0;    // Tracks last sampling time
int rx_state = IDLE;               // Current state of the receiver
int rx_bit_counter = 0;            // Counter for received data bits
char rx_frame = 0;                 // Stores the received frame
int calculated_parity = 1;         // For parity calculation in receiver
int rx_done_flag = 0;			   // For hamming_rx, letting it know to start

// Global variables for usart_tx
int  LAYER_MODE =HAMMING;
int data_length = 0;
char string_data[16]= "Leiba & Zaidman";
unsigned long tx_last_time = 0;    // Tracks last transmission time
int tx_state = IDLE;               // Current state of the transmitter
char tx_data = 0;                  // Data to transmit (ASCII 'a')
int tx_bit_counter = 0;            // Counter for transmitted bits
unsigned long random_wait_time = 1000000; // Initial random wait time in microseconds
int parity_bit = 1;                // Parity bit for transmission
//Global variables for layer2_rx
int MLB_flag = 1;
int decripted_word=0;
char rx_data_string[16] = "";
//Global variables for layer2_tx


void setup() {
  
  //setting up the input and output pins and also setting the output to 1.
  
  
  pinMode(TX_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT);
  digitalWrite(TX_PIN, HIGH);      // Set line HIGH (idle state)
  Serial.begin(19200);
  randomSeed(analogRead(0));       // makes it truly random
  
  
  if (LAYER_MODE == HAMMING){  //calc the right data_length
  
	data_length=8;	
	
	}else{
	  data_length=12;
	}						



}






void uart_tx() {
	
  unsigned long current_time = micros();

  // Random wait time between frames
  if (tx_state == IDLE && current_time - tx_last_time >= random_wait_time) {
    
	tx_bit_counter = 0;
    tx_state = START;
    random_wait_time = random(1000000, 5000000); // Random delay: 1 to 5 seconds

    
    parity_bit = 1; 
    
	
  }

  if (tx_state > 0 && current_time - tx_last_time >= BIT_WAIT_TIME) {
    switch (tx_state) {
      case START:
        digitalWrite(TX_PIN, LOW); // Start bit
        tx_state = DATA;
        break;

      case DATA:
        digitalWrite(TX_PIN, (tx_data >> tx_bit_counter) & 1); // Send data bits
      	//Serial.print(" starting Parity bit: ");
	    //Serial.println(parity_bit);
		parity_bit ^= ((tx_data >> tx_bit_counter) & 1); //calculate parity_bit
        /*Serial.print(" BIT ");
	    Serial.println((tx_data >> tx_bit_counter) & 1);
        Serial.print(" Parity ");
	    Serial.println(parity_bit);
		Serial.print(" tx_bit_counter ");
	    Serial.println(tx_bit_counter);*/     
        tx_bit_counter++;
      	
        if (tx_bit_counter >= data_length) {
			
		  //Serial.print("Parity ");
	      //Serial.println(parity_bit);
		  tx_bit_counter =0;
          tx_state = PARITY;
        }
        break;

      case PARITY:
        digitalWrite(TX_PIN, parity_bit); // send parity bit
        
         /*Serial.print("Parity bit sent: ");
         Serial.println(parity_bit);*/
        tx_state = STOP;
        break;

      case STOP:
        digitalWrite(TX_PIN, HIGH); // send stop bit
        tx_state = IDLE;
        break;
    }
    tx_last_time = current_time;
  }
}


void uart_rx() {
	
  unsigned long current_time = micros();
  static int sample_counter = 0;
  static int sampled_value = 0;
  int bit = 0;

  if (rx_state == IDLE) {
    
	if (digitalRead(RX_PIN) == LOW) {  // see if there is start bit
      rx_state = START;
      rx_last_time = current_time;
      sample_counter = 0;
      sampled_value = 0;
      calculated_parity = 1; // starting from 1 same as in tx
      rx_bit_counter = 0;
      rx_frame = 0;
      
    }
  } else if (current_time - rx_last_time >= DELTA_TIME) {
	  
    sample_counter++;
    sampled_value = (sampled_value << 1) | digitalRead(RX_PIN);

    if (sample_counter == 5) {  // after sampling 5 times
      
	  int middle_bits = (sampled_value & 0b1110);
      if (middle_bits == 0b0000){
       bit = 0; 
        
      }else if (middle_bits == 0b1110){
        bit = 1; 

      }else {
		  
		rx_state = IDLE;  // reset due to false bit
		Serial.println("bad bit detected");
			
	  }
      sample_counter = 0; // Reset for the next bit
      sampled_value = 0;

      switch (rx_state) {
		  
        case START:
          if (bit == 0) {  // start bit
            rx_state = DATA;
            
          } else {
            rx_state = IDLE;  // Reset on invalid start bit
            Serial.println("false start detected");
          }
          break;

        case DATA:
          rx_frame |= (bit << rx_bit_counter); // store the received bit
          calculated_parity ^= bit; // Update calculated parity
          rx_bit_counter++;
          if (rx_bit_counter >= data_length) {
            rx_state = PARITY;
            //Serial.println(" RX_frame: ");
            //Serial.println(rx_frame,BIN);
          }
          break;

        case PARITY:
          if (bit == calculated_parity) {
            
			//Serial.println("Parity OK");
            rx_state = STOP;
          } else {
            Serial.println("Parity error detected");
            Serial.println("Parity detected: ");
            Serial.println(bit);
            Serial.println("Parity caculated: ");
            Serial.println(calculated_parity);
            rx_state = IDLE; // reset on parity error
          }
          break;

        case STOP:
          if (bit == 1) {  // validate stop bit
            //Serial.print("Received Frame: ");
            //Serial.println(rx_frame, BIN);
            //Serial.print("Received Character: ");
            //Serial.println((char)rx_frame);
          } else {
            Serial.println("Stop bit error detected");
          }
          rx_state = IDLE;  // reset after processing frame
          rx_done_flag = 1;
          break;
      }
    }
    rx_last_time = current_time;
  }
}

void layer2_tx(){

	switch(LAYER_MODE){
		
		case HAMMING:
		Hamming47_tx();
		break;
		
		case CRC:
		CRC4_tx();
		break;

	   }
}

void layer2_rx(){
	
	switch(LAYER_MODE){
		
		case HAMMING:
		Hamming47_rx();
		break;
		
		case CRC:
		CRC4_rx();
		break;

	   }

}


void Hamming47_tx(){
  	int current_time=micros();
	int string_length = sizeof(string_data);
	static int HAM_tx_counter=0;
	static int IDLE_HAM_counter=0;
	static int current_char=0;  
  	static int current_4bits=0; 
	if (tx_state==IDLE && current_time-tx_last_time>=BIT_WAIT_TIME){
		current_char = string_data[HAM_tx_counter];
      	//Serial.print(" HAM_tx_counter ");
  		//Serial.print(HAM_tx_counter);
		int MSB_char=current_char>>4;
      	int LSB_char=current_char;
      	if (IDLE_HAM_counter==0){
			IDLE_HAM_counter=1;
          	current_char=MSB_char;
		}else{
			current_char=LSB_char;
			HAM_tx_counter++;
			IDLE_HAM_counter=0;
		}
		current_4bits = current_char&HAM_TX_mask;
		tx_data= create_hamming_word(current_4bits);
      	parity_bit=1;
		tx_state = START;
      	//Serial.println(" HAM_tx_counter: ");
  		//Serial.println(HAM_tx_counter);
		if (HAM_tx_counter==string_length){
			HAM_tx_counter=0;
		}
      	//Serial.println(" tx_data: ");
  		//Serial.println(tx_data,BIN);
		
	}
	
	
}
int create_hamming_word(int HAM_data){
	
	int D1 = HAM_data&0b1;
	int D2 = (HAM_data&0b10)>>1;
	int D3 = (HAM_data&0b100)>>2;
	int D4 = (HAM_data&0b1000)>>3;
	int P1 = D1^D2^D4;
	int P2 = D1^D3^D4;
	int P3 = D2^D3^D4;
	int word = 0;
	bitWrite (word,0,P1);
	bitWrite (word,1,P2);
	bitWrite (word,2,D1);
	bitWrite (word,3,P3);
	bitWrite (word,4,D2);
	bitWrite (word,5,D3);
	bitWrite (word,6,D4);
	/*Serial.print(" HAM_data: ");
	Serial.print(HAM_data,BIN);
	Serial.print(" P1: ");
  	Serial.print(P1);
	Serial.print(" P2: ");
  	Serial.print(P2);
	Serial.print(" P3: ");
  	Serial.print(P3);
	Serial.println(" word: ");
  	Serial.println(word,BIN);*/
	return word;
}

void Hamming47_rx(){
  if(rx_done_flag){
	int current_4bits = 0;
	int coded_word=rx_frame;
	int eror_detected=hamming_eror_detection(coded_word);
	if (eror_detected==0){
		bitWrite (current_4bits,0,(coded_word&0b100)>>2);
		bitWrite (current_4bits,1,(coded_word&0b10000)>>4);
		bitWrite (current_4bits,2,(coded_word&0b100000)>>5);
		bitWrite (current_4bits,3,(coded_word&0b1000000)>>6); 
		decripted_word |= current_4bits;
      	//Serial.println(" decripted_word bits: ");
      	//Serial.println(decripted_word,BIN);
		if (MLB_flag==1){
			decripted_word = decripted_word<<4;
			MLB_flag=0;
			//Serial.println(" saved 4MLB bits: ");
			//Serial.println(decripted_word,BIN);
		}
		else {
			MLB_flag=1;
			Serial.println(" char detected, bin: ");
			Serial.println(decripted_word,BIN);
			Serial.println(" char detected: ");
			Serial.println((char) decripted_word);
			int len = strlen(rx_data_string);
			rx_data_string[len] = decripted_word;
			rx_data_string[len+1] = '\0'; // add null
			Serial.println(" rx_data_string: ");
			Serial.println(rx_data_string);
			decripted_word=0;
		}
	}
	else {
		Serial.println(" Eror detected ");
	}
  }
  rx_done_flag=0;
	
	
	
}


int hamming_eror_detection(int word){
	int P1 = word&0b1;
	int P2 = (word&0b10)>>1;
	int D1 = (word&0b100)>>2;
	int P3 = (word&0b1000)>>3;
	int D2 = (word&0b10000)>>4;
	int D3 = (word&0b100000)>>5;
	int D4 = (word&0b1000000)>>6;
	int P1_test = D1^D2^D4;
	int P2_test = D1^D3^D4;
	int P3_test = D2^D3^D4;
	if((P1!=P1_test)||(P2!=P2_test)||(P3!=P3_test)){
		return 1;
	}
	else{
		return 0;
	}
	
	
	
}



void CRC4_rx(){
	
	
	
}


void CRC4_tx(){
	
	
	
}





void loop() {
  
  layer2_tx();
  uart_tx();
  uart_rx();
  layer2_rx();
  
  
  

}




