/*
	EvaluationKit.ino - Sketch for the Evaluation Kit running on the Driver v5 board
	Created by @BFFonseca - Ynvisible, May 2025
  Modifyed by JoCFMendes - Ynvisible, January 2026
	For Driver 5.x Hardware
*/

#include <Arduino.h>
#include <Wire.h>
#include "YnvisibleDriverV5.h"
#include "YnvisibleEvaluationKit.h"

unsigned int selectedAnimation = 0;
bool animationChanged = false;
bool runSelectedAnimation = false;
bool pauseAnimation = false;              // Pause the current animation.
bool cancelAnimation = false;             // abort the currently playing animation. Used by long press Start Button or by starting another animation while one is already on-going

bool directToggleState = false;

void setup() {
  // --------------- Main Power DC-DC ---------------
  pinMode(MCU_PWR_ON, OUTPUT);        // Keep the Board Power ON
  digitalWrite(MCU_PWR_ON, HIGH);
  
  // --------------- RGB LED Setup ---------------
  pinMode(LED_R, OUTPUT);
  digitalWrite(LED_R, HIGH);
  
  pinMode(LED_G, OUTPUT);
  digitalWrite(LED_G, HIGH);

  pinMode(LED_B, OUTPUT);
  digitalWrite(LED_B, LOW);           // RGB Blue ON

  // --------------- Control Buttons Setup ---------------
  pinMode(BTN_START, INPUT);
  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);

  attachInterrupt(digitalPinToInterrupt(BTN_START), buttonStartPressedISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(BTN_UP), buttonUpPressedISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_DOWN), buttonDownPressedISR, RISING);
  interrupts();

  greenLEDsInit();
  updateAnimationLEDs(selectedAnimation);

  runSelectedAnimation = false;
  evaluationKitInit();
}

void loop() {
  
  if(animationChanged){
    updateAnimationLEDs(selectedAnimation);
    animationChanged = false;
  }

  if(runSelectedAnimation){
    digitalWrite(LED_B, HIGH);      // RGB Blue OFF
    digitalWrite(LED_G, LOW);       // RGB Green ON
    
    switch(selectedAnimation){
      case EVAL_ANIMATION_DIRECT_TOGGLE:
        displayDirectToggle();
      break;
      case EVAL_ANIMATION_15SEG_NEGATIVE_POS_UP:
        animation15SegPositiveUp();
      break;
      case EVAL_ANIMATION_15SEG_NEGATIVE_POS_DOWN:
        animation15SegPositiveDown();
      break;
      case EVAL_ANIMATION_15SEG_NEGATIVE_NEG_UP:
        animation15SegNegativeUp();
      break;
      case EVAL_ANIMATION_15SEG_NEGATIVE_NEG_DOWN:
        animation15SegNegativeDown();
      break;
      case EVAL_ANIMATION_15SEG_DOT_UP:
        animation15SegDotUp();
      break;
      case EVAL_ANIMATION_15SEG_DOT_DOWN:
        animation15SegDotDown();
      break;
      case EVAL_ANIMATION_SINGLE_ON:
        animationSingleOn();
      break;
      case EVAL_ANIMATION_7SEG_DOT_COUNT_UP:
        animation7SegDotCountUp();
      break;
      case EVAL_ANIMATION_7SEG_DOT_COUNT_DOWN:
        animation7SegDotCountDown();
      break;
      case EVAL_ANIMATION_7BARS_COUNT_UP:
        animation7BarsCountUp();
      break;
      case EVAL_ANIMATION_7BARS_COUNT_DOWN:
        animation7BarsCountDown();
      break;
      case EVAL_ANIMATION_3BARS_COUNT_UP:
        animation3BarsCountUp();
      break;
      case EVAL_ANIMATION_3BARS_COUNT_DOWN:
        animation3BarsCountDown();
      break;
      case EVAL_ANIMATION_3BARS_MID_TOP_BOT:
        animation3BarsMidTopBot();
      break;
      default:
        digitalWrite(LED_G, HIGH);      // RGB Green OFF
        for(int i = 0; i < 3; i++){
          digitalWrite(LED_R, LOW);
          delay(500);
          digitalWrite(LED_R, HIGH);
          delay(500);
        }
        runSelectedAnimation = false;
        pauseAnimation = false;
      break;
    }
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, HIGH);          // RGB Green OFF
  }
  checkAndCancelCurrentAnimation();
}

