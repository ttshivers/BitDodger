#include <stdbool.h>
#include <stdint.h>

#include "msp430g2553.h"


#include "graphics.h"
#include "rand.h"
#include "sound.h"

// Constants
static const uint8_t kTurnsWinThreshold = 100; // number of remaining turns needed to win
static const uint8_t kMaxItems = 8; // max number of items
static const uint8_t kCoinReward = 20;  // time reward for coin
static const uint8_t kBombPenalty = 20;
static const uint8_t kItemGenerationPeriod = 2;
static const uint8_t kCoinChance = 4;
static const uint8_t kTurnDelay = 20;

static const enum Color kPlayerColor = kGreen;
static const enum Color kBombColor = kRed;
static const enum Color kCoinColor = kYellow;



enum ItemType {
    kUnallocatedItem,
    kCoin,
    kBomb
};


enum Button {
    kNoButton,
    kLeftButton,
    kRightButton
};

struct Item {
    enum ItemType type;
    uint8_t x_coordinate;
    uint8_t y_coordinate;
}__attribute__((packed));


static struct Item items[kMaxItems];


static enum Button button_pressed = kNoButton;

static unsigned int player_x_coordinate = 0;                    // starts player at (0,7)
static int remaining_turns = kTurnsWinThreshold / 2;       // initializes remaining turns to the max

static uint8_t item_generation_delay = 0;





static bool IsItemUnallocated(const uint8_t item_index) {
    return items[item_index].type == kUnallocatedItem;
}

static enum Color GetItemColor(const uint8_t item_index) {
    switch (items[item_index].type) {
        case kBomb: {
            return kBombColor;
        }

        case kCoin: {
            return kCoinColor;
        }

        default: {
            return kBlack;
        }
    }
}




static void UpdatePlayerPosition() {
    /*
     * Updates player position
     */
    switch (button_pressed) {
        case kLeftButton: {
            if (player_x_coordinate < kScreenMaxCoordinate) {
                // If player is not on the right side of the screen
                ++player_x_coordinate;
            }
            break;
        }

        case kRightButton: {
            if (player_x_coordinate > 0) {
                // If player is on the left side of screen
                --player_x_coordinate;
            }
            break;
        }
    }

    // Reset button to nothing
    button_pressed = kNoButton;
}


static int GetFreeItemIndex() {
    for (uint8_t item_index = 0; item_index < kMaxItems; ++item_index) {
        if (IsItemUnallocated(item_index)) {
            return item_index;
        }
    }

    return -1;
}

// picks either bomb or coin with a ratio of 8:1
enum ItemType GenerateRandomItemType() {
    return rand8() < kCoinChance ? kCoin : kBomb;
}

static bool CreateRandomItem() {
    const int item_index = GetFreeItemIndex();
    if (item_index >= 0) {
        items[item_index].type = GenerateRandomItemType();
        items[item_index].x_coordinate = rand8();
        items[item_index].y_coordinate = 0;
        return true;
    }

    return false;
}

static void HandleItemGeneration() {
    //generates new item at a set rate
    if (item_generation_delay >= kItemGenerationPeriod) {
        CreateRandomItem();
        item_generation_delay = 0;
    } else {
        ++item_generation_delay;
    }
}

static bool IsItemOffscreen(const uint8_t item_index) {
    return items[item_index].y_coordinate > kScreenMaxCoordinate;
}

static void MoveItemDown(const uint8_t item_index) {
    items[item_index].y_coordinate += 1;
}


static void RemoveItem(const uint8_t item_index) {
    items[item_index].type = kUnallocatedItem;
}

static bool IsItemOverlappingPlayer(const uint8_t item_index) {
    return items[item_index].y_coordinate == kScreenMaxCoordinate && items[item_index].x_coordinate == player_x_coordinate;
}


static void HandlePlayerCoinCollission() {
    remaining_turns += kCoinReward;
}

static void HandlePlayerBombCollission() {
    remaining_turns -= kBombPenalty;
}

static void UpdateItemPosition(const uint8_t item_index) {
    MoveItemDown(item_index);

    if (IsItemOffscreen(item_index)) {
        RemoveItem(item_index);
    } else if (IsItemOverlappingPlayer(item_index)) {
        switch(items[item_index].type) {
            case kCoin: {
                HandlePlayerCoinCollission();
                break;
            }

            case kBomb: {
                HandlePlayerBombCollission();
                break;
            }
        }
    }
}


static void UpdateItemsPosition() {
    for (uint8_t item_index = 0; item_index < kMaxItems; ++item_index) {
        if (!IsItemUnallocated(item_index)) {
            UpdateItemPosition(item_index);
        }
    }
}

static bool IsGameWon() {
    return remaining_turns >= kTurnsWinThreshold;
}

static bool IsGameLost() {
    return remaining_turns <= 0;
}





