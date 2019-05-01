#include <msp430g2553.h>
#include <stdint.h>

#include "rand.h"

static uint16_t lfsr;


extern void srand(const uint16_t seed) {
    lfsr = seed;
}

/* This function should be initialized with a non-zero seed before used to generate random numbers.*/
extern unsigned int rand8() {
    const uint8_t lsb = lfsr & 1;
    lfsr >>= 1;

    if (lsb == 1) {
        lfsr ^= 0xB400u;
    }

    return (lfsr & 0x07); // output a random integer in range [0,3]
}
