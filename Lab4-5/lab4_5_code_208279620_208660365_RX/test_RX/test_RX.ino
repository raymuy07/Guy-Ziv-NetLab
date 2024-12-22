#include "EthernetLab.h"

uint8_t receive_buffer[25];  // Buffer for received frames
char payload_data[16];       // Buffer for payload data
unsigned long received_crc;  // CRC from the received frame
unsigned long calculated_crc;// Calculated CRC
uint8_t send_buffer[25];     // Buffer for ACK
uint8_t ack_Sn = 0;          // Sequence number for ACK
uint8_t destination_address = 0x1A;
uint8_t source_address = 0x0A;
uint8_t frame_type = 0;
uint8_t payload_length = 17;

void setup() {
    setAddress(RX, 10);
    Serial.begin(115200);
}

void extract_raw_data(uint8_t *raw_data) {
    uint8_t seq_num = raw_data[4] & 0x01;
    memcpy(payload_data, &raw_data[5], payload_length - 1);
    payload_data[payload_length - 1] = '\0'; // Null-terminate the payload
    memcpy(&received_crc, &raw_data[4 + payload_length], 4);
    calculated_crc = calculateCRC(raw_data, 21);

    if (calculated_crc == received_crc && seq_num == ack_Sn) {
        ack_Sn = (seq_num + 1) % 2; // Increment ACK sequence number
        Serial.print("Received: ");
        Serial.println(payload_data);
    } else {
        Serial.println("CRC or Sequence Error");
    }
}

void build_ack_packet() {
    memset(send_buffer, 0, sizeof(send_buffer));
    send_buffer[0] = destination_address;
    send_buffer[1] = source_address;
    send_buffer[2] = frame_type;
    send_buffer[3] = payload_length;
    send_buffer[4] = ack_Sn;
    unsigned long crc = calculateCRC(send_buffer, 21);
    memcpy(&send_buffer[4 + payload_length], &crc, 4);
}

void loop() {
    if (readPackage(receive_buffer, sizeof(receive_buffer))) {
        extract_raw_data(receive_buffer);
        build_ack_packet();
        sendPackage(send_buffer, sizeof(send_buffer));
    }
}
