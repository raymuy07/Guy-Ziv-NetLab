#include "EthernetLab.h"

#define WINDOW_SIZE 3 // Define the window size
#define GAP_FACTOR 5  // To calculate 1/5 RTT

uint8_t frame_type = 0x00;
uint8_t source_address = 0x1A;
uint8_t destination_address = 0x0A;
unsigned long RTT = 0;
unsigned long timer_start = 0;
uint8_t base = 0;              // Sequence number of the oldest unacknowledged frame
uint8_t next_seq_num = 0;      // Sequence number of the next frame to send
char dataset[4][16] = {"Leiba & Zaidman", "Zaidman & Leiba", "Guy & Ziv1234567", "Ziv & Guy1234567"};
char payload_data[16] = {0};
uint8_t payload_length = 17;
uint8_t send_buffer[25];       // Buffer for sending frames
uint8_t ack_buffer[25];
unsigned long zero_time = 0;
float total_frames_counter = 0;
float bad_frames_counter = 0;

void setup() {
    zero_time = millis();
    setAddress(TX, 10);
    strncpy(payload_data, dataset[0], payload_length - 1);
    payload_data[payload_length - 1] = '\0'; // Ensure null-termination
    Serial.begin(115200);
    build_packet(base);
    timer_start = millis();
}

void build_packet(uint8_t seq_num) {
    memset(send_buffer, 0, sizeof(send_buffer));
    send_buffer[0] = destination_address;
    send_buffer[1] = source_address;
    send_buffer[2] = frame_type;
    send_buffer[3] = payload_length;
    send_buffer[4] = (payload_length << 1) | (seq_num & 0x01);
    memcpy(&send_buffer[5], payload_data, payload_length - 1);
    unsigned long crc = calculateCRC(send_buffer, 21);
    memcpy(&send_buffer[5 + payload_length - 1], &crc, 4);
}

void send_window_frames() {
    while (next_seq_num < base + WINDOW_SIZE && next_seq_num < 4) {
        build_packet(next_seq_num);
        sendPackage(send_buffer, sizeof(send_buffer));
        delay(RTT / GAP_FACTOR); // Add 1/5 RTT gap between frames
        next_seq_num++;
    }
}

void is_ack() {
    bool received_ack = readPackage(ack_buffer, sizeof(ack_buffer));
    if (received_ack) {
        uint8_t ack_sn = ack_buffer[4] & 0x01;
        if (ack_sn >= base) {
            base = ack_sn + 1; // Slide the window
            timer_start = millis(); // Reset the timer for the oldest frame
        }
    }
}

void is_time_out() {
    if (millis() - timer_start >= time_out) {
        Serial.println("Timeout! Resending all frames in the window...");
        for (uint8_t i = base; i < next_seq_num; i++) {
            build_packet(i);
            sendPackage(send_buffer, sizeof(send_buffer));
            delay(RTT / GAP_FACTOR); // Add gap between retransmissions
        }
        timer_start = millis(); // Reset timer
    }
}

void loop() {
    is_time_out();
    is_ack();
    send_window_frames();
}
