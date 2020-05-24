#include <stdint.h>
#include "pla.h"

/*
  Wiring diagram:
                                               +-----+
                  +----[PWR]-------------------| USB |--+
                  |                            +-----+  |
                  |         GND/RST2  [ ][ ]            |
                  |       MOSI2/SCK2  [ ][ ]  A5/SCL[ ] |   C5 
                  |          5V/MISO2 [ ][ ]  A4/SDA[ ] |   C4 
                  |                             AREF[ ] |
                  |                              GND[ ] |
                  | [ ]N/C                    SCK/13[ ] |   B5 --- PLA I13
                  | [ ]IOREF                 MISO/12[ ] |   B4 --- PLA I12
                  | [ ]RST                   MOSI/11[ ]~|   B3 --- PLA I11
                  | [ ]3V3    +---+               10[ ]~|   B2 --- PLA I10
                  | [ ]5v    -| A |-               9[ ]~|   B1 --- PLA I9
                  | [ ]GND   -| R |-               8[ ] |   B0 --- PLA I8
                  | [ ]GND   -| D |-                    |
                  | [ ]Vin   -| U |-               7[ ] |   D7 --- PLA I7
                  |          -| I |-               6[ ]~|   .
  PLA I14 --- C0  | [ ]A0    -| N |-               5[ ]~|   .
  PLA I15 --- C1  | [ ]A1    -| O |-               4[ ] |   .
 PLA F0/4 --- C2  | [ ]A2     +---+           INT1/3[ ]~|   .
 PLA F1/5 --- C3  | [ ]A3                     INT0/2[ ] |   .
 PLA F2/6 --- C4  | [ ]A4/SDA  RST SCK MISO     TX>1[ ] |   .
 PLA F3/7 --- C5  | [ ]A5/SCL  [ ] [ ] [ ]      RX<0[ ] |   D0 --- PLA I0
                  |            [ ] [ ] [ ]              |
                  |  UNO_R3    GND MOSI 5V  ____________/
                  \_______________________/
      
                  http://busyducks.com/ascii-art-arduinos
 
  Remaining PLA pins:

  - pin 28 (Vcc) -> +5v
  - pin 19 (/CE) -> GND
  - pin 14 (Vss) -> GND

  How to diagnose:

  1. Test electrical condition of the chip **FIRST**:

  - Check for shorted Vcc
  - Inputs Ix and /CE must be high-Z. If shorted to 0v/5v, it's broken.
  - PLA outputs Fx when chip is enabled are push-pull. If an output is high-Z, it's broken.
  - PLA outputs Fx when chip disabled must be high-Z.

  2. Run this test:

  Due to the lack of pins, output nybbles must be tested separately, use #define below.

  - Random activity on LED (pin 13)  -> TEST PASS
  - A series of n LED flashes        -> TEST FAIL output F(n-1)

  My particular PLA had these problems:

  - One output (F0) was in high impedance
  - None of the remaining outputs matched the expected combinational logic (= fried input/s?)

  Uncomment the FREE_RUN define to run uninterrupted if you want to peek at the signals with a scope.
 */


// -------------- BEGIN CONFIG AREA -----------------

// Due to the lack of pins, output nybbles must be tested separately
#define TEST_NIBBLE 0   // 0 to test low nibble (outputs F0-3), 1 to test high nibble (outputs F4-7)

// Uncomment to run without interruptions for visual inspection with a scope
//#define FREE_RUN

// --------------- END CONFIG AREA ------------------


void setup(); // WTF arduino?
void loop();


inline uint8_t
peek(uint16_t a) 
{
  PORTD = a & 0b11111111;
  PORTB = (a >> 8) & 0b00111111;
  PORTC = (PORTC & 0b11111100) | ((a >> 14) & 0b00000011);

  asm("nop"); // 62.5 ns
  asm("nop"); // 62.5 ns

  return ((PINC & 0b00111100) >> 2);
}

