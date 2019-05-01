#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <stdint.h>


extern const uint8_t kScreenMaxCoordinate;

enum Color {
    kBlack = 0,
    kBlue = 1,
    kGreen = 128,
    kYellow = 196,
    kRed = 255
};


extern void EraseLedBuffer();
extern void SetScreenBufferColor(const uint8_t x_coordinate, const uint8_t y_coordinate, const enum Color color);
extern void SetStatusLedColor(const enum Color color);
extern void SendFrameBuffer();
extern void InitializeGraphics();
extern void SetScreenSolidColor(enum Color color);


#endif /* GRAPHICS_H_ */
