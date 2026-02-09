
/**
 * @file YnvisibleEvaluationKit.cpp
 * @brief Preconfigured display objects and helper functions for Ynvisible Evaluation Kits.
 *
 * This module provides initialization routines, display masks, and convenience 
 * functions for operating different Ynvisible evaluation kit configurations.
 * It instantiates and configures YNV_ECD objects for:
 *  - Single segment display
 *  - 7-segment display with dot
 *  - Dual 7-segment (15-seg) with minus sign
 *  - Dual 7-segment (15-seg) with middle dot
 *  - 3-bar display
 *  - 7-bar display
 *
 * Responsibilities:
 *  - Configure Eval Kit-specific ECD_Config parameters (thresholds, voltages, timings).
 *  - Expose high-level display functions (set digit, bars, clear, direct drive).
 *  - Maintain per-display masks for 7-segment rendering.
 *  - Provide a generic pointer to the "current" display for animation control.
 *
 * Notes:
 *  - Core ECD driving logic is implemented in YnvisibleECD.cpp.
 *  - This file is intended for demonstration/UI helpers with Eval Kits.
 * 
 * Created by @BFFonseca - Ynvisible (2025)
 * Updated by JoCFMendes - Ynvisible (Jan 2026)
 * 
 */

#include "YnvisibleECD.h"
#include "YnvisibleEvaluationKit.h"


/***************************************************************************/
/**************************** GLOBAL VARIABLES *****************************/
/***************************************************************************/

// Pin lists for each Evaluation Kit display variant
int evalKitSinglePinList                                            = EVAL_KIT_SINGLE_PIN_LIST;
int evalKit7SegDotPinList   [EVAL_KIT_7SEG_DOT_NUM_SEGMENTS]        = EVAL_KIT_7SEG_DOT_PIN_LIST;
int evalKit15SegNegPinList  [EVAL_KIT_15SEG_NEGATIVE_NUM_SEGMENTS]  = EVAL_KIT_15SEG_NEGATIVE_PIN_LIST;
int evalKit15SegDotPinList  [EVAL_KIT_15SEG_DOT_NUM_SEGMENTS]       = EVAL_KIT_15SEG_DOT_PIN_LIST;
int evalKit3BarsPinList     [EVAL_KIT_3BARS_NUM_SEGMENTS]           = EVAL_KIT_3BARS_PIN_LIST;
int evalKit7BarsPinList     [EVAL_KIT_7BARS_NUM_SEGMENTS]           = EVAL_KIT_7BARS_PIN_LIST;

// Pointer to the currently active display (used by generic helpers)
static YNV_ECD* p_currentDisplay = nullptr;

// Static global driving flag (shared by all YNV_ECD objects)
bool YNV_ECD::m_stopDrivingFlag = false;

// Pre-instantiated YNV_ECD objects for each display type
YNV_ECD ecdEvalKitSingle   (EVAL_KIT_SINGLE_NUM_SEGMENTS,        &evalKitSinglePinList);      // Single segment display
YNV_ECD ecdEvalKit7SegDot  (EVAL_KIT_7SEG_DOT_NUM_SEGMENTS,       evalKit7SegDotPinList);    // 7-seg with dot
YNV_ECD ecdEvalKit15SegNeg (EVAL_KIT_15SEG_NEGATIVE_NUM_SEGMENTS, evalKit15SegNegPinList);   // Dual 7-seg with minus sign
YNV_ECD ecdEvalKit15SegDot (EVAL_KIT_15SEG_DOT_NUM_SEGMENTS,      evalKit15SegDotPinList);   // Dual 7-seg with middle dot
YNV_ECD ecdEvalKit3Bars    (EVAL_KIT_3BARS_NUM_SEGMENTS,          evalKit3BarsPinList);      // 3-bar display (bottom to top)
YNV_ECD ecdEvalKit7Bars    (EVAL_KIT_7BARS_NUM_SEGMENTS,          evalKit7BarsPinList);      // 7-bar display (bottom to top)