/**
 * Checks if the animation was canceled and stops the current animation
 */
void checkAndCancelCurrentAnimation(void){
  if(cancelAnimation){
      displayCancelAnimation();
    digitalWrite(LED_B, LOW);           // RGB Blue ON
    pauseAnimation = false;
    runSelectedAnimation = false;
    cancelAnimation = false;
    return;
  }
  while(pauseAnimation){
    digitalWrite(LED_G, HIGH);
    delay(DRIVER_ANIMATION_DELAY_PAUSE);
    digitalWrite(LED_G, LOW);
    delay(DRIVER_ANIMATION_DELAY_PAUSE);
  }
}

/**
 * Check if the animation is canceled
 * @returns TRUE: animation is canceled \
 * @returns FALSE: animation was not canceled
 */
bool isAnimationCanceled(void){
  if(cancelAnimation){
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_R, LOW);
    return true;
  }
  while(pauseAnimation){
    digitalWrite(LED_G, HIGH);
    delay(DRIVER_ANIMATION_DELAY_PAUSE);
    digitalWrite(LED_G, LOW);
    delay(DRIVER_ANIMATION_DELAY_PAUSE);
  }
  return false;
}

/**
 * Handle the Button Start press Interrupt
 */
void buttonStartPressedISR(void){
  static long lastStartPress = 0;                 // Stores the time at which the last press/depress on a button was made. Used to debounce the buttons.
  static long lastStartPressHigh = 0;             // Stores the time at which the last press on a button was made. Used for Long Press check.
  long currentTime = millis();
  
  if ((currentTime - lastStartPress) <= DRIVER_BUTTON_DEBOUNCE_MS) {   // avoids button bouncing
    return;
  }

  lastStartPress = currentTime;

  if(digitalRead(BTN_START) == LOW){
    lastStartPressHigh = currentTime;
  }
  else if(currentTime - lastStartPressHigh < DRIVER_BUTTON_LONG_PRESS_MS){     // Short Press
    if(runSelectedAnimation){
      pauseAnimation = !pauseAnimation;
      return;
    }
    runSelectedAnimation = true;                       // Start/Stop the animation
  }
  else if(runSelectedAnimation){                       // Long Press and an animation is running
    pauseAnimation = false;
    cancelAnimation = true;
    displayStopAnimation();
  }
}

/**
 * Handle the Button Up press Interrupt
 */
void buttonUpPressedISR(void){
  static long lastUpPress = 0;                // Stores the time at which the last press on a button was made. Used to debounce the buttons.
  long currentTime = millis();
  
  if ((currentTime - lastUpPress) <= DRIVER_BUTTON_DEBOUNCE_MS || runSelectedAnimation) {   // avoids button bouncing and changing animations while running one
    return;
  }
  lastUpPress = currentTime;

  animationChanged = true;
  if(selectedAnimation == EVAL_KIT_NUM_ANIMATIONS-1){   // As 0 doesn't count, we have have EVAL_KIT_NUM_ANIMATION equal to the actual number of animations
    selectedAnimation = 0;                              // 1 is the first animation, 0 does not exist in the context of LEDs or displays (0 = SIGN_ANIMATION_NONE)
    return;
  }
  selectedAnimation++;
}

/**
 * Handle the Button Down press Interrupt
 */
void buttonDownPressedISR(void){
  static long lastDownPress = 0;                // Stores the time at which the last press on a button was made. Used to debounce the buttons.
  long currentTime = millis();
  
  if ((currentTime - lastDownPress) <= DRIVER_BUTTON_DEBOUNCE_MS || runSelectedAnimation) {   // avoids button bouncing and changing animations while running one
    return;
  }
  lastDownPress = currentTime;
  
  animationChanged = true;
  if(selectedAnimation == 0){                         // 1 is the first animation, 0 does not exist in the context of LEDs or displays (0 = SIGN_ANIMATION_NONE)
    selectedAnimation = EVAL_KIT_NUM_ANIMATIONS-1;    // As 0 doesn't count, we have have EVAL_KIT_NUM_ANIMATION equal to the actual number of animations
    return;
  }
  selectedAnimation--;
}


