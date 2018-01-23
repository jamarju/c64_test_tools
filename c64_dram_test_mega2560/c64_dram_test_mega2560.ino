#include <stdint.h>

/*
 64K x 4 bit DRAM test for: Nec D41464C, TMS4464, etc.

 Wiring diagram:
 Robotdyn Mega Pro CH340G /Atmega2560 (Arduino MEGA compatible)

                   +---------------
                   | VIN[ ][ ]VIN  \                  
                   | GND[ ][ ]GND  /                  
                   |  5V[ ][ ]5V   \                  
                   | 3V3[ ][ ]3V3  /                  
                   | RST[ ][ ]AREF \                  
               E1  |  TX[ ][ ]RX   /  E0
               E5  |  D3[ ][ ]D2   \  E4
               E3  |  D5[ ][ ]D4   /  G5
               H4  |  D7[ ][ ]D6   \  H3
               H6  |  D9[ ][ ]D8   /  H5
               B5  | D11[ ][ ]D10  \  B4
               B7  | D13[ ][ ]D12  /  B6
               J0  | D15[ ][ ]D14  \  J1
               H0  | D17[ ][ ]D16  /  H1              
               D2  | D19[ ][ ]D18  \  D3              
               D0  | D21[ ][ ]D20  /  D1              
    RAM A1 --- A1  | D23[ ][ ]D22  \  A0 --- RAM A0
    RAM A3 --- A3  | D25[ ][ ]D24  /  A2 --- RAM A2
    RAM A5 --- A5  | D27[ ][ ]D26  \  A4 --- RAM A5
    RAM A7 --- A7  | D29[ ][ ]D28  /  A6 --- RAM A7
               C6  | D31[ ][ ]D30  \  C7
                   +----------------

                    --+-----+-------+
                    \ |     |       |
                    / | USB | [RST] |
                    \ +-----+       |
                    /               |
                    \               |
    RAM IO2 --- F1  /   A1[ ][ ]A0  |  F0 --- RAM IO1
    RAM IO4 --- F3  \   A3[ ][ ]A2  |  F2 --- RAM IO3
                F5  /   A5[ ][ ]A4  |  F4
                F7  \   A7[ ][ ]A6  |  F6
                K1  /   A9[ ][ ]A8  |  K0
                K3  \  A11[ ][ ]A10 |  K2
                K5  /  A13[ ][ ]A12 |  K4
                K7  \  A15[ ][ ]A14 |  K6
                C4  /  D33[ ][ ]D32 |  C5
   RAM /RAS --- C2  \  D35[ ][ ]D34 |  C3 --- RAM /CAS
    RAM /OE --- C0  /  D37[ ][ ]D36 |  C1 --- RAM /WE
                G2  \  D39[ ][ ]D38 |  D7
                G0  /  D41[ ][ ]D40 |  G1
                L6  \  D43[ ][ ]D42 |  L7
                L4  /  D45[ ][ ]D44 |  L5
                L2  \  D47[ ][ ]D46 |  L3
                     ---------------+


  DRAM chip pinout (NEC 41464, TMS 4464, etc.):

                     +-----\/-----+
                   1 | /OE    GND | 18
                   2 | I/O1  I/O4 | 17
                   3 | I/O2  /CAS | 16
                   4 | /WE   I/O3 | 15
                   5 | /RAS    A0 | 14
                   6 | A6      A1 | 13
                   7 | A5      A2 | 12
                   8 | A4      A3 | 11
                   9 | VCC     A7 | 10
                     +------------+
  DRAM power pins:

  - pin 9  (Vcc) -> +5v
  - pin 18 (GND) -> GND

  How to diagnose:

  1. Test electrical condition of the chip **FIRST**:

  - Check for shorted Vcc
  - Inputs Ax, /OE, /WE, /RAS, /CAS must be high-Z

  2. Run this test and check serial output (115200 bps) for PASS/FAIL test cases.
  
 */

#define nOE  0
#define nWE  1
#define nRAS 2
#define nCAS 3

#define SETB(port, b)  port |= _BV(b)
#define CLRB(port, b)  port &= ~_BV(b)
#define ISSET(port, b) ((port & _BV(b)) != 0)

uint32_t prng_state;

/* Not sure what's wrong with Arduino but you MUST split the return type 
 * and function name in separate lines or you'll get weird compiler errors
 * on perfectly okay C code.
 */
inline void
w(uint8_t row, uint8_t col, uint8_t v) 
{
    DDRF |= 0b00001111;                               // Set I/Ox as OUTPUT
    PORTF = (PORTF & 0b11110000) | (v & 0b00001111);  // set i/o data
    PORTA = row;                                      // set row
    CLRB(PORTC, nRAS);                                // RAS down
    PORTA = col;                                      // hold nWE on
    CLRB(PORTC, nCAS);                                // CAS down
    asm("nop");                                       // > th(CLD) = 45 ns (TMS4464)
    CLRB(PORTC, nWE);                                 // WE down
    asm("nop");
    SETB(PORTC, nWE);                                 // WE up
    SETB(PORTC, nRAS);                                // RAS up
    SETB(PORTC, nCAS);                                // CAS up
    DDRF &= 0b11110000;                               // Set I/Ox as INPUT again
    PORTF &= 0b11110000;                              // ... no pull-ups
}