// Last two-digit value shown on 15-seg displays (used to detect tens rollover)
EK_15Seg_Values_t last15SegNumber {0, 0};

// Flags used to control whether tens digits need to be fully refreshed
bool display15SegDotUpdateTens = false;
bool display15SegNegUpdateTens = false;

// 7-seg (with dot) mask table: [digit][segmentIndex]
// Active HIGH mask (we later map to ON/OFF state).
const bool mask7SegDotsDisplay [EVAL_KIT_7SEG_DOT_MASK_NUM_OF_ANIMATIONS][EVAL_KIT_7SEG_DOT_NUM_SEGMENTS] = {
    {1,1,1,1,1,1,0},  // 0
    {0,1,1,0,0,0,0},  // 1
    {1,1,0,1,1,0,1},  // 2
    {1,1,1,1,0,0,1},  // 3
    {0,1,1,0,0,1,1},  // 4
    {1,0,1,1,0,1,1},  // 5
    {1,0,1,1,1,1,1},  // 6
    {1,1,1,0,0,0,0},  // 7
    {1,1,1,1,1,1,1},  // 8
    {1,1,1,1,0,1,1},  // 9
    {0,0,0,0,0,0,0}   // "10" - All OFF
};


/***************************************************************************/
/**
 * @brief Initialize all Evaluation Kit displays with predefined parameters.
 *
 * Configures the ECD_Config structure for each display type and calls
 * setConfig() on the corresponding YNV_ECD instances. This defines
 * thresholds, voltages and timings tailored to each Evaluation Kit.
 */
