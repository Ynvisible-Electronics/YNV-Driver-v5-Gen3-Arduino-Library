/**
 * Driver v5 specific code for normal operation
 * Created by @BFFonseca - Ynvisible, May 2025
 */

#include <Arduino.h>
#include "YnvisibleECD.h"
#include "YnvisibleDriverV5.h"

const unsigned int greenLEDsPinList[7] = {LED_1, LED_2, LED_3, LED_4, LED_5, LED_6, LED_7};

/**
 * Green LED Animations Table
 * boolean matrix indicating the LEDs state for each Display animation.
 * LED is active LOW
 * LOW  (0) - LED ON
 * HIGH (1) - LED OFF
 */
const bool greenLEDsAnimationTable[28][7] = {
  {0, 1, 1, 1, 1, 1, 1}, // Animation 1
  {1, 0, 1, 1, 1, 1, 1}, // Animation 2
  {1, 1, 0, 1, 1, 1, 1}, // Animation 3
  {1, 1, 1, 0, 1, 1, 1}, // Animation 4
  {1, 1, 1, 1, 0, 1, 1}, // Animation 5
  {1, 1, 1, 1, 1, 0, 1}, // Animation 6
  {1, 1, 1, 1, 1, 1, 0}, // Animation 7
  {0, 0, 1, 1, 1, 1, 1}, // Animation 8
  {0, 1, 0, 1, 1, 1, 1}, // Animation 9
  {0, 1, 1, 0, 1, 1, 1}, // Animation 10
  {0, 1, 1, 1, 0, 1, 1}, // Animation 11
  {0, 1, 1, 1, 1, 0, 1}, // Animation 12
  {0, 1, 1, 1, 1, 1, 0}, // Animation 13
  {0, 0, 0, 1, 1, 1, 1}, // Animation 14
  {0, 0, 1, 0, 1, 1, 1}, // Animation 15
  {0, 0, 1, 1, 0, 1, 1}, // Animation 16
  {0, 0, 1, 1, 1, 0, 1}, // Animation 17
  {0, 0, 1, 1, 1, 1, 0}, // Animation 18
  {0, 0, 0, 0, 1, 1, 1}, // Animation 19
  {0, 0, 0, 1, 0, 1, 1}, // Animation 20
  {0, 0, 0, 1, 1, 0, 1}, // Animation 21
  {0, 0, 0, 1, 1, 1, 0}, // Animation 22
  {0, 0, 0, 0, 0, 1, 1}, // Animation 23
  {0, 0, 0, 0, 1, 0, 1}, // Animation 24
  {0, 0, 0, 0, 1, 1, 0}, // Animation 25
  {0, 0, 0, 0, 0, 0, 1}, // Animation 26
  {0, 0, 0, 0, 0, 1, 0}, // Animation 27
  {0, 0, 0, 0, 0, 0, 0}  // Animation 28
};

/**
 * @brief Turn ON all Green LEDs (L1 - L7)
 * 
 * @param t_delay ms - delay between each LED turning ON
 */
void greenLEDsAllOn(unsigned int t_delay){
  digitalWrite(LED_1, LOW);
  delay(t_delay);
  digitalWrite(LED_2, LOW);
  delay(t_delay);
  digitalWrite(LED_3, LOW);
  delay(t_delay);
  digitalWrite(LED_4, LOW);
  delay(t_delay);
  digitalWrite(LED_5, LOW);
  delay(t_delay);
  digitalWrite(LED_6, LOW);
  delay(t_delay);
  digitalWrite(LED_7, LOW);
  delay(t_delay);
}


/**
 * @brief Turn OFF all Green LEDs (L1 - L7)
 * 
 * @param t_delay ms - delay between each LED turning OFF
 */
void greenLEDsAllOff(unsigned int t_delay){
  digitalWrite(LED_7, HIGH);
  delay(t_delay);
  digitalWrite(LED_6, HIGH);
  delay(t_delay);
  digitalWrite(LED_5, HIGH);
  delay(t_delay);
  digitalWrite(LED_4, HIGH);
  delay(t_delay);
  digitalWrite(LED_3, HIGH);
  delay(t_delay);
  digitalWrite(LED_2, HIGH);
  delay(t_delay);
  digitalWrite(LED_1, HIGH);
  delay(t_delay);
}

/**
 * @brief Initialize the Green LEDs
 * 
 * Initialize the LEDs PINs and do an 
 * ON/OFF sequence with a specific delay 
 */
void greenLEDsInit(void){
  // --------------- Green LEDs Pin Setup ---------------
  for(int i = 0; i < 7; i++){
    pinMode(greenLEDsPinList[i], OUTPUT);
    digitalWrite(greenLEDsPinList[i], HIGH);
  }

  greenLEDsAllOn(DRIVER_BOOT_UP_SEQUENCE_DELAY/3);    // Turn all On
  delay(DRIVER_BOOT_UP_SEQUENCE_DELAY * 2);        // Wait a bit
  greenLEDsAllOff(DRIVER_BOOT_UP_SEQUENCE_DELAY);   // Turn all Off
}

/**
 * @brief Set the states of each LED based on the animation table
 * 
 * @param t_selectedAnimation Number of the currently active animation
 */
void updateAnimationLEDs(unsigned int t_selectedAnimation) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(greenLEDsPinList[i], greenLEDsAnimationTable[t_selectedAnimation][i]);  // Turn ON with LOW, OFF with HIGH
  }
}