static void RenderPlayer() {
    SetScreenBufferColor(player_x_coordinate, kScreenMaxCoordinate, kPlayerColor);

}

static void RenderItem(const uint8_t item_index) {
    SetScreenBufferColor(items[item_index].x_coordinate, items[item_index].y_coordinate, GetItemColor(item_index));
}

static void DisplayStatus() {
    SetStatusLedColor(256 * (unsigned int)remaining_turns / kTurnsWinThreshold);
}

static void RenderItems() {
    for (uint8_t item_index = 0; item_index < kMaxItems; ++item_index) {
        if (!IsItemUnallocated(item_index)) {
            RenderItem(item_index);
        }
    }
}

static void RenderGraphics() {
    RenderPlayer();
    RenderItems();
    DisplayStatus();
}

static void sleep(uint8_t count) {
    for (uint8_t i = 0; i < count; ++i) {
        __bis_SR_register(CPUOFF + GIE);
    }
}

static void ResetGameState() {
    for (uint8_t i = 0; i < kMaxItems; ++i) {
        items[i].type = kUnallocatedItem;
    }

    player_x_coordinate = 0;
    remaining_turns = kTurnsWinThreshold / 2;
}

// animation for loss from time
static void HandleTimeLoss() {
    EraseLedBuffer();


    uint8_t counter = 0;
    while (true) {
        //EraseLedBuffer();

        uint8_t led_counter = 0;
        for (uint8_t i = 0; i <= kScreenMaxCoordinate; ++i) {
            for (uint8_t j = 0; j <= kScreenMaxCoordinate; ++j) {
                if (led_counter == counter) {
                    SetScreenBufferColor(i, j, kRed);
                }

                ++led_counter;
            }
        }

        SendFrameBuffer();

       sleep(1);

       if (++counter >= 65) {
           break;
       }
    }


    while (true) {
        ChooseSong(2);
        if (button_pressed != kNoButton) {
            button_pressed = kNoButton;
            ResetGameState();
            break;
        }

        EraseLedBuffer();
        SendFrameBuffer();
        sleep(3);

        ChooseSong(2);
        SetScreenSolidColor(kRed);
        SendFrameBuffer();
        sleep(20);
    }

}

static void StartingAnimation() {
    EraseLedBuffer();
    uint8_t color = 1;
    uint8_t global_counter = 0;

    const uint8_t dim = kScreenMaxCoordinate + 1;
    while (true) {
        ChooseSong(0);
        if (button_pressed != kNoButton) {
            button_pressed = kNoButton;
            break;
        }

        uint8_t m = dim;
        uint8_t n = dim;
        int i, k = 0, l = 0;

        /*  k - starting row index
            m - ending row index
            l - starting column index
            n - ending column index
            i - iterator
        */
        uint8_t counter = 0;
        while (k < m && l < n)  {
            /* Print the first row from the remaining rows */
            for (i = l; i < n; ++i) {
                if (counter == global_counter) {
                    SetScreenBufferColor(k, i, color++);
                }
                ++counter;
            }
            k++;

            /* Print the last column from the remaining columns */
            for (i = k; i < m; ++i) {
                if (counter == global_counter) {
                    SetScreenBufferColor(i, n-1, color++);
                }
                ++counter;
            }
            n--;

            /* Print the last row from the remaining rows */
            if (k < m) {
                for (i = n-1; i >= l; --i) {
                    if (counter == global_counter) {
                        SetScreenBufferColor(m-1, i, color++);
                    }
                    ++counter;
                }
                m--;
            }

            /* Print the first column from the remaining columns */
            if (l < n) {
                for (i = m-1; i >= k; --i) {
                    if (counter == global_counter) {
                        SetScreenBufferColor(i, l, color++);
                    }
                    ++counter;
                }
                l++;
            }
        }



        global_counter++;
        if (global_counter >= 65) {
            global_counter = 0;
        }
        SendFrameBuffer();
        sleep(1);
    }
}

// animation for loss from bomb
static void HandleBombLoss() {
    while (true) {
        ChooseSong(2);
        if (button_pressed != kNoButton) {
            button_pressed = kNoButton;
            ResetGameState();
            break;
        }

        for (uint8_t i = 0; i <= kScreenMaxCoordinate; ++i) {
            for (uint8_t j = 0; j <= kScreenMaxCoordinate; ++j) {
                if (i == player_x_coordinate || j == kScreenMaxCoordinate) {
                    SetScreenBufferColor(i, j, kBlack);
                }
            }
        }

        SendFrameBuffer();
        sleep(3);

        ChooseSong(2);
        for (uint8_t i = 0; i <= kScreenMaxCoordinate; ++i) {
            for (uint8_t j = 0; j <= kScreenMaxCoordinate; ++j) {
                if (i == player_x_coordinate || j == kScreenMaxCoordinate) {
                    SetScreenBufferColor(i, j, kRed);
                }
            }
        }
        SendFrameBuffer();
        sleep(20);
    }
}