/***************************************************************************/
void evaluationKitInit(void) {

    //----------------------------------------------------/ 
    // Configuration for 3 Bars Display
    //----------------------------------------------------/
    ECD_Config evalKit3BarsConfig;

    evalKit3BarsConfig.refreshColorLimitHVoltage    = 1.1;   // (V) Color Target Threshold for refresh
    evalKit3BarsConfig.refreshColorLimitLVoltage    = 0.95;  // (V) Color Refresh Threshold for refresh
    
    evalKit3BarsConfig.refreshBleachLimitHVoltage   = 0.3;   // (V) Bleach threshold (near CE)
    evalKit3BarsConfig.refreshBleachLimitLVoltage   = 0.5;   // (V) Bleach target (further from CE)

    evalKit3BarsConfig.coloringVoltage              = 1.3;   // (V) Color transition amplitude
    evalKit3BarsConfig.refreshColoringVoltage       = 1.3;   // (V) Color refresh amplitude
    evalKit3BarsConfig.coloringTime                 = 900;   // (ms) Color transition pulse duration
    evalKit3BarsConfig.refreshColorPulseTime        = 200;   // (ms) Color refresh pulse duration

    evalKit3BarsConfig.bleachingVoltage             = 0.7;   // (V) Bleach transition amplitude
    evalKit3BarsConfig.refreshBleachingVoltage      = 0.7;   // (V) Bleach refresh amplitude
    evalKit3BarsConfig.bleachingTime                = 900;   // (ms) Bleach transition pulse duration
    evalKit3BarsConfig.refreshBleachPulseTime       = 100;   // (ms) Bleach refresh pulse duration

    ecdEvalKit3Bars.setConfig(evalKit3BarsConfig);

    //----------------------------------------------------/ 
    // Configuration for Single Segment 
    //----------------------------------------------------/
    ECD_Config evalKitSingleConfig;

    evalKitSingleConfig.refreshColorLimitHVoltage   = 1.1;   // (V) Color Target Threshold
    evalKitSingleConfig.refreshColorLimitLVoltage   = 0.95;  // (V) Color Refresh Threshold
    
    evalKitSingleConfig.refreshBleachLimitHVoltage  = 0.3;   // (V) Bleach threshold (near CE)
    evalKitSingleConfig.refreshBleachLimitLVoltage  = 0.4;   // (V) Bleach target (further from CE)

    evalKitSingleConfig.coloringVoltage             = 1.3;   // (V) Color transition amplitude
    evalKitSingleConfig.refreshColoringVoltage      = 1.3;   // (V) Color refresh amplitude
    evalKitSingleConfig.coloringTime                = 550;   // (ms) Color transition pulse
    evalKitSingleConfig.refreshColorPulseTime       = 200;   // (ms) Color refresh pulse

    evalKitSingleConfig.bleachingVoltage            = 0.7;   // (V) Bleach transition amplitude
    evalKitSingleConfig.refreshBleachingVoltage     = 0.7;   // (V) Bleach refresh amplitude
    evalKitSingleConfig.bleachingTime               = 550;   // (ms) Bleach transition pulse
    evalKitSingleConfig.refreshBleachPulseTime      = 200;   // (ms) Bleach refresh pulse

    ecdEvalKitSingle.setConfig(evalKitSingleConfig);

    //----------------------------------------------------/ 
    // Configuration for 7-segment display with dot
    //----------------------------------------------------/
    ECD_Config evalKit7SegConfig;

    evalKit7SegConfig.refreshColorLimitHVoltage     = 1.1;   // (V) Color Target Threshold
    evalKit7SegConfig.refreshColorLimitLVoltage     = 0.95;  // (V) Color Refresh Threshold
    
    evalKit7SegConfig.refreshBleachLimitHVoltage    = 0.3;   // (V) Bleach threshold (near CE)
    evalKit7SegConfig.refreshBleachLimitLVoltage    = 0.5;   // (V) Bleach target (further from CE)

    evalKit7SegConfig.coloringVoltage               = 1.3;   // (V) Color transition amplitude
    evalKit7SegConfig.refreshColoringVoltage        = 1.3;   // (V) Color refresh amplitude
    evalKit7SegConfig.coloringTime                  = 350;   // (ms) Color transition pulse
    evalKit7SegConfig.refreshColorPulseTime         = 100;   // (ms) Color refresh pulse

    evalKit7SegConfig.bleachingVoltage              = 0.7;   // (V) Bleach transition amplitude
    evalKit7SegConfig.refreshBleachingVoltage       = 0.6;   // (V) Bleach refresh amplitude
    evalKit7SegConfig.bleachingTime                 = 350;   // (ms) Bleach transition pulse
    evalKit7SegConfig.refreshBleachPulseTime        = 100;   // (ms) Bleach refresh pulse

    ecdEvalKit7SegDot.setConfig(evalKit7SegConfig);

    //----------------------------------------------------/ 
    // Configuration for 15 Segment Displays (negative and dot variants)
    //----------------------------------------------------/
    ECD_Config evalKit15SegConfig;

    evalKit15SegConfig.refreshColorLimitHVoltage    = 1.1;   // (V) Color Target Threshold
    evalKit15SegConfig.refreshColorLimitLVoltage    = 1.0;   // (V) Color Refresh Threshold
    
    evalKit15SegConfig.refreshBleachLimitHVoltage   = 0.3;   // (V) Bleach threshold (near CE)
    evalKit15SegConfig.refreshBleachLimitLVoltage   = 0.4;   // (V) Bleach target (further from CE)

    evalKit15SegConfig.coloringVoltage              = 1.3;   // (V) Color transition amplitude
    evalKit15SegConfig.refreshColoringVoltage       = 1.3;   // (V) Color refresh amplitude
    evalKit15SegConfig.coloringTime                 = 350;   // (ms) Color transition pulse
    evalKit15SegConfig.refreshColorPulseTime        = 100;   // (ms) Color refresh pulse

    evalKit15SegConfig.bleachingVoltage             = 0.7;   // (V) Bleach transition amplitude
    evalKit15SegConfig.refreshBleachingVoltage      = 0.6;   // (V) Bleach refresh amplitude
    evalKit15SegConfig.bleachingTime                = 350;   // (ms) Bleach transition pulse
    evalKit15SegConfig.refreshBleachPulseTime       = 100;   // (ms) Bleach refresh pulse

    // Apply same 15-seg configuration to both 15SegNeg and 15SegDot displays
    ecdEvalKit15SegNeg.setConfig(evalKit15SegConfig);
    ecdEvalKit15SegDot.setConfig(evalKit15SegConfig);

    //----------------------------------------------------/ 
    // Configuration for 7 Bars Display
    //----------------------------------------------------/
    ECD_Config evalKit7BarsConfig;

    evalKit7BarsConfig.refreshColorLimitHVoltage    = 1.1;   // (V) Color Target Threshold
    evalKit7BarsConfig.refreshColorLimitLVoltage    = 0.95;  // (V) Color Refresh Threshold
    
    evalKit7BarsConfig.refreshBleachLimitHVoltage   = 0.3;   // (V) Bleach threshold (near CE)
    evalKit7BarsConfig.refreshBleachLimitLVoltage   = 0.4;   // (V) Bleach target (further from CE)

    evalKit7BarsConfig.coloringVoltage              = 1.3;   // (V) Color transition amplitude
    evalKit7BarsConfig.refreshColoringVoltage       = 1.3;   // (V) Color refresh amplitude
    evalKit7BarsConfig.coloringTime                 = 350;   // (ms) Color transition pulse
    evalKit7BarsConfig.refreshColorPulseTime        = 100;   // (ms) Color refresh pulse

    evalKit7BarsConfig.bleachingVoltage             = 0.8;   // (V) Bleach transition amplitude
    evalKit7BarsConfig.refreshBleachingVoltage      = 0.7;   // (V) Bleach refresh amplitude
    evalKit7BarsConfig.bleachingTime                = 350;   // (ms) Bleach transition pulse
    evalKit7BarsConfig.refreshBleachPulseTime       = 200;   // (ms) Bleach refresh pulse
    
    ecdEvalKit7Bars.setConfig(evalKit7BarsConfig);
}


