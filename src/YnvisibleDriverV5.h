
/**
 * @file YnvisibleDriverV5.h
 * @brief Hardware abstraction and LED control interface for the Ynvisible Driver v5 board.
 *
 * This header provides the public API for controlling the 7 on‑board green LEDs
 * present on the Ynvisible Driver v5 hardware. These LEDs are used solely for
 * user‑feedback and animation purposes and are independent from all electrochromic
 * display driving operations.
 *
 * Responsibilities:
 *  - Expose LED control functions for ON/OFF sequencing and animation.
 *  - Provide boot‑up visual feedback using predefined LED patterns.
 *  - Allow external modules to trigger LED animations by index.
 *
 * Notes:
 *  - LEDs are active LOW: LOW = ON, HIGH = OFF.
 *  - The animation table and LED pin mapping are implemented in YnvisibleDriverV5.cpp.
 *  - Timing values (delays, debouncing, long‑press thresholds) are defined here.
 *
 * Created by @BFFonseca - Ynvisible (May 2025)
 * Updated by JoCFMendes - Ynvisible (2026)
 */

#ifndef YNVISIBLE_DRIVER_5_H
#define YNVISIBLE_DRIVER_5_H

#include "YnvisibleECD.h"
#include <Arduino.h>


// ---------------------------------------------------------------------------
// Driver v5 Configuration Parameters
// ---------------------------------------------------------------------------

/** @brief Delay (ms) between each LED operation during boot‑up sequence. */
#define DRIVER_BOOT_UP_SEQUENCE_DELAY   100

/** @brief Button debounce time (ms). */
#define DRIVER_BUTTON_DEBOUNCE_MS       50

/** @brief Minimum duration (ms) to identify a button long‑press. */
#define DRIVER_BUTTON_LONG_PRESS_MS     1000

/** @brief Delay (ms) between LED animation frames. */
#define DRIVER_ANIMATION_DELAY_PAUSE    500


// ---------------------------------------------------------------------------
// Public API Function Prototypes
// ---------------------------------------------------------------------------

/***************************************************************************/
/**
 * @brief Turn ON all LEDs sequentially (L1 → L7).
 *
 * LEDs are active LOW; each LED is turned ON using LOW, with an optional delay
 * between each step.
 *
 * @param t_delay Delay in milliseconds between each LED activation.
 */
/***************************************************************************/
void greenLEDsAllOn(unsigned int t_delay);


/***************************************************************************/
/**
 * @brief Turn OFF all LEDs sequentially (L7 → L1).
 *
 * LEDs are active LOW; turning OFF is performed using HIGH, with an optional
 * delay between each LED.
 *
 * @param t_delay Delay in milliseconds between each LED deactivation.
 */
/***************************************************************************/
void greenLEDsAllOff(unsigned int t_delay);


/***************************************************************************/
/**
 * @brief Execute the Driver v5 LED boot‑up animation.
 *
 * Initializes LED pins, turns all LEDs ON sequentially, waits, and then turns
 * them OFF in reverse order. Used as a visual indicator during system startup.
 */
/***************************************************************************/
void greenLEDsInit(void);


/***************************************************************************/
/**
 * @brief Update LED states according to the selected animation pattern.
 *
 * Applies a predefined LED animation frame (stored in the animation table)
 * to all green LEDs. LOW turns a given LED ON and HIGH turns it OFF.
 *
 * @param t_selectedAnimation Index of the animation frame to display.
 *
 * @note The animation index is passed as a parameter because global animation
 *       state may change asynchronously (e.g., via button input). Passing the
 *       value ensures deterministic and repeatable transitions.
 */
/***************************************************************************/
void updateAnimationLEDs(unsigned int t_selectedAnimation);

#endif  // YNVISIBLE_DRIVER_5_H

/***************************************************************************
 ****************************** END OF FILE ********************************
 ***************************************************************************/