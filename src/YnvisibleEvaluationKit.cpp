#include "YnvisibleECD.h"
#include "YnvisibleEvaluationKit.h"

int evalKitSinglePinList = EVAL_KIT_SINGLE_PIN_LIST;
int evalKit7SegDotPinList[EVAL_KIT_7SEG_DOT_NUM_SEGMENTS] = EVAL_KIT_7SEG_DOT_PIN_LIST;
int evalKit15SegNegPinList[EVAL_KIT_15SEG_NEGATIVE_NUM_SEGMENTS] = EVAL_KIT_15SEG_NEGATIVE_PIN_LIST;
int evalKit15SegDotPinList[EVAL_KIT_15SEG_DOT_NUM_SEGMENTS] = EVAL_KIT_15SEG_DOT_PIN_LIST;
int evalKit3BarsPinList[EVAL_KIT_3BARS_NUM_SEGMENTS] = EVAL_KIT_3BARS_PIN_LIST;
int evalKit7BarsPinList[EVAL_KIT_7BARS_NUM_SEGMENTS] = EVAL_KIT_7BARS_PIN_LIST;

static YNV_ECD * p_currentDisplay;
bool YNV_ECD::m_stopDrivingFlag = false;

YNV_ECD ecdEvalKitSingle(EVAL_KIT_SINGLE_NUM_SEGMENTS, &evalKitSinglePinList);                   // Object for a Single Segment Electrochromic Display
YNV_ECD ecdEvalKit7SegDot(EVAL_KIT_7SEG_DOT_NUM_SEGMENTS, evalKit7SegDotPinList);                         // Object for a 7-Segment Electrochromic Display
YNV_ECD ecdEvalKit15SegNeg(EVAL_KIT_15SEG_NEGATIVE_NUM_SEGMENTS, evalKit15SegNegPinList);      // Object for a Double 7-Segment with Minus sign Electrochromic Display
YNV_ECD ecdEvalKit15SegDot(EVAL_KIT_15SEG_DOT_NUM_SEGMENTS, evalKit15SegDotPinList);            // Object for a Double 7-Segment with a decimal Dot Electrochromic Display
YNV_ECD ecdEvalKit3Bars(EVAL_KIT_3BARS_NUM_SEGMENTS, evalKit3BarsPinList);                      // Object for a 3 Bars (Segments) Electrochromic Display -> Bottom to Top
YNV_ECD ecdEvalKit7Bars(EVAL_KIT_7BARS_NUM_SEGMENTS, evalKit7BarsPinList);                      // Object for a 7 Bars (Segments) Electrochromic Display -> Bottom to Top

EK_15Seg_Values_t last15SegNumber {0, 0};
bool display15SegDotUpdateTens = false;
bool display15SegNegUpdateTens = false;

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

void evaluationKitInit(void){
    // Configuration for 3 Bars Display
    ECD_Config evalKit3BarsConfig;
    evalKit3BarsConfig.coloringTime                 = 1200;
    evalKit3BarsConfig.bleachingTime                = 900;

    evalKit3BarsConfig.refreshColoringVoltage       = 1.3;
    evalKit3BarsConfig.refreshColorPulseTime        = 300;

    evalKit3BarsConfig.refreshBleachingVoltage      = 0.8;
    evalKit3BarsConfig.refreshBleachPulseTime       = 100;
    ecdEvalKit3Bars.setConfig(evalKit3BarsConfig);
 
    // Configuration for Single Segment Kit
    ECD_Config evalKitSingleConfig;
    evalKitSingleConfig.coloringTime                = 450;
    evalKitSingleConfig.bleachingTime               = 250;

    evalKitSingleConfig.refreshColoringVoltage      = 1.3;
    evalKitSingleConfig.refreshColorPulseTime       = 300;
    evalKitSingleConfig.refreshBleachingVoltage     = 0.9;
    evalKitSingleConfig.refreshBleachPulseTime      = 75;

    evalKitSingleConfig.refreshBleachLimitLVoltage  = 0.44;
    
    ecdEvalKitSingle.setConfig(evalKitSingleConfig);

    // Configuration for 15 Segment Displays
    ECD_Config evalKit15SegConfig;
    evalKit15SegConfig.refreshColoringVoltage       = 1.3;
    evalKit15SegConfig.refreshColorLimitHVoltage    = 1.0;
    evalKit15SegConfig.refreshBleachPulseTime       = 75;
    evalKit15SegConfig.refreshBleachLimitLVoltage   = 0.3;     // Bleach Limit Low [V]
    ecdEvalKit15SegNeg.setConfig(evalKit15SegConfig);
    ecdEvalKit15SegDot.setConfig(evalKit15SegConfig);

    // Configuration for 7 Bars Display
    ECD_Config evalKit7BarsConfig;
    evalKit7BarsConfig.refreshBleachLimitLVoltage   = 0.4;
    evalKit7BarsConfig.bleachingTime                = 700;
    ecdEvalKit7Bars.setConfig(evalKit7BarsConfig);
}