/******************************************************************************
 *                              ANIMATIONS                                    *
 ******************************************************************************/

/**
 * Calculates delay after the display runs the animation.
 * @details:  Assumes the animation is looping without any other delays and uses the last 
 *            recorded value inside this function as a refference for calculating the 
 *            remaining delay.
 * @param animationDelay desired animation delay. This is used to calculate the remaining 
 *  	                  delay duration, based on the elapsed time during display run.
 */
void delayAfterDisplayRun(unsigned int animationDelay){
  static uint64_t lastTime = millis();

  uint64_t timePassed = millis() - lastTime;
  
  if(timePassed < animationDelay){
    delay(animationDelay-timePassed);
  }
  lastTime = millis();
}

/**
 * Toggle any display with direct driving
 */
void displayDirectToggle(void){
  directToggleState = !directToggleState;
  displayDirectSetAll(directToggleState, EVAL_KIT_DIRECT_TOGGLE_DELAY);
  runSelectedAnimation = false;
  digitalWrite(LED_B, LOW);     // RGB Blue ON
  
}

/****************** Double 7 Segments Display (15 Segments) *******************/
/**
 * Counts up from 0 to 99 with the Double 7 Segments Display
 */
void animation15SegPositiveUp(void){
  display15SegNegInit();
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 0; i <= 99; i++){
    if(isAnimationCanceled()){
      return;
    }
    display15SegNegRun(i, false);
    delayAfterDisplayRun(EVAL_KIT_15SEG_COUNT_DELAY);
  }
}

/**
 * Counts down from 99 to 0 with the Double 7 Segments Display
 */
void animation15SegPositiveDown(void){
  display15SegNegInit();
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 99; i >= 0; i--){
    if(isAnimationCanceled()){
      return;
    }
    display15SegNegRun(i, false);
    delayAfterDisplayRun(EVAL_KIT_15SEG_COUNT_DELAY);
  }
}

/**
 * Counts up from -99 to 0 with the Double 7 Segments Display
 */
void animation15SegNegativeUp(void){
  display15SegNegInit();
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 99; i >= 0; i--){
    if(isAnimationCanceled()){
      return;
    }
    display15SegNegRun(i, true);
    delayAfterDisplayRun(EVAL_KIT_15SEG_COUNT_DELAY);
  }
}

/**
 * Counts down from 0 to -99 with the Double 7 Segments Display
 */
void animation15SegNegativeDown(void){
  display15SegNegInit();
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 0; i <= 99; i++){
    if(isAnimationCanceled()){
      return;
    }
    display15SegNegRun(i, true);
    delayAfterDisplayRun(EVAL_KIT_15SEG_COUNT_DELAY);
  }
}

/**
 * Counts up from 0 to 99 with the Double 7 Segments Display
 * and a dot in the middle
 */
void animation15SegDotUp(void){
  display15SegDotInit();
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 0; i <= 99; i++){
    if(isAnimationCanceled()){
      return;
    }
    display15SegDotRun(i, true);
    delayAfterDisplayRun(EVAL_KIT_15SEG_COUNT_DELAY);
  }
}

/**
 * Counts down from 99 to 0 with the Double 7 Segments Display
 * and a dot in the middle
 */
void animation15SegDotDown(void){
  display15SegDotInit();
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 99; i >= 0; i--){
    if(isAnimationCanceled()){
      return;
    }
    display15SegDotRun(i, true);
    delayAfterDisplayRun(EVAL_KIT_15SEG_COUNT_DELAY);
  }
}

/************************** Single Segment Display ****************************/
/**
 * Turn ON for some time and then turn Off the Single Segment Display
 */
void animationSingleOn(void){
  displaySingleSet(SEGMENT_STATE_COLOR);
  if(isAnimationCanceled()){
    return;
  }
  
  delayAfterDisplayRun(0);  // Reset timer
  delayAfterDisplayRun(EVAL_KIT_SINGLE_ON_TIME);
  displaySingleSet(SEGMENT_STATE_BLEACH);
  
  if(isAnimationCanceled()){
    return;
  }
  delayAfterDisplayRun(500);
}

/***************************** 7 Segment Display ******************************/
/**
 * Count up with the 7 segment display
 */