/***************************************************************************/
/**
 * @brief Request the current display to stop any ongoing driving.
 *
 * Sets the stop-driving flag on the currently active display, if available.
 */
/***************************************************************************/
void displayStopAnimation(void) {

    if (p_currentDisplay != nullptr) {
        p_currentDisplay->setStopDrivingFlag();
    }
}


/***************************************************************************/
/**
 * @brief Cancel the current animation and bleach all segments.
 *
 * Clears the stop-driving flag, bleaches all segments on the current display
 * and executes the display update. Does nothing if no display is active.
 */
/***************************************************************************/
void displayCancelAnimation(void) {

    if (p_currentDisplay == nullptr) {
        return;
    }

    p_currentDisplay->clearStopDriving();        // Ensure driving is allowed again
    p_currentDisplay->setAllSegmentsBleach();    // Bleach all segments
    p_currentDisplay->executeDisplay();          // Apply the change on hardware
}


/***************************************************************************/
/**
 * @brief Initialize the negative 15-segment display.
 *
 * Marks tens digits for refresh on the next update and bleaches the extra
 * (minus) segment.
 */
/***************************************************************************/
void display15SegNegInit(void) {

    display15SegNegUpdateTens = true;                            // Force tens refresh on next run
    ecdEvalKit15SegNeg.setSegmentState(0, SEGMENT_STATE_BLEACH); // Extra segment OFF (BLEACH)
}


/***************************************************************************/
/**
 * @brief Display a two-digit number on the negative 15-seg display.
 *
 * Handles tens/units updates efficiently and manages the minus segment.
 * Tens are only refreshed when crossing boundaries (e.g. 09->10, 10->09),
 * to minimize unnecessary transitions.
 *
 * @param number Unsigned integer to display (0–99).
 * @param minus  Boolean indicating the minus segment state.
 */