void
err(int e) {
  // FAIL, notify error in output Fi with i+1 led blinks
  digitalWrite(13, LOW);
  delay(1000);
  for (int i = 0; i < e; i++) {
    digitalWrite(13, HIGH);
    delay(200);
    digitalWrite(13, LOW);
    delay(100);
  }
  delay(1000);
}

void
free_run() {
  addr = 0;
  do {
    peek(addr);
    addr++;
  } while (addr);
}

void
seq_test() {
  uint8_t test_nibble, pla_nibble;

  do {
    pla_nibble = peek(addr);
    if (TEST_NIBBLE == 0) {
      if (!!(F0) != !!(pla_nibble & 1)) { err(1); break; }
    } else {
      if (!!(F4) != !!(pla_nibble & 1)) { err(5); break; }
    }
    addr++;
  } while (addr);

  do {
    pla_nibble = peek(addr);
    if (TEST_NIBBLE == 0) {
      if (!!(F1) != !!(pla_nibble & 2)) { err(2); break; }
    } else {
      if (!!(F5) != !!(pla_nibble & 2)) { err(6); break; }
    }
    addr++;
  } while (addr);

  do {
    pla_nibble = peek(addr);
    if (TEST_NIBBLE == 0) {
      if (!!(F2) != !!(pla_nibble & 4)) { err(3); break; }
    } else {
      if (!!(F6) != !!(pla_nibble & 4)) { err(7); break; }
    }
    addr++;
  } while (addr);

  do {
    pla_nibble = peek(addr);
    if (TEST_NIBBLE == 0) {
      if (!!(F3) != !!(pla_nibble & 8)) { err(4); break; }
    } else {
      if (!!(F7) != !!(pla_nibble & 8)) { err(8); break; }
    }
    addr++;
  } while (addr);
}

void setup() {
  // 0 = in, 1 = out
  DDRD    = 0b11111111;     DDRC    = 0b11000011;     DDRB    = 0b11111111;
  // PD7  (7) 1------- I7
  // PD6  (6) -1------ I6
  // PD5  (5) --1----- I5   // PC5 (13) --0----- F3/7 // PB5 (13) --1----- I13
  // PD4  (4) ---1---- I4   // PC4 (12) ---0---- F2/6 // PB4 (12) ---1---- I12
  // PD3  (3) ----1--- I3   // PC3 (11) ----0--- F1/5 // PB3 (11) ----1--- I11
  // PD2  (2) -----1-- I2   // PC2 (10) -----0-- F0/4 // PB2 (10) -----1-- I10
  // PD1  (1) ------1- I1   // PC1 ( 9) ------1- I15  // PB1 ( 9) ------1- I9
  // PD0  (0) -------1 I0   // PC0 ( 8) -------1 I14  // PB0 ( 8) -------1 I8

  // if DDRD == 0 (in)  -> 0 = no pull-up, 1 = pull-up
  // if DDRD == 1 (out) -> 0 = low output, 1 = high output
  PORTD   = 0b00000000;     PORTB   = 0b00000000;     PORTC   = 0b00000000;
  // PD7  (7) 0-------; I7
  // PD6  (6) -0------; I6
  // PD5  (5) --0-----; I5  // PB5 (13) --0----- F3/7 // PC5 (13) --0----- I13
  // PD4  (4) ---0----; I4  // PB4 (12) ---0---- F2/6 // PC4 (12) ---0---- I12
  // PD3  (3) ----0---; I3  // PB3 (11) ----0--- F1/5 // PC3 (11) ----0--- I11
  // PD2  (2) -----0--; I2  // PB2 (10) -----0-- F0/4 // PC2 (10) -----0-- I10
  // PD1  (1) ------0-; I1  // PB1 ( 9) ------0- I15  // PC1 ( 9) ------0- I9
  // PD0  (0) -------0; I0  // PB0 ( 8) -------0 I14  // PC0 ( 8) -------0 I8

  for (;;) {
#ifdef FREE_RUN
    free_run();  // for visual test with scope
#else
    seq_test();
#endif
    delay(10);
  }
}


void loop() {
}