inline uint8_t
r(uint8_t row, uint8_t col)
{
    // read
    PORTA = row;                                      // set row
    CLRB(PORTC, nRAS);                                // RAS down
    asm("nop");                                       // hold >15n (TMS4464 -10)
    PORTA = col;                                      // set col
    CLRB(PORTC, nCAS);                                // CAS down
    CLRB(PORTC, nOE);                                 // OE down
    asm("nop");
    
    uint8_t v = PINF & 0b00001111;                    // do READ

    SETB(PORTC, nOE);                                 // OE up
    SETB(PORTC, nRAS);                                // RAS up
    SETB(PORTC, nCAS);                                // CAS up

    return v;
}

inline uint8_t
prng()
{
  uint32_t x = prng_state;
  x ^= x << 13;
  x ^= x << 17;
  x ^= x << 5;
  prng_state = x;
  return x & 0xff;
}

void
setup() 
{
  Serial.begin(115200);

  // Pin functions:
  
  // PA0..7 = address generator (RAM A0..7/8..15 multiplexed)
  // PC0..4 = RAM control
  // PF0..7 = I/O1..4
  
  // Pin direction: 0 = in/pullup, 1 = out
  DDRA     = 0b11111111;     DDRC     = 0b00001111;     DDRF    = 0b00000000;    
  // PA7 (D29) 1------- A7
  // PA6 (D28) -1------ A6
  // PA5 (D27) --1----- A5
  // PA4 (D26) ---1---- A4
  // PA3 (D25) ----1--- A3   // PC3 (D34) ----1--- /CAS // PF3 (A3) ----0--- I/O4
  // PA2 (D24) -----1-- A2   // PC2 (D35) -----1-- /RAS // PF2 (A2) -----0-- I/O3
  // PA1 (D23) ------1- A1   // PC1 (D36) ------1- /WE  // PF1 (A1) ------0- I/O2
  // PA0 (D22) -------1 A0   // PC0 (D37) -------1 /OE  // PF0 (A0) -------0 I/O1

  // if DDRD == 0 (in)  -> 0 = no pull-up, 1 = pull-up
  // if DDRD == 1 (out) -> 0 = low output, 1 = high output
  PORTA    = 0b00000000;     PORTC    = 0b00001111;     PORTF   = 0b00000000;
  // PA7 (D29) 0------- A7
  // PA6 (D28) -0------ A6
  // PA5 (D27) --0----- A5
  // PA4 (D26) ---0---- A4
  // PA3 (D25) ----0--- A3   // PC3 (D34) ----1--- A11  // PF3 (A3) ----0--- D3
  // PA2 (D24) -----0-- A2   // PC2 (D35) -----1-- A10  // PF2 (A2) -----0-- D2
  // PA1 (D23) ------0- A1   // PC1 (D36) ------1- A9   // PF1 (A1) ------0- D1
  // PA0 (D22) -------0 A0   // PC0 (D37) -------1 A8   // PF0 (A0) -------0 D0
}


void
test_and_print(char *title, int (*test_func)()) 
{
  Serial.print(title);
  Serial.print("... ");
  Serial.flush();
  cli();
  if ((*test_func)()) {
    Serial.println("PASS");
  } else {
    Serial.println("FAIL!!!");
  }
  sei();
}

int
test(int v) 
{
  uint8_t col = 0, row = 0;
  v &= 0b1111;

  // Write v
  do {
    do {
      w(row, col, v);
      row++;    
    } while (row);
    col++;
  } while (col);

  // Verify
  do {
    do {
      if (r(row, col) != v) return 0;
      row++;
    } while (row);
    col++;
  } while (col);

  return 1;
}

int
test_1() {
  return test(0b1111);
}

int
test_0() {
  return test(0b0000);
}

int
test_01() {
  return test(0b0101);
}

int
test_10() {
  return test(0b1010);
}

int
test_rand() {
  uint8_t col = 0, row = 0;
  uint32_t seed = micros();

  // Write
  prng_state = seed;
  do {
    do {
      w(row, col, prng() & 0b1111);
      row++;    
    } while (row);
    col++;
  } while (col);

  // Verify
  prng_state = seed;
  do {
    do {
      if (r(row, col) != (prng() & 0b1111)) return 0;
      row++;
    } while (row);
    col++;
  } while (col);
  return 1;
}

void
loop() {
  test_and_print("1111", test_1);
  test_and_print("0000", test_0);
  test_and_print("0101", test_01);
  test_and_print("1010", test_10);
  // TODO: minimum row refresh cycle of 4 ms can't be guaranteed with the current RNG, let's not do this
  // test_and_print("rand", test_rand); 
}
