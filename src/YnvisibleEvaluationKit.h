#ifndef _YNVISIBLE_EVAL_KIT_
#define _YNVISIBLE_EVAL_KIT_
#include "YnvisibleECD.h"

#define EVAL_KIT_SINGLE_NUM_SEGMENTS                1
#define EVAL_KIT_SINGLE_PIN_LIST                    {PIN_SEG_1}

#define EVAL_KIT_7SEG_DOT_NUM_SEGMENTS              8
#define EVAL_KIT_7SEG_DOT_PIN_LIST                  {PIN_SEG_8, PIN_SEG_7, PIN_SEG_5, PIN_SEG_6, PIN_SEG_4, PIN_SEG_3, PIN_SEG_1, PIN_SEG_2}
#define EVAL_KIT_7SEG_DOT_MASK_NUM_OF_ANIMATIONS    11

#define EVAL_KIT_15SEG_NEGATIVE_NUM_SEGMENTS        15
#define EVAL_KIT_15SEG_NEGATIVE_PIN_LIST            {PIN_SEG_4, PIN_SEG_2, PIN_SEG_1, PIN_SEG_8, PIN_SEG_7, PIN_SEG_6, PIN_SEG_3, PIN_SEG_5, PIN_SEG_14, PIN_SEG_13, PIN_SEG_11, PIN_SEG_10, PIN_SEG_9, PIN_SEG_15, PIN_SEG_12} //Negative Seg, Tens, Units

#define EVAL_KIT_15SEG_DOT_NUM_SEGMENTS             15
#define EVAL_KIT_15SEG_DOT_PIN_LIST                 {PIN_SEG_8, PIN_SEG_1, PIN_SEG_7, PIN_SEG_6, PIN_SEG_5, PIN_SEG_4, PIN_SEG_2, PIN_SEG_3, PIN_SEG_14, PIN_SEG_13, PIN_SEG_11, PIN_SEG_10, PIN_SEG_9, PIN_SEG_15, PIN_SEG_12}

#define EVAL_KIT_3BARS_NUM_SEGMENTS                 3
#define EVAL_KIT_3BARS_PIN_LIST                     {PIN_SEG_2, PIN_SEG_1, PIN_SEG_3}

#define EVAL_KIT_7BARS_NUM_SEGMENTS                 7
#define EVAL_KIT_7BARS_PIN_LIST                     {PIN_SEG_4, PIN_SEG_3, PIN_SEG_5, PIN_SEG_2, PIN_SEG_6, PIN_SEG_1, PIN_SEG_7}

#define EVAL_KIT_NUM_ANIMATIONS                     15

/***********************************************
 *          Animation Parameters               * 
 ***********************************************/
#define EVAL_KIT_DIRECT_TOGGLE_DELAY            4500        // ms - time for each toggle step (on/off)

// 15 Segment Displays
#define EVAL_KIT_15SEG_COUNT_DELAY              3000        // ms - time each number is ON

// Single Segment Display
#define EVAL_KIT_SINGLE_ON_TIME                 10000       // ms - time the display is ON
#define EVAL_KIT_SINGLE_OFF_TIME                500         // ms - time the display is ON

// 7 Segment Display
#define EVAL_KIT_7SEG_DOT_COUNT_DELAY           2000        // ms - time each number is ON

// 7 Bar Display
#define EVAL_KIT_7BAR_COUNT_DELAY               1000        // ms - time each bar takes to turn ON

// 3 Bar Display
#define EVAL_KIT_3BAR_COUNT_DELAY               500        // ms - time each bar takes to turn ON
#define EVAL_KIT_3BAR_BLINK_TIME                500         // ms - time for each blink step (on/off).
#define EVAL_KIT_3BAR_BLINK_NUM                 3           // number of blinks for each bar

enum evaluationKitAnimations_e{
    EVAL_ANIMATION_DIRECT_TOGGLE = 0,
    EVAL_ANIMATION_15SEG_NEGATIVE_POS_UP,
    EVAL_ANIMATION_15SEG_NEGATIVE_POS_DOWN,
    EVAL_ANIMATION_15SEG_NEGATIVE_NEG_UP,
    EVAL_ANIMATION_15SEG_NEGATIVE_NEG_DOWN,
    EVAL_ANIMATION_15SEG_DOT_UP,
    EVAL_ANIMATION_15SEG_DOT_DOWN,
    EVAL_ANIMATION_SINGLE_ON,
    EVAL_ANIMATION_7SEG_DOT_COUNT_UP,
    EVAL_ANIMATION_7SEG_DOT_COUNT_DOWN,
    EVAL_ANIMATION_7BARS_COUNT_UP,
    EVAL_ANIMATION_7BARS_COUNT_DOWN,
    EVAL_ANIMATION_3BARS_COUNT_UP,
    EVAL_ANIMATION_3BARS_COUNT_DOWN,
    EVAL_ANIMATION_3BARS_MID_TOP_BOT
};

struct EK_15Seg_Struct_t{
    bool extra;             // Minus or dot segments
    bool *tens;             // Tens digit segments (Left)
    bool *units;            // Units digit segments (Right)
};
struct EK_15Seg_Values_t{
    unsigned int tensDigit;
    unsigned int unitsDigit;
};

void evaluationKitInit(void);
void displayStopAnimation(void);
void displayCancelAnimation(void);

void display15SegNegInit(void);
void display15SegNegRun(unsigned int number, bool minus);
void display15SegDotInit(void);
void display15SegDotRun(unsigned int number, bool dot);

void displaySingleSet(bool state);

void display7SegDotRun(unsigned int number, bool dot);

void display7BarsSet(unsigned int segment, bool state);
void display7BarsClear(void);

void display3BarsSet(unsigned int segment, bool state);
void display3BarsClear(void);

void displayDirectSetAll(bool state, uint16_t driveTime);

#endif  // _YNVISIBLE_DRIVER5_EVALUATION