/***************************************************************************/
void display15SegNegRun(unsigned int number, bool minus) {

    unsigned int tensDigit  = number / 10;
    unsigned int unitsDigit = number % 10;

    // Detect ascending or descending sequences that require tens refresh:
    //  - units: 9 -> 0 and tens increments (or wraps 9->0)
    //  - units: 0 -> 9 and tens decrements (or wraps 0->9)
    if ( (unitsDigit == 0 && last15SegNumber.unitsDigit == 9 &&
          (tensDigit == last15SegNumber.tensDigit + 1 ||
          (last15SegNumber.tensDigit == 9 && tensDigit == 0))) ||
         (unitsDigit == 9 && last15SegNumber.unitsDigit == 0 &&
          (tensDigit == last15SegNumber.tensDigit - 1 ||
          (last15SegNumber.tensDigit == 0 && tensDigit == 9))) )
    {
        display15SegNegUpdateTens = true;
    }

    p_currentDisplay = &ecdEvalKit15SegNeg;  // Select active display backend

    EK_15Seg_Struct_t displayMask {};
    displayMask.extra = minus;                                       // Minus sign segment
    displayMask.tens  = (bool*)mask7SegDotsDisplay[tensDigit];       // Tens digit mask
    displayMask.units = (bool*)mask7SegDotsDisplay[unitsDigit];      // Units digit mask

    // First step: bleach tens and units segments (avoid ghosting)
    for (int i = 0; i < 7; ++i) {
        if (display15SegNegUpdateTens) {
            ecdEvalKit15SegNeg.setSegmentState(i + 1, SEGMENT_STATE_BLEACH); // Tens
        }
        ecdEvalKit15SegNeg.setSegmentState(i + 8, SEGMENT_STATE_BLEACH);     // Units
    }
    ecdEvalKit15SegNeg.executeDisplay();
    
    // Set extra (minus) segment according to "minus" flag
    ecdEvalKit15SegNeg.setSegmentState(0, displayMask.extra);

    // Apply new tens and units segments according to mask
    for (int i = 0; i < 7; ++i) {
        if (display15SegNegUpdateTens) {
            ecdEvalKit15SegNeg.setSegmentState(i + 1, displayMask.tens[i]);
        }
        ecdEvalKit15SegNeg.setSegmentState(i + 8, displayMask.units[i]);
    }

    // Update hardware with new segment states
    ecdEvalKit15SegNeg.executeDisplay();
    
    // Store last displayed number for future sequence detection
    last15SegNumber.tensDigit  = tensDigit;
    last15SegNumber.unitsDigit = unitsDigit;
    display15SegNegUpdateTens  = false;
}


/***************************************************************************/
/**
 * @brief Initialize the dot 15-segment display.
 *
 * Marks tens digits for refresh on the next update and bleaches the dot segment.
 */
/***************************************************************************/
void display15SegDotInit(void) {

    display15SegDotUpdateTens = true;                            // Force tens refresh on next run
    ecdEvalKit15SegDot.setSegmentState(0, SEGMENT_STATE_BLEACH); // Extra (dot) segment OFF
}


/***************************************************************************/
/**
 * @brief Display a two-digit number on the 15-seg display with middle dot.
 *
 * Similar to display15SegNegRun(), but the extra segment represents a dot
 * instead of a minus sign.
 *
 * @param number Unsigned integer to display (0–99).
 * @param dot    Boolean indicating the dot segment state.
 */
