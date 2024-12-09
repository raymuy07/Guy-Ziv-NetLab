#include "EthernetLab.h"
void setup() {
  // put your setup code here, to run once:
  setAddress (TX,10);

}

void loop() {
  // put your main code here, to run repeatedly:
  testTX();
}
