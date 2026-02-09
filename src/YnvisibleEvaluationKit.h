
/**
 * @file YnvisibleEvaluationKit.h
 * @brief Pin mappings, segment masks and high-level helper API for Ynvisible Evaluation Kits.
 *
 * This header defines all Evaluation Kit variants supported by the Ynvisible Driver v5,
 * including:
 *  - Single Segment Display
 *  - 7-Segment Display with Dot
 *  - 15-Segment Displays (Negative sign and Middle Dot variants)
 *  - 3-Bar Display (vertical bar graph)
 *  - 7-Bar Display (vertical bar graph)
 *
 * Responsibilities:
 *  - Provide pin lists and segment counts for each Eval Kit type.
 *  - Define animation identifiers and timing parameters.
 *  - Declare high-level helpers used in YnvisibleEvaluationKit.cpp.
 *  - Expose masks and structures used for 15-segment digit mapping.
 *
 * Notes:
 *  - All displays are driven internally using the YNV_ECD class.
 *  - This module intentionally contains no low-level driving logic.
 *    It acts as a factory + UI helper layer for demos and prototypes.
 *
 * Created by @BFFonseca - Ynvisible (2025)
 * Updated by JoCFMendes - Ynvisible (Jan 2026)
 */

#ifndef _YNVISIBLE_EVAL_KIT_
#define _YNVISIBLE_EVAL_KIT_

#include "YnvisibleECD.h"


/***************************************************************************/
/*************************** DISPLAY CONFIGURATION *************************/
/***************************************************************************/

/* ------------------------- Single Segment Display ------------------------ */
#define EVAL_KIT_SINGLE_NUM_SEGMENTS      1                            // Nr. of segments
#define EVAL_KIT_SINGLE_PIN_LIST          { PIN_SEG_1 }                // Single WE pin


/* ------------------------ 7-Segment Display (with Dot) ------------------- */
#define EVAL_KIT_7SEG_DOT_NUM_SEGMENTS              8                  // 7-seg + dot
#define EVAL_KIT_7SEG_DOT_PIN_LIST                  \
        { PIN_SEG_8, PIN_SEG_7, PIN_SEG_5, PIN_SEG_6, PIN_SEG_4, PIN_SEG_3, PIN_SEG_1, PIN_SEG_2 }
#define EVAL_KIT_7SEG_DOT_MASK_NUM_OF_ANIMATIONS    11                 // 0..9 + "all off"


/* ------------------------ 15-Segment Display (Negative) ------------------ */
/* Segment order: Extra sign, Tens digits, Units digits */
#define EVAL_KIT_15SEG_NEGATIVE_NUM_SEGMENTS        15
#define EVAL_KIT_15SEG_NEGATIVE_PIN_LIST            \
        { PIN_SEG_4, PIN_SEG_2, PIN_SEG_1, PIN_SEG_8, PIN_SEG_7, PIN_SEG_6, PIN_SEG_3, PIN_SEG_5, \
          PIN_SEG_14, PIN_SEG_13, PIN_SEG_11, PIN_SEG_10, PIN_SEG_9, PIN_SEG_15, PIN_SEG_12 }


/* ------------------------ 15-Segment Display (Dot) ----------------------- */
#define EVAL_KIT_15SEG_DOT_NUM_SEGMENTS             15
#define EVAL_KIT_15SEG_DOT_PIN_LIST                 \
        { PIN_SEG_8, PIN_SEG_1, PIN_SEG_7, PIN_SEG_6, PIN_SEG_5, PIN_SEG_4, PIN_SEG_2, PIN_SEG_3, \
          PIN_SEG_14, PIN_SEG_13, PIN_SEG_11, PIN_SEG_10, PIN_SEG_9, PIN_SEG_15, PIN_SEG_12 }


/* ------------------------ 3-Bar Display --------------------------------- */
#define EVAL_KIT_3BARS_NUM_SEGMENTS                 3
#define EVAL_KIT_3BARS_PIN_LIST                     { PIN_SEG_2, PIN_SEG_1, PIN_SEG_3 }


