#include <stdint.h>
#include "charom.h"
#include "basic.h"
#include "kernal.h"

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
                  | [ ]N/C                    SCK/13[ ] |   B5 --- ROM O8
                  | [ ]IOREF                 MISO/12[ ] |   B4 --- ROM O7
                  | [ ]RST                   MOSI/11[ ]~|   B3 --- ROM A11
                  | [ ]3V3    +---+               10[ ]~|   B2 --- ROM A10
                  | [ ]5v    -| A |-               9[ ]~|   B1 --- ROM A9
                  | [ ]GND   -| R |-               8[ ] |   B0 --- ROM A8
                  | [ ]GND   -| D |-                    |
                  | [ ]Vin   -| U |-               7[ ] |   D7 --- ROM A7
                  |          -| I |-               6[ ]~|   .
   ROM O1 --- C0  | [ ]A0    -| N |-               5[ ]~|   .
   ROM O2 --- C1  | [ ]A1    -| O |-               4[ ] |   .
   ROM O3 --- C2  | [ ]A2     +---+           INT1/3[ ]~|   .
   ROM O4 --- C3  | [ ]A3                     INT0/2[ ] |   .
   ROM O5 --- C4  | [ ]A4/SDA  RST SCK MISO     TX>1[ ] |   .
   ROM O6 --- C5  | [ ]A5/SCL  [ ] [ ] [ ]      RX<0[ ] |   D0 --- ROM A0
                  |            [ ] [ ] [ ]              |
                  |  UNO_R3    GND MOSI 5V  ____________/
                  \_______________________/
      
                  http://busyducks.com/ascii-art-arduinos

  Remaining ROM pins:

  - pin 24 (Vcc) -> +5v
  - pin 20 (/CE) -> GND
  - pin 12 (GND) -> GND

  For 2332 ROM (CHAROM):

  - pin 21 (CE2) -> +5v

  For 2364 ROMS (BASIC, KERNAL):

  - pin 21 (A12) -> 0v/5v to test low/high halves.


  How to diagnose:

  1. Test electrical condition of the chip **FIRST**:

  - Check for shorted Vcc
  - Inputs Ax and /CE must be high-Z. If shorted to 0v/5v, it's broken.
  - Outputs Dx when chip is enabled are push-pull. If an output is high-Z, it's broken.
  - Outputs Dx when chip disabled must be high-Z.

  2. Run this test:

  Due to the lack of pins, the A12 pin must be switched high/low manually. See A12 #define below.

  - Random activity on LED (pin 13)  -> TEST PASS
  - LED is stuck either on or off    -> TEST FAIL

  My CHAROM was broken with all of its outputs shorted to GND regardless of /CE.
  This is a very unfortunate failure because it can chain-fry other chips when they legitimatelly
  own the bus.

 */


// -------------- BEGIN CONFIG AREA -----------------

// UNCOMMENT ROM TO TEST (ONLY ONE AT A TIME!!):
#define ROM characters_901225_01_bin
// #define ROM kernal_901227_03_bin
// #define ROM basic_901226_01_bin

// 8 KB ROMs must have each half tested separately pulling A12 (ROM pin 21) low or high externally
#define A12 0   // 0 to test low half (pull A12 low), 1 to test high half (pull A12 high)

// --------------- END CONFIG AREA ------------------


#define ROM_SIZE sizeof(ROM)

void setup(); // WTF arduino?
void loop();

uint16_t addr = 0;

inline uint8_t
peek(uint16_t a) 
{
  PORTD = a & 0b11111111;
  PORTB = (PORTB & 0b11110000) | ((a >> 8) & 0b00001111); 

  asm("nop"); // 62.5 ns * 7 = 437.5 ns (2332 chip spec is 450 ns)
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");
  asm("nop");

  return ((PINC & 0b00111111) | ((PINB & 0b00110000) << 2));
}


void setup() {
  // 0 = in, 1 = out
  DDRD    = 0b11111111;     DDRC    = 0b11000000;     DDRB    = 0b11001111;
  // PD7  (7) 1------- A7
  // PD6  (6) -1------ A6
  // PD5  (5) --1----- A5   // PC5 (13) --0----- O6   // PB5 (13) --0----- O8
  // PD4  (4) ---1---- A4   // PC4 (12) ---0---- O5   // PB4 (12) ---0---- O7
  // PD3  (3) ----1--- A3   // PC3 (11) ----0--- O4   // PB3 (11) ----1--- A11
  // PD2  (2) -----1-- A2   // PC2 (10) -----0-- O3   // PB2 (10) -----1-- A10
  // PD1  (1) ------1- A1   // PC1 ( 9) ------0- O2   // PB1 ( 9) ------1- A9
  // PD0  (0) -------1 A0   // PC0 ( 8) -------0 O1   // PB0 ( 8) -------1 A8

  // if DDRD == 0 (in)  -> 0 = no pull-up, 1 = pull-up
  // if DDRD == 1 (out) -> 0 = low output, 1 = high output
  PORTD   = 0b00000000;     PORTB   = 0b00000000;     PORTC   = 0b00000000;
  // PD7  (7) 0-------; A7
  // PD6  (6) -0------; A6
  // PD5  (5) --0-----; A5  // PB5 (13) --0----- O8   // PC5 (13) --0----- O6
  // PD4  (4) ---0----; A4  // PB4 (12) ---0---- O7   // PC4 (12) ---0---- O5
  // PD3  (3) ----0---; A3  // PB3 (11) ----0--- A11  // PC3 (11) ----0--- O4
  // PD2  (2) -----0--; A2  // PB2 (10) -----0-- A10  // PC2 (10) -----0-- O3
  // PD1  (1) ------0-; A1  // PB1 ( 9) ------0- A9   // PC1 ( 9) ------0- O2
  // PD0  (0) -------0; A0  // PB0 ( 8) -------0 A8   // PC0 ( 8) -------0 O1

  for (;;) {
    uint8_t test_byte, rom_byte;

    if (A12 == 0) {
      test_byte = pgm_read_byte_near(ROM + addr); // test low half
    } else {
      test_byte = pgm_read_byte_near(ROM + 4096 + addr); // test high half
    }
    
    rom_byte = peek(addr);

    if (test_byte != rom_byte) break;

    addr = (addr + 1) % 4096;
    if (addr == 0) { delay(100); }
  }
}


void loop() {
  // FAIL, just stay here forever
  // We can't signal failure with the LED because it's used as O6.
  // TODO: use pin 13 as an output, DOH!!!
}