/***************************************************************************/
void display15SegDotRun(unsigned int number, bool dot) {

    unsigned int tensDigit  = number / 10;
    unsigned int unitsDigit = number % 10;
    
    // Detect ascending/descending rollovers that require tens refresh
    if ( (unitsDigit == 0 && last15SegNumber.unitsDigit == 9 &&
          (tensDigit == last15SegNumber.tensDigit + 1 ||
          (last15SegNumber.tensDigit == 9 && tensDigit == 0))) ||
         (unitsDigit == 9 && last15SegNumber.unitsDigit == 0 &&
          (tensDigit == last15SegNumber.tensDigit - 1 ||
          (last15SegNumber.tensDigit == 0 && tensDigit == 9))) )
    {
        display15SegDotUpdateTens = true;
    }
    
    p_currentDisplay = &ecdEvalKit15SegDot;

    EK_15Seg_Struct_t displayMask {};
    displayMask.extra = dot;                                      // Dot segment state
    displayMask.tens  = (bool*)mask7SegDotsDisplay[tensDigit];    // Tens digit mask
    displayMask.units = (bool*)mask7SegDotsDisplay[unitsDigit];   // Units digit mask
    
    // First step: bleach tens and units segments (avoid ghosting)
    for (int i = 0; i < 7; ++i) {
        if (display15SegDotUpdateTens) {
            ecdEvalKit15SegDot.setSegmentState(i + 1, SEGMENT_STATE_BLEACH); // Tens
        }
        ecdEvalKit15SegDot.setSegmentState(i + 8, SEGMENT_STATE_BLEACH);     // Units
    }

    // On tens rollover, also bleach the dot so everything turns OFF together
    if (display15SegDotUpdateTens) {
        ecdEvalKit15SegDot.setSegmentState(0, SEGMENT_STATE_BLEACH);         // Dot
    }

    ecdEvalKit15SegDot.executeDisplay();
    
    // Apply dot/extra segment for the new value
    ecdEvalKit15SegDot.setSegmentState(0, displayMask.extra);

    // Apply new tens and units segments
    for (int i = 0; i < 7; ++i) {
        if (display15SegDotUpdateTens) {
            ecdEvalKit15SegDot.setSegmentState(i + 1, displayMask.tens[i]);
        }
        ecdEvalKit15SegDot.setSegmentState(i + 8, displayMask.units[i]);
    }

    ecdEvalKit15SegDot.executeDisplay();
    
    last15SegNumber.tensDigit  = tensDigit;
    last15SegNumber.unitsDigit = unitsDigit;
    display15SegDotUpdateTens  = false;
}


/***************************************************************************/
/**
 * @brief Set the state of the single segment display.
 *
 * @param state Segment state (true = COLOR, false = BLEACH).
 */
/***************************************************************************/
void displaySingleSet(bool state) {

    p_currentDisplay = &ecdEvalKitSingle;

    ecdEvalKitSingle.setSegmentState(0, state);
    ecdEvalKitSingle.executeDisplay();
}


/***************************************************************************/
/**
 * @brief Display a digit on a 7-seg with dot display.
 *
 * Applies the mask for the requested digit and sets the dot state. Index 3
 * is treated as the dot segment and handled separately.
 *
 * @param number Digit to display (0–9) or 10 for all segments OFF.
 * @param dot    Dot segment state.
 */
/***************************************************************************/
void display7SegDotRun(unsigned int number, bool dot) {

    uint8_t mask_iterator = 0;
    p_currentDisplay = &ecdEvalKit7SegDot;

    // Only process known digit masks
    if (number < EVAL_KIT_7SEG_DOT_MASK_NUM_OF_ANIMATIONS) {

        // First clear all segments
        for (int i = 0; i < EVAL_KIT_7SEG_DOT_NUM_SEGMENTS; ++i) {
            ecdEvalKit7SegDot.setSegmentState(i, 0);
        }
        ecdEvalKit7SegDot.executeDisplay();

        // Apply mask to all segments except index 3 (dot)
        for (int i = 0; i < EVAL_KIT_7SEG_DOT_NUM_SEGMENTS; ++i) {
            if (i != 3) {
                ecdEvalKit7SegDot.setSegmentState(i, mask7SegDotsDisplay[number][mask_iterator]);
                mask_iterator++;
            }
        }
    }

    // Set dot segment separately
    ecdEvalKit7SegDot.setSegmentState(3, dot);
    ecdEvalKit7SegDot.executeDisplay();
}