/**
 * Set the flag to stop the driving of the display
 */
void displayStopAnimation(void){
    p_currentDisplay->setStopDrivingFlag();
}

/**
 * Cancel the current animation and turn off all segments
 */
void displayCancelAnimation(void){

    p_currentDisplay->clearStopDriving();

    // Set All Segments to bleach - call this to prevent bleaching in inexistent segments
    p_currentDisplay->setAllSegmentsBleach();

    p_currentDisplay->executeDisplay();

}

void display15SegNegInit(void){
    display15SegNegUpdateTens = true;
    ecdEvalKit15SegNeg.setSegmentState(0,  SEGMENT_STATE_BLEACH);
}
/**
 * @brief Displays a two-digit number on a double 7-segment display.
 *
 * This function takes an unsigned integer number and a boolean indicating if the number is negative,
 * and displays the number on a double 7-segment display. The tens and units digits are extracted from
 * the number and displayed on the respective segments of the display.
 *
 * @param number The unsigned integer number to be displayed (0-99).
 * @param minus A boolean indicating the Minus segment state.
 */
void display15SegNegRun(unsigned int number, bool minus){
    unsigned int tensDigit = number / 10;
    unsigned int unitsDigit = number % 10;

    // Need to check all of this because we don't know if the function call comes from the same sequence or not
    if( (unitsDigit == 0 && last15SegNumber.unitsDigit == 9 && (tensDigit == last15SegNumber.tensDigit+1 || (last15SegNumber.tensDigit == 9 && tensDigit == 0))) ||          //If the units go from 9 to 0 and the tens increase by 1 or loop back from 9 to 0, the sequence is ascending and the tens need to refresh.
        (unitsDigit == 9 && last15SegNumber.unitsDigit == 0 && (tensDigit == last15SegNumber.tensDigit-1 || (last15SegNumber.tensDigit == 0 && tensDigit == 9))) )           //If the units go from 0 to 9 and the tens decrease by 1 or loop back from 0 to 9, the sequence is descending and the tens need to refresh.
        {
            display15SegNegUpdateTens = true;
        }

    p_currentDisplay = &ecdEvalKit15SegNeg;

    EK_15Seg_Struct_t displayMask {};
    
    displayMask.extra = minus;
    displayMask.tens = (bool*)mask7SegDotsDisplay[tensDigit];
    displayMask.units = (bool*)mask7SegDotsDisplay[unitsDigit];

    // Set Tens number Segments
    for (int i = 0; i < 7; ++i) {
        if(display15SegNegUpdateTens){
            ecdEvalKit15SegNeg.setSegmentState(i+1, SEGMENT_STATE_BLEACH);
        }
        ecdEvalKit15SegNeg.setSegmentState(i+8, SEGMENT_STATE_BLEACH);
    }
    ecdEvalKit15SegNeg.executeDisplay();
    
    // Set Extra Segment
    ecdEvalKit15SegNeg.setSegmentState(0,  displayMask.extra);

    for (int i = 0; i < 7; ++i) {
        if(display15SegNegUpdateTens){
            ecdEvalKit15SegNeg.setSegmentState(i+1, displayMask.tens[i]);
        }
        ecdEvalKit15SegNeg.setSegmentState(i+8, displayMask.units[i]);
    }

    //Execute Display
    ecdEvalKit15SegNeg.executeDisplay();
    
    last15SegNumber.tensDigit = tensDigit;
    last15SegNumber.unitsDigit = unitsDigit;
    display15SegNegUpdateTens = false;
}