// animation for win from collecting coins
static void HandleWin() {
    EraseLedBuffer();

    uint8_t color = 1;

    uint8_t counter = 0;
    while (true) {
        ChooseSong(1);
        if (button_pressed != kNoButton) {
            button_pressed = kNoButton;
            ResetGameState();
            break;
        }


        uint8_t led_counter = 0;
        for (uint8_t i = 0; i <= kScreenMaxCoordinate; ++i) {
            for (uint8_t j = 0; j <= kScreenMaxCoordinate; ++j) {
                if (led_counter == counter) {
                    SetScreenBufferColor(i, j, color);
                    ++color;
                }

                ++led_counter;
            }
        }

        SendFrameBuffer();

        if (++counter >= 65) {
            counter = 0;
        }
        sleep(1);
    }

}

static void HandleTurn() {
    EraseLedBuffer();

    //checks whether time has run out
    if (IsGameLost()) {
        HandleTimeLoss();
        return;
    }
    UpdateItemsPosition();
    HandleItemGeneration();
    UpdatePlayerPosition();


    RenderGraphics();
    SendFrameBuffer();

    // Win Condition check coins
    if (IsGameWon()) {
        HandleWin();
    } else if (IsGameLost()) {
        HandleBombLoss();
    }

    --remaining_turns;
}


int main() {
    if (CALBC1_1MHZ == 0xFF || CALDCO_1MHZ == 0xff)
        while (1);

    BCSCTL1 = CALBC1_1MHZ;      // DCO to 1 MHz
    DCOCTL = CALDCO_1MHZ;       // load calibration data
    BCSCTL3 |= LFXT1S_2;        // ACLK source from VLO

    // Buzzer output pins
    P2DIR |= BIT1 + BIT5;       // P2.1 & P2.5 output pins for buzzer
    P2OUT &= ~BIT5;             // P2.5 ground pin for buzzer
    P2SEL |= BIT1;              // P2.1 buzzer PWM for TA1.1

    InitializeGraphics();                //SPI and led port setup

    // TimerA1.1 PWM setup
    TA1CCTL1 = OUTMOD_7; // Output is high until the counter reaches the value of CCR1
    TA1CTL = TASSEL_2 + MC_1;     // source from SMCLK, upmode
    TA1CCR0 = 0;                  // PWM period to init value
    TA1CCR1 = 0;                  // Duty cycle 0%

    // TimerA0 Setup. Use CLK as source of noise for rng seed
    TA0CCR0 = 60000;                 // TA0 PWM period
    TA0CCR1 = 0;                   // TA0CCR1 PWM duty cycle
    TA0CCTL1 |= OUTMOD_7;          // TA0CCR1 set output mode to reset/set
    TA0CTL |= TASSEL_2 + MC_1; // Source TA0 from SMCLK, set to Up Mode (Counts to TA0CCR0, reset

    // Watchdog timer init
    WDTCTL = WDT_MDLY_8;        // Enter WDT ISR every 8ms
    IE1 |= WDTIE;               // WDT interrupts enabled

    // btn input pins config
    P2DIR &= ~(BIT0 + BIT2); // P2.0 button 1 input, P2.2 button 2 input, P12.3 button 3 input, P2.4 button 4 input
    P2IE |= BIT0 + BIT2; // P2.0, P2.2, P2.4, P2.4 interrupt enabled
    P2IES |= BIT0 + BIT2;        // Initially detect falling edges
    P2IFG &= ~(BIT0 + BIT2); // P2.0, P2.2, P2.3, P2.4 IFG cleared
    P2REN |= BIT0 + BIT2;         // Pull up until button press

    StartingAnimation();
    srand(TA0R);    // init seed

    while (true) {
        StopSound();
        HandleTurn();
        sleep(kTurnDelay);
    }
    return 0;
}



// WDT ISR
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
    __bic_SR_register_on_exit(CPUOFF + GIE); //Upon wdt interrupt, return to main while loop.
}

// Port2 ISR - button press detection
#pragma vector=PORT2_VECTOR
__interrupt void port_2(void)
{
    //P2IE &= ~P2IE;       // P2.0 interrupt disabled to avoid bouncing effect
    button_pressed = (P2IFG & BIT0) == BIT0 ? kLeftButton : kRightButton;
    P2IFG = 0;
    __bic_SR_register_on_exit(CPUOFF + GIE);
}


// Transmit Interrupt Vector
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCIB0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCIB0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    IFG2 &= ~UCA0TXIFG; // Do not allow data transfer while buffer is being updated
    __bic_SR_register_on_exit(CPUOFF + GIE); //Upon interrupt, return to regular code.
}