void animation7SegDotCountUp(void){
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 0; i <= 9; i++){
    if(isAnimationCanceled()){
      return;
    }
    display7SegDotRun(i, true);
    delayAfterDisplayRun(EVAL_KIT_7SEG_DOT_COUNT_DELAY);
  }
  display7SegDotRun(EVAL_KIT_7SEG_DOT_MASK_NUM_OF_ANIMATIONS, false);
}

/**
 * Count down with the 7 segment display
 */
void animation7SegDotCountDown(void){
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 9; i >= 0; i--){
    if(isAnimationCanceled()){
      return;
    }
    display7SegDotRun(i, true);
    delayAfterDisplayRun(EVAL_KIT_7SEG_DOT_COUNT_DELAY);
  }
  display7SegDotRun(EVAL_KIT_7SEG_DOT_MASK_NUM_OF_ANIMATIONS, false);
}

/******************************* 7 Bars Display *******************************/

/**
 * Count up with the 7 bars display
 */
void animation7BarsCountUp(void){
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 0; i < 7; i++){
    if(isAnimationCanceled()){
      return;
    }
    display7BarsSet(i, SEGMENT_STATE_COLOR);
    delayAfterDisplayRun(EVAL_KIT_7BAR_COUNT_DELAY);
  }
  display7BarsClear();
}

/**
 * Count down with the 7 bars display
 */
void animation7BarsCountDown(void){
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 6; i >= 0; i--){
    if(isAnimationCanceled()){
      return;
    }
    display7BarsSet(i, SEGMENT_STATE_COLOR);
    delayAfterDisplayRun(EVAL_KIT_7BAR_COUNT_DELAY);
  }
  display7BarsClear();
}

/******************************* 3 Bars Display *******************************/

/**
 * Count up with the 3 bars display
 */
void animation3BarsCountUp(void){
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 0; i < 3 ; i++){
    if(isAnimationCanceled()){
      return;
    }
    display3BarsSet(i, SEGMENT_STATE_COLOR);
    delayAfterDisplayRun(EVAL_KIT_3BAR_COUNT_DELAY);
  }
  delay(EVAL_KIT_3BAR_COUNT_DELAY);
  delay(EVAL_KIT_3BAR_COUNT_DELAY);


  for(int i = 0; i < 3 ; i++){
    if(isAnimationCanceled()){
      return;
    }
    display3BarsSet(i, SEGMENT_STATE_BLEACH);
  }
  delay(EVAL_KIT_3BAR_COUNT_DELAY);
}

/**
 * Count down with the 3 bars display
 */
void animation3BarsCountDown(void){
  delayAfterDisplayRun(0);  // Reset timer
  for(int i = 2; i >= 0; i--){
    if(isAnimationCanceled()){
      return;
    }
    display3BarsSet(i, SEGMENT_STATE_COLOR);
    delayAfterDisplayRun(EVAL_KIT_3BAR_COUNT_DELAY);
  }
  for(int i = 2; i >= 0; i--){
    if(isAnimationCanceled()){
      return;
    }
    display3BarsSet(i, SEGMENT_STATE_BLEACH);
  }
  delay(EVAL_KIT_3BAR_COUNT_DELAY);
}

/**
 * Turn ON the 3 bars display in the following order:
 *  1. Middle Segment
 *  2. Top Segment
 *  3. Bottom Segment
 */
void animation3BarsMidTopBot(void){
  delayAfterDisplayRun(0);  // Reset timer
  display3BarsSet(1, SEGMENT_STATE_COLOR);
  delayAfterDisplayRun(EVAL_KIT_3BAR_COUNT_DELAY);

  display3BarsSet(2, SEGMENT_STATE_COLOR);
  delayAfterDisplayRun(EVAL_KIT_3BAR_COUNT_DELAY);

  display3BarsSet(0, SEGMENT_STATE_COLOR);
  delayAfterDisplayRun(EVAL_KIT_3BAR_COUNT_DELAY);  
  
  display3BarsSet(1, SEGMENT_STATE_BLEACH);
  display3BarsSet(0, SEGMENT_STATE_BLEACH);
  display3BarsSet(2, SEGMENT_STATE_BLEACH);
  delay(EVAL_KIT_3BAR_COUNT_DELAY);
}