void display15SegDotInit(void){
    display15SegDotUpdateTens = true;
    ecdEvalKit15SegDot.setSegmentState(0,  SEGMENT_STATE_BLEACH);
}

/**
 * @brief Displays a two-digit number on a double 7-segment display with a dot in the middle.
 *
 * This function takes an unsigned integer number and a boolean indicating if the number is negative,
 * and displays the number on a double 7-segment display. The tens and units digits are extracted from
 * the number and displayed on the respective segments of the display. A dot is displayed in the middle
 * of the display.
 *
 * @param number The unsigned integer number to be displayed (0-99).
 * @param dot A boolean indicating the Dot segment state.
 */
void display15SegDotRun(unsigned int number, bool dot){
    unsigned int tensDigit = number / 10;
    unsigned int unitsDigit = number % 10;
    
    // Need to check all of this because we don't know if the function call comes from the same sequence or not
    if( (unitsDigit == 0 && last15SegNumber.unitsDigit == 9 && (tensDigit == last15SegNumber.tensDigit+1 || (last15SegNumber.tensDigit == 9 && tensDigit == 0))) ||          //If the units go from 9 to 0 and the tens increase by 1 or loop back from 9 to 0, the sequence is ascending and the tens need to refresh.
        (unitsDigit == 9 && last15SegNumber.unitsDigit == 0 && (tensDigit == last15SegNumber.tensDigit-1 || (last15SegNumber.tensDigit == 0 && tensDigit == 9))) )           //If the units go from 0 to 9 and the tens decrease by 1 or loop back from 0 to 9, the sequence is descending and the tens need to refresh.
        {
            display15SegDotUpdateTens = true;
        }
    
    p_currentDisplay = &ecdEvalKit15SegDot;

    EK_15Seg_Struct_t displayMask {};
    
    displayMask.extra = dot;
    displayMask.tens = (bool*)mask7SegDotsDisplay[tensDigit];
    displayMask.units = (bool*)mask7SegDotsDisplay[unitsDigit];
    
    
    // Set Tens number Segments
    for (int i = 0; i < 7; ++i) {
        if(display15SegDotUpdateTens){
            ecdEvalKit15SegDot.setSegmentState(i+1, SEGMENT_STATE_BLEACH);
        }
        ecdEvalKit15SegDot.setSegmentState(i+8, SEGMENT_STATE_BLEACH);
    }
    ecdEvalKit15SegDot.executeDisplay();
    
    // Set Extra Segment
    ecdEvalKit15SegDot.setSegmentState(0,  displayMask.extra);

    for (int i = 0; i < 7; ++i) {
        if(display15SegDotUpdateTens){
            ecdEvalKit15SegDot.setSegmentState(i+1, displayMask.tens[i]);
        }
        ecdEvalKit15SegDot.setSegmentState(i+8, displayMask.units[i]);
    }

    //Execute Display
    ecdEvalKit15SegDot.executeDisplay();
    
    last15SegNumber.tensDigit = tensDigit;
    last15SegNumber.unitsDigit = unitsDigit;
    display15SegDotUpdateTens = false;
}

/**
 * @brief Displays a single segment on a single segment display.
 * @param state The state of the segment.
 */
