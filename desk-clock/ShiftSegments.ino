/*
 Code to control two UA5651-11EWRS 7-segment displays using SPI communication
 and two TPIC6B595 shift registers. Written for the Arduino MKR Zero originally.
 */

// inslude the SPI library:
#include <SPI.h>

// digital pins for latching the shift registers
const int registerClock = 13;
const int shiftRegisterClear = 14;

/*
 * these int values correspond to the bits that are set when
 * 0-9 are shown on the UA5651-11EWRS 7-segment displays. The
 * LSB corresponds to the A segment, and the MSB corresponds
 * to the decimal point segment (8 segments really).
 */
const int segmentNums[] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 103};

void setup() {
  // setup pins for controlling shift register
  pinMode (registerClock, OUTPUT);
  pinMode (shiftRegisterClear, OUTPUT);
  digitalWrite(registerClock, LOW);
  digitalWrite(shiftRegisterClear, HIGH);

  // initialize SPI:
  SPI.begin();
}

void loop() {
  // cycle through 0-99 repeatedly
  for(int num = 0; num < 100; num++){
    delay(200);
    shiftRegisterWrite(num);
  }
}

void shiftRegisterWrite(int num) {
  // send the updated shift register data via SPI
  int registerContent = segmentNums[num % 10] << 8;
  registerContent += segmentNums[num / 10];
  SPI.transfer16(registerContent);

  // pulse the register clock to show updated value
  digitalWrite(registerClock, HIGH);
  delay(50);
  digitalWrite(registerClock, LOW);
}

