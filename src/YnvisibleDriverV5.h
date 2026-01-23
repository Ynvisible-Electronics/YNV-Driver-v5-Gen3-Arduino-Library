#ifndef YNVISIBLE_DRIVER_5_H
#define YNVISIBLE_DRIVER_5_H

#include "YnvisibleECD.h"
#include <Arduino.h>

#define DRIVER_BOOT_UP_SEQUENCE_DELAY   100      // ms - delay between each LED operation for boot-up sequence

#define DRIVER_BUTTON_DEBOUNCE_MS       50
#define DRIVER_BUTTON_LONG_PRESS_MS     1000

#define DRIVER_ANIMATION_DELAY_PAUSE    500

/**
 * Turns ON all LEDs, from left to right
 * @param t_delay delay between each LED operation
 */
void greenLEDsAllOn(unsigned int t_delay);

/**
 * Turns OFF all LEDs, from right to left
 * @param t_delay delay between each LED operation
 */
void greenLEDsAllOff(unsigned int t_delay);

/**
 * Run the Boot Up green LEDs Sequence
 * @details
 * Turns ON leds from left to right with a delay and
 * turn them off from right to left with the same delay.
 */
void greenLEDsInit(void);

/**
 * Change the Green LEDs to match the currently selected animation.
 * @param t_selectedAnimation Currently selected animation
 * @note We get the selected animation as a parameter because the global variable can change
 * during function runtime (if a button is pressed) and it can cause weird behaviors.
 * This also makes sure we don't skip any animation.
 */
void updateAnimationLEDs(unsigned int t_selectedAnimation);

#endif  // YNVISIBLE_DRIVER_5_H