/***************************************************************************/
/**
 * @brief Set a single bar segment on the 7-bar display.
 *
 * @param segment Index of the bar segment to control.
 * @param state   Segment state (true = COLOR, false = BLEACH).
 */
/***************************************************************************/
void display7BarsSet(unsigned int segment, bool state) {

    p_currentDisplay = &ecdEvalKit7Bars;

    ecdEvalKit7Bars.setSegmentState(segment, state);
    ecdEvalKit7Bars.executeDisplay();
}


/***************************************************************************/
/**
 * @brief Clear all segments on the 7-bar display (bleach all).
 */
/***************************************************************************/
void display7BarsClear(void) {

    p_currentDisplay = &ecdEvalKit7Bars;

    ecdEvalKit7Bars.setAllSegmentsBleach();
    ecdEvalKit7Bars.executeDisplay();
}


/***************************************************************************/
/**
 * @brief Set a single bar segment on the 3-bar display.
 *
 * @param segment Index of the bar segment to control.
 * @param state   Segment state (true = COLOR, false = BLEACH).
 */
/***************************************************************************/
void display3BarsSet(unsigned int segment, bool state) {

    p_currentDisplay = &ecdEvalKit3Bars;

    ecdEvalKit3Bars.setSegmentState(segment, state);
    ecdEvalKit3Bars.executeDisplay();
}


/***************************************************************************/
/**
 * @brief Clear all segments on the 3-bar display (bleach all).
 */
/***************************************************************************/
void display3BarsClear(void) {

    p_currentDisplay = &ecdEvalKit3Bars;

    ecdEvalKit3Bars.setAllSegmentsBleach();
    ecdEvalKit3Bars.executeDisplay();
}


/***************************************************************************/
/**
 * @brief Direct-drive all pins of the 15-seg negative display.
 *
 * This helper bypasses the normal ECD state machine and forces all segment
 * pins to a given logic state for a fixed duration, using a fixed CE level.
 * Use carefully, as it ignores OCP and refresh logic.
 *
 * @param state     Segment logic state (true = HIGH, false = LOW).
 * @param driveTime Duration in milliseconds to hold the driven state.
 */
/***************************************************************************/
void displayDirectSetAll(bool state, uint16_t driveTime) {

    p_currentDisplay = &ecdEvalKit15SegNeg;

    // Set CE according to requested state
    if (state) {
        // COLOR-like drive: CE below WE
        ecdEvalKit15SegNeg.enableCounterElectrode(SUPPLY_VOLTAGE - REFRESH_COLORING_VOLTAGE);
    } else {
        // BLEACH-like drive: CE above WE
        ecdEvalKit15SegNeg.enableCounterElectrode(REFRESH_BLEACHING_VOLTAGE);
    }
    delay(10);  // Short settling time for CE and DAC

    // Force all segments to the requested state
    for (int i = 0; i < EVAL_KIT_15SEG_NEGATIVE_NUM_SEGMENTS; i++) {
        pinMode(evalKit15SegNegPinList[i], OUTPUT);
        digitalWrite(evalKit15SegNegPinList[i], state);
    }

    // Hold state for requested time
    delay(driveTime);

    // Return all segments to High-Z
    for (int i = 0; i < EVAL_KIT_15SEG_NEGATIVE_NUM_SEGMENTS; i++) {
        pinMode(evalKit15SegNegPinList[i], INPUT);
    }
    
    // Release CE to High-Z
    ecdEvalKit15SegNeg.disableCounterElectrode();
    delay(10);   // Small guard delay after disabling CE
}


/***************************************************************************
 ****************************** END OF FILE ********************************
 ***************************************************************************/
