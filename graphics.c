#include <stdint.h>
#include <string.h>

#include "msp430g2553.h"

#include "graphics.h"

const uint8_t kScreenMaxCoordinate = 7;

static const uint8_t kLedBrightness = 0xE1;


//static enum Color led_colors[(kScreenMaxCoordinate + 1) * (kScreenMaxCoordinate + 1) + 1];
static enum Color led_colors[65];



extern void EraseLedBuffer() {
    led_colors[0] = kBlack;
    SetScreenSolidColor(kBlack);
}


extern void SetScreenBufferColor(const uint8_t x_coordinate, const uint8_t y_coordinate, const enum Color color) {
    led_colors[y_coordinate * (kScreenMaxCoordinate + 1) + (kScreenMaxCoordinate - x_coordinate) + 1] = color;
}

extern void SetStatusLedColor(const enum Color color) {
    led_colors[0] = color;
}


// Linearly increases red intensity over second half of range, zero otherwise
static uint8_t r_val(const uint8_t temp) {
    if (temp == 0) {
        return 0;
    } else if (temp < 128) {
        return 0;               // Returns 0 red value for first half of range
    } else {
        return (temp - 128) << 1; // Multiply by 2 and prevent overflow using left shift
    }
}

// Linearly increases green intensity over first half of range and linearly decreases green intensity for second half of range
static uint8_t g_val(const uint8_t temp) {
    if (temp == 0) {
        return 0;
    } else if (temp < 128) {
        return temp << 1;         // Multiply by 2 and prevent overflow using left shift
    } else {
        return (255 - temp) << 1; // Multiply by 2 and prevent overflow using left shift
    }
}

// Linearly decreases blue intensity over first half of range, zero otherwise
static uint8_t b_val(const uint8_t temp) {
    if (temp == 0) {
        return 0;
    } else if (temp < 128) {
        return (127 - temp) << 1; // Multiply by 2 and prevent overflow using left shift
    } else {
        return 0;                   // Returns 0 blue value for second half of range
    }
}


// initializes SPI communication to LEDs
extern void InitializeGraphics() {
    P1SEL |= BIT2 + BIT4; // P1.4 as USCI_A0 SPI clock, P1.2 as USCI_A0 SPI MOSI pin
    P1SEL2 |= BIT2 + BIT4;

    UCA0CTL1 = UCSWRST;                           // Disable SPI
    UCA0CTL0 |= UCCKPH + UCMST + UCSYNC + UCMSB; // 8-bit SPI master, MSb 1st, synchronous mode
    UCA0CTL1 |= UCSSEL_2;                         // SMCLK
    UCA0BR0 = 0x04;                               // Set Frequency divider to 4
    UCA0BR1 = 0;
    UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine, USCI reset released for operation.
    //IE2 |= UCA0TXIE; // Activate USCI_B transmit interrupt--calls when transmit buffer is ready for new data
}

extern void SendSpiByte(const uint8_t byte) {
    UCA0TXBUF = byte;
    IFG2 |= UCA0TXIFG;         // Tells USCI to trigger transfer into the buffer
    IE2 |= UCA0TXIE;
    __bis_SR_register(CPUOFF + GIE); //enable general interrupts, enter sleep
    IE2 &= ~UCA0TXIE;
}


extern void SendFrameBuffer() {
    // Send first frame (4 bytes of all 0s)
    for (uint8_t i = 0; i < 4; ++i) {
        SendSpiByte(0x00);
    }

    for (uint8_t i = 0; i < 65; ++i) {
        SendSpiByte(kLedBrightness);

        SendSpiByte(b_val(led_colors[i]));
        SendSpiByte(g_val(led_colors[i]));
        SendSpiByte(r_val(led_colors[i]));



    }

    for (uint8_t i = 0; i < 4; ++i) {
        SendSpiByte(0xFF);
    }
}

extern void SetScreenSolidColor(enum Color color) {
    memset(led_colors + 1, color, 64);
}