void displaySingleSet(bool state){
    p_currentDisplay = &ecdEvalKitSingle;

    ecdEvalKitSingle.setSegmentState(0, state);

    ecdEvalKitSingle.executeDisplay();
}

/**
 * @brief Displays a number on a 7-segment display.
 * @param number The number to be displayed.
 */
void display7SegDotRun(unsigned int number, bool dot){
    uint8_t mask_iterator = 0;
    p_currentDisplay = &ecdEvalKit7SegDot;


    if(number < EVAL_KIT_7SEG_DOT_MASK_NUM_OF_ANIMATIONS){
        for (int i = 0; i < EVAL_KIT_7SEG_DOT_NUM_SEGMENTS; ++i) {
                ecdEvalKit7SegDot.setSegmentState(i, 0);
        }
        
        ecdEvalKit7SegDot.executeDisplay();

        for (int i = 0; i < EVAL_KIT_7SEG_DOT_NUM_SEGMENTS; ++i) {
            if(i != 3){
                ecdEvalKit7SegDot.setSegmentState(i, mask7SegDotsDisplay[number][mask_iterator]);
                mask_iterator++;
            }
        }
    }
    ecdEvalKit7SegDot.setSegmentState(3, dot);
    
    ecdEvalKit7SegDot.executeDisplay();
}

/**
 * @brief Displays a number on a 7-bar display.
 * @param segment The segment to be set.
 * @param state The state of the segment.
 */
void display7BarsSet(unsigned int segment, bool state){
    p_currentDisplay = &ecdEvalKit7Bars;

    ecdEvalKit7Bars.setSegmentState(segment, state);
    
    ecdEvalKit7Bars.executeDisplay();
}

 /**
  * @brief Clears all segments on a 7-bar display.
  */
void display7BarsClear(void){
    p_currentDisplay = &ecdEvalKit7Bars;

    ecdEvalKit7Bars.setAllSegmentsBleach();

    ecdEvalKit7Bars.executeDisplay();
}

/**
 * @brief Displays a number on a 3-bar display.
 * @param segment The segment to be set.
 * @param state The state of the segment.
 */
void display3BarsSet(unsigned int segment, bool state){
    p_currentDisplay = &ecdEvalKit3Bars;

    ecdEvalKit3Bars.setSegmentState(segment, state);

    ecdEvalKit3Bars.executeDisplay();
}

/**
 * @brief Clears all segments on a 3-bar display.
 */
void display3BarsClear(void){
    p_currentDisplay = &ecdEvalKit3Bars;

    ecdEvalKit3Bars.setAllSegmentsBleach();

    ecdEvalKit3Bars.executeDisplay();
}

/**
 * @brief Direct drive all pins of a display.
 * @param state The state of the segments.
 * @param driveTime The time to drive the segments.
 */
void displayDirectSetAll(bool state, uint16_t driveTime){
    p_currentDisplay = &ecdEvalKit15SegNeg;

    if(state){
        ecdEvalKit15SegNeg.enableCounterElectrode(SUPPLY_VOLTAGE-REFRESH_COLORING_VOLTAGE);
    }else{
        ecdEvalKit15SegNeg.enableCounterElectrode(REFRESH_BLEACHING_VOLTAGE);
    }
    delay(10);

    for (int i = 0; i < EVAL_KIT_15SEG_NEGATIVE_NUM_SEGMENTS; i++)
    {
        pinMode(evalKit15SegNegPinList[i], OUTPUT);
        digitalWrite(evalKit15SegNegPinList[i], state);
    }

    delay(driveTime);

    for (int i = 0; i < EVAL_KIT_15SEG_NEGATIVE_NUM_SEGMENTS; i++)
    {
        pinMode(evalKit15SegNegPinList[i], INPUT);
    }
    
    ecdEvalKit15SegNeg.disableCounterElectrode();
    delay(10);
}