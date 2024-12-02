#define TX_PIN 5                 // Transmission pin
#define RX_PIN 4                 // Reception pin
#define BIT_WAIT_TIME 20000    // Bit duration in microseconds (1 second per bit)
#define DELTA_TIME (BIT_WAIT_TIME / 5) // Sampling granularity

// Receiver states
#define IDLE 0
#define START 1
#define DATA 2
#define STOP 3

// Global variables
unsigned long rx_last_time = 0;  // Tracks last sampling time
int rx_state = IDLE;             // Current state of the receiver
int rx_bit_counter = 0;          // Counter for received data bits
char rx_frame = 0;               // Stores the received frame

unsigned long tx_last_time = 0;  // Tracks last transmission time
int tx_state = 0;                // Current state of the transmitter
char tx_data = 0b01100001;       // Data to transmit (ASCII 'a')
int tx_bit_counter = 0;          // Counter for transmitted bits
unsigned long random_wait_time = 1000000; // Initial random wait time in microseconds

void setup() {
  pinMode(TX_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT);
  digitalWrite(TX_PIN, HIGH);    // Set line HIGH (idle state)
  Serial.begin(19200);
  randomSeed(analogRead(0));     // Seed random generator
  Serial.println("Transceiver Ready");
}

// Transmitter function
void transmit() {
  unsigned long current_time = micros();

  // Random wait time between frames
  if (tx_state == 0 && current_time - tx_last_time >= random_wait_time) {
    tx_bit_counter = 0;
    tx_state = 1;
    random_wait_time = random(1000000, 5000000); // Random delay: 1 to 5 seconds
    //Serial.println("Transmitting...");
  }

  if (tx_state > 0 && current_time - tx_last_time >= BIT_WAIT_TIME) {
    switch (tx_state) {
      case 1:
        digitalWrite(TX_PIN, LOW); // Start bit
        //Serial.println("Start bit sent");
        tx_state = 2;
        break;

      case 2:
        digitalWrite(TX_PIN, (tx_data >> tx_bit_counter) & 1); // Send data bits
        //Serial.print("Data bit sent: ");
        //Serial.println((tx_data >> tx_bit_counter) & 1);
        tx_bit_counter++;
        if (tx_bit_counter >= 8) tx_state = 3;
        break;

      case 3:
        digitalWrite(TX_PIN, HIGH); // Stop bit
        //Serial.println("Stop bit sent");
        tx_state = 0; // Back to idle
        break;
    }
    tx_last_time = current_time;
  }
}

// Receiver function
void receive() {
  unsigned long current_time = micros();
  static int sample_counter = 0;
  static int sampled_value = 0;

  if (rx_state == IDLE) {
    if (digitalRead(RX_PIN) == LOW) {  // Detect start bit
      rx_state = START;
      rx_last_time = current_time;
      sample_counter = 0;
      sampled_value = 0;
      //Serial.println("Start bit detected");
    }
  } else if (current_time - rx_last_time >= DELTA_TIME) {
    sample_counter++;
    sampled_value = (sampled_value << 1) | digitalRead(RX_PIN);

    if (sample_counter == 5) {  // Process after sampling 5 times
      int middle_bits = (sampled_value & 0b1110) >> 1; // Extract middle 3 bits
      int ones_count = (middle_bits & 1) + ((middle_bits >> 1) & 1) + ((middle_bits >> 2) & 1);
      int bit = (ones_count >= 2) ? 1 : 0; // Majority voting

      sample_counter = 0; // Reset for the next bit
      sampled_value = 0;

      switch (rx_state) {
        case START:
          if (bit == 0) {  // Validate start bit
            rx_state = DATA;
            rx_bit_counter = 0;
            rx_frame = 0;
        //    Serial.println("Valid start bit");
          } else {
            rx_state = IDLE;  // Reset on invalid start bit
            Serial.println("False start detected");
          }
          break;

        case DATA:
          rx_frame |= (bit << rx_bit_counter); // Store received bit
          rx_bit_counter++;
          //Serial.print("Received Bit: ");
          //Serial.println(bit);
          if (rx_bit_counter == 8) {  // After 8 bits, move to STOP state
            rx_state = STOP;
          }
          break;

        case STOP:
          if (bit == 1) {  // Validate stop bit
            Serial.print("Received Frame: ");
            Serial.println(rx_frame, BIN);
            Serial.print("Received Character: ");
            Serial.println((char)rx_frame);
          } else {
            Serial.println("Stop bit error detected");
          }
          rx_state = IDLE;  // Reset after processing frame
          break;
      }
    }
    rx_last_time = current_time;
  }
}

void loop() {
  transmit();
  receive();
}

