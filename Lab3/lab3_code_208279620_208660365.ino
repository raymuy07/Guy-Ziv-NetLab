#define TX_PIN 5                 // Transmission pin
#define RX_PIN 4                 // Reception pin
#define BIT_WAIT_TIME 20000      // Bit duration in microseconds (50 bps)
#define NUMBER_OF_SAMPLES 3      // Number of samples per bit
#define DELTA_TIME (BIT_WAIT_TIME / (NUMBER_OF_SAMPLES + 2)) 

#define CRC_TX_mask 0b10011        
#define DIVISOR_LENGTH 5    //these two comes together

#define HAMMING 0
#define CRC 1

// States
#define IDLE 0
#define START 1
#define DATA 2
#define PARITY 3
#define STOP 4



///need to arrange all of this:

// Global variables for usart_rx
int  LAYER_MODE = CRC;

unsigned long rx_last_time = 0;    // Tracks last sampling time
int rx_state = IDLE;               // Current state of the receiver
int rx_bit_counter = 0;            // Counter for received data bits
uint16_t rx_frame = 0;                 // Stores the received frame
int calculated_parity = 1;         // For parity calculation in receiver

// Global variables for usart_tx
int data_length;
char string_data[16]= "Leiba & Zaidman";

unsigned long tx_last_time = 0;    // Tracks last transmission time
int tx_state = IDLE;               // Current state of the transmitter

uint16_t tx_data;


int tx_bit_counter = 0;            // Counter for transmitted bits
unsigned long random_wait_time = 1000000; // Initial random wait time in microseconds
int parity_bit;                // Parity bit for transmission

void setup() {
  
  //setting up the input and output pins and also setting the output to 1.
  
  
  pinMode(TX_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT);
  digitalWrite(TX_PIN, HIGH);      // Set line HIGH (idle state)
  Serial.begin(19200);
  randomSeed(analogRead(0));       // makes it truly random
  
  
  if (LAYER_MODE == HAMMING){  //calc the right data_length
  
	data_length=7;	
	
	}else{
	  data_length=12;
	}						



}






void uart_tx() {
	
  unsigned long current_time = micros();
 
  if (tx_state > 0 && current_time - tx_last_time >= BIT_WAIT_TIME) {
    switch (tx_state) {
      
	  case START:  
		parity_bit = 1; 
		tx_bit_counter = 0;
        digitalWrite(TX_PIN, LOW); // Start bit
        tx_state = DATA;
        break;

      case DATA:
	 
		 
        digitalWrite(TX_PIN, (tx_data >> tx_bit_counter) & 1); // Send data bits
        
		parity_bit ^= ((tx_data >> tx_bit_counter) & 1); //calculate parity_bit
        tx_bit_counter++;
        if (tx_bit_counter >= data_length) {
			
          tx_state = PARITY;
          
        }
        break;

      case PARITY:
        digitalWrite(TX_PIN, parity_bit); // send parity bit
        
        // Serial.print("Parity bit sent: ");
        // Serial.println(parity_bit);
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
          }
          break;

        case PARITY:
          if (bit == calculated_parity) {
            
            Serial.println(calculated_parity);

			Serial.println("Parity OK");
            rx_state = STOP;
          } else {

            Serial.println("Parity error detected");
            rx_state = IDLE; // reset on parity error
          }
          break;

        case STOP:
          if (bit == 1) {  // validate stop bit
            
            Serial.print("Received Frame: ");
            Serial.println(rx_frame, BIN);
          } else {
            Serial.println("Stop bit error detected");
          }
          rx_state = IDLE;  // reset after processing frame
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
	
	
	
}

void Hamming47_rx(){
	
	
	
}

void CRC4_rx(){

	
  if ((rx_state == IDLE)&&(rx_frame!=0)){
    
    // Separate the data and CRC
	uint8_t received_data = (rx_frame >> 4) & 0xFF; // First 8 bits are data
	uint8_t received_crc = rx_frame & 0xF;          // Last 4 bits are CRC

	// Recalculate CRC on the received data
	uint16_t dividend = received_data << 4; // Append 4 zero bits
	uint16_t remainder = dividend;

	for (int i = 12 - DIVISOR_LENGTH; i >= 0; i--) {
		if (remainder & (1 << (i + DIVISOR_LENGTH - 1))) { // Check MSB
			remainder ^= (CRC_TX_mask << i); // XOR with aligned divisor
		}
	}

    uint8_t calculated_crc = remainder & 0xF; // Extract the last 4 bits
    	
	if (calculated_crc == received_crc) {
            Serial.println("CRC Valid");
        } else {
            Serial.println("CRC Error");
        }
		
  }else{
    
   return; 
  }
	
}


void CRC4_tx(){
	
	unsigned long current_time = micros(); //for the randomtime between frame to frame

	static int crc_counter = 0;    
    
	static uint8_t crc_word;         
	static uint16_t dividend;
	
    static const int dividend_len = 12;  
	
  	if (tx_state == IDLE && current_time - tx_last_time >= random_wait_time){
		
		crc_word = string_data[crc_counter];
		dividend = crc_word <<4; 
		uint16_t remainder = dividend; // start with the full dividend

		for (int i = 12 - DIVISOR_LENGTH; i >= 0; i--) {
			if (remainder & (1 << (i + DIVISOR_LENGTH - 1))) { // Check MSB
				remainder ^= (CRC_TX_mask << i); // XOR with aligned divisor
			}
    }
		
	  tx_data = (dividend | remainder); // Combine data and CRC
	  tx_state = START;
		
		Serial.println("the transmit");
		Serial.println(tx_data,BIN);

	
	
		if (crc_counter < sizeof(string_data)){
				
				crc_counter++;
				random_wait_time = random(1000000, 3000000); // Random delay: 1 to 5 seconds

			}
			
			else{
				crc_counter = 0;
			
				}
	}else{
      
		return;
      
	}

}


void loop() {
  
  layer2_tx();
  uart_tx();
  layer2_rx();
  uart_rx();
  
  

}

