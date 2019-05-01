#include <stdint.h>

#include "msp430g2553.h"

#define l1 261 // low C
#define l2 294 // low D
#define l3 329 // low E
#define l4 349 // low F
#define l5 391 // low G
#define l5s 415 // low G#
#define l6 440 // low A
#define l6s 455 // low A#
#define l7 494 // low B
#define m1 523 // mid C
#define m1s 554 // mid C#
#define m2 587 // mid D
#define m2s 622 // mid D#
#define m3 659 // mid E
#define m4 698 // mid F
#define m4s 740 // mid F#
#define m5 784 // mid G
#define m5s 830 // mid G#
#define m6 880 // mid A
#define h69 10000
#define h70 5000
#define h50 3000


static const unsigned int start_song[] = {m1, m6, m1, m6, m4, m3, m2, m1};
static const int win_song[] = {h69, m6, m1, h50, m4, h70, m4, m6};
static const int lose_song[] = {m2, m1, l6s, l6, l5, l4, 14, 14};

static uint8_t count1 = 0;
static uint8_t i = 0;

static void PlaySong(unsigned int *note_string, uint8_t notes_length, uint8_t tempo) {
    if (count1 == tempo) {
        count1 = 0;
        TA1CCR0 = 1000000 / note_string[i]; // This determines note frequency, PWM period
        TA1CCR1 = TA1CCR0 / 2;              // Switch buzzer off for part of the period
        i++;
        if(i == notes_length){
            i = 0;
        }
    } else if (count1 == tempo - 1) {
        TA1CCR0 = 0;    // Stop tone for a little bit
        TA1CCR1 = 0;
    }
    count1++;

}

extern void StopSound() {
    TA1CCR1 = 0;    // Turn off the buzzer
    count1 = 0;
    i = 0;
}

extern void ChooseSong(uint8_t song_num) {
    switch (song_num)
    {
    case 0:
        PlaySong(start_song, 8, 8);
        break;
    case 1:
        PlaySong(win_song, 8, 8);
        break;
    case 2:
        PlaySong(lose_song, 8, 3);
        break;
    }
}