/* ------------------------ 7-Bar Display --------------------------------- */
#define EVAL_KIT_7BARS_NUM_SEGMENTS                 7
#define EVAL_KIT_7BARS_PIN_LIST                     \
        { PIN_SEG_4, PIN_SEG_3, PIN_SEG_5, PIN_SEG_2, PIN_SEG_6, PIN_SEG_1, PIN_SEG_7 }


/***************************************************************************/
/***************************** ANIMATION CONFIG *****************************/
/***************************************************************************/

#define EVAL_KIT_NUM_ANIMATIONS                     15      // Total number of demo animations

// Direct Toggle animation
#define EVAL_KIT_DIRECT_TOGGLE_DELAY                4500    // (ms) ON/OFF duration

// 15-Segment displays
#define EVAL_KIT_15SEG_COUNT_DELAY                  3000    // (ms) Time each number is ON

// Single Segment display
#define EVAL_KIT_SINGLE_ON_TIME                     10000   // (ms) BLEACH→COLOR hold
#define EVAL_KIT_SINGLE_OFF_TIME                    500     // (ms) COLOR→BLEACH hold

// 7-Segment display
#define EVAL_KIT_7SEG_DOT_COUNT_DELAY               2000    // (ms) Time each number is ON

// 7-Bar display
#define EVAL_KIT_7BAR_COUNT_DELAY                   1000    // (ms) Steps for count-up/down

// 3-Bar display
#define EVAL_KIT_3BAR_COUNT_DELAY                   500     // (ms) Steps for count-up/down
#define EVAL_KIT_3BAR_BLINK_TIME                    500     // (ms) ON/OFF blink duration
#define EVAL_KIT_3BAR_BLINK_NUM                     3       // Number of blink repetitions


/***************************************************************************/
/***************************** ANIMATION ENUM ******************************/
/***************************************************************************/

/**
 * @brief Types of animations used in Evaluation Kit demo sequences.
 */
enum evaluationKitAnimations_e {
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


/***************************************************************************/
/****************************** DATA STRUCTURES ****************************/
/***************************************************************************/

/**
 * @brief Structure representing a full 15-segment character.
 *
 * @param extra Extra segment (minus or dot)
 * @param tens  Pointer to 7‑segment tens mask
 * @param units Pointer to 7‑segment units mask
 */
struct EK_15Seg_Struct_t {
    bool extra;     
    bool *tens;     
    bool *units;    
};

/**
 * @brief Structure storing last displayed two-digit value.
 */
struct EK_15Seg_Values_t {
    unsigned int tensDigit;
    unsigned int unitsDigit;
};


/***************************************************************************/
/******************************* API FUNCTIONS ******************************/
/***************************************************************************/

/* ---- Initialization ---- */
void evaluationKitInit(void);

/* ---- Driving Control ---- */
void displayStopAnimation(void);
void displayCancelAnimation(void);

/* ---- 15-Segment Displays ---- */
void display15SegNegInit(void);
void display15SegNegRun(unsigned int number, bool minus);
void display15SegDotInit(void);
void display15SegDotRun(unsigned int number, bool dot);

/* ---- Single Segment ---- */
void displaySingleSet(bool state);

/* ---- 7-Segment ---- */
void display7SegDotRun(unsigned int number, bool dot);

/* ---- 7-Bar Display ---- */
void display7BarsSet(unsigned int segment, bool state);
void display7BarsClear(void);

/* ---- 3-Bar Display ---- */
void display3BarsSet(unsigned int segment, bool state);
void display3BarsClear(void);

/* ---- Direct Drive ---- */
void displayDirectSetAll(bool state, uint16_t driveTime);

#endif  // _YNVISIBLE_EVAL_KIT_


/***************************************************************************
 ****************************** END OF FILE ********************************
 ***************************************************************************/
