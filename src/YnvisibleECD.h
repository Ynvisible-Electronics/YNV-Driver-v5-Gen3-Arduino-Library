
/**
 * @file YnvisibleECD.h
 * @brief Public API for driving Ynvisible Electrochromic Displays using the Driver v5 hardware.
 *
 * This header defines the main interface for the Ynvisible ECD (Electrochromic 
 * Display) driver. It exposes the configuration structure, public methods,
 * enumerations, constants, and internal parameters required to drive individual
 * electrochromic segments on the Ynvisible Driver v5 platform.
 *
 * Responsibilities:
 *  - Provide high-level methods for coloring and bleaching display segments.
 *  - Handle OCP (Open-Circuit Potential) sampling and per-segment refresh logic.
 *  - Maintain amplitude thresholds and timing parameters for display operation.
 *  - Configure and control CE (Counter Electrode) voltage via DAC.
 *  - Abstract all low-level ADC, DAC, GPIO and refresh logic internally.
 *
 * Notes:
 *  - Internal logic operates in LSB for better precision and MCU-level safety.
 *  - Refresh thresholds depend on supply voltage and are recalculated dynamically.
 *  - This module is independent from LED animation code (YnvisibleDriverV5.*).
 *
 * Created by @BFFonseca - Ynvisible (Oct 2024)
 * Updated by JoCFMendes - Ynvisible (Jan 2026)
 */

#ifndef _YNVISIBLE_ECD
#define _YNVISIBLE_ECD

#include "Arduino.h"


// ---------------------------------------------------------------------------
// Static Configuration Macros
// ---------------------------------------------------------------------------

#define MAX_NUMBER_OF_SEGMENTS              15            // Max number of segment pins supported by the driver
#define MAX_REFRESH_RETRIES                 30            // Max number of refresh attempts before refresh is considered failed

#define SUPPLY_VOLTAGE                      3.0           // (V) MCU supply voltage used for DAC/ADC scaling
#define ADC_DAC_RESOLUTION                  10            // ADC/DAC resolution in bits (10-bit = 0..1023)
#define ADC_DAC_MAX_LSB                     1023          // Maximum LSB value for the configured ADC/DAC resolution
#define LSB_TO_VOLT_CONV                    0.0029325513f // Conversion factor from LSB to volts (SUPPLY_VOLTAGE / 1023)

#define REFRESH_COLOR_LIMIT_H_REL_AMP       1.1           // (V) High amplitude threshold for Color refresh detection
#define REFRESH_COLOR_LIMIT_L_REL_AMP       0.95          // (V) Low amplitude threshold for Color refresh detection

#define REFRESH_BLEACH_LIMIT_H_REL_AMP      0.3           // (V) High amplitude threshold for Bleach refresh detection
#define REFRESH_BLEACH_LIMIT_L_REL_AMP      0.5           // (V) Low amplitude threshold for Bleach refresh target

#define COLORING_VOLTAGE                    1.3           // (V) Pulse amplitude used during Color transition
#define REFRESH_COLORING_VOLTAGE            1.3           // (V) Pulse amplitude used during Color refresh
#define COLORING_TIME                       350           // (ms) Duration of Color transition pulse
#define REFRESH_COLOR_PULSE_TIME            100           // (ms) Duration of Color refresh pulse

#define BLEACHING_VOLTAGE                   0.7           // (V) Pulse amplitude used during Bleach transition
#define REFRESH_BLEACHING_VOLTAGE           0.7           // (V) Pulse amplitude used during Bleach refresh
#define BLEACHING_TIME                      350           // (ms) Duration of Bleach transition pulse
#define REFRESH_BLEACH_PULSE_TIME           10            // (ms) Duration of Bleach refresh pulse


// ---------------------------------------------------------------------------
// Enums & Configuration Structures
// ---------------------------------------------------------------------------

/**
 * @brief Segment electrochromic states.
 */
enum ecdSegmentState_e {
    SEGMENT_STATE_UNDEFINED = -1,   // Undefined state (power-up)
    SEGMENT_STATE_BLEACH    = 0,    // Bleached (OFF)
    SEGMENT_STATE_COLOR     = 1     // Colored (ON)
};

/**
 * @brief Configuration structure for all ECD driving parameters.
 *
 * Stores threshold voltages, pulse amplitudes, and timing values in V/ms.
 * All internal conversions to LSB occur inside updateRefreshLimits().
 */
struct ECD_Config {

    float refreshColorLimitHVoltage         { REFRESH_COLOR_LIMIT_H_REL_AMP };  // (V) High threshold (Color)
    float refreshColorLimitLVoltage         { REFRESH_COLOR_LIMIT_L_REL_AMP };  // (V) Low threshold (Color)
    
    float refreshBleachLimitHVoltage        { REFRESH_BLEACH_LIMIT_H_REL_AMP }; // (V) High threshold (Bleach)
    float refreshBleachLimitLVoltage        { REFRESH_BLEACH_LIMIT_L_REL_AMP }; // (V) Low threshold (Bleach)

    float coloringVoltage                   { COLORING_VOLTAGE };               // (V) Main Color pulse amplitude
    float refreshColoringVoltage            { REFRESH_COLORING_VOLTAGE };       // (V) Refresh Color amplitude
    int   coloringTime                      { COLORING_TIME };                  // (ms) Color pulse duration
    int   refreshColorPulseTime             { REFRESH_COLOR_PULSE_TIME };       // (ms) Refresh Color pulse duration

    float bleachingVoltage                  { BLEACHING_VOLTAGE };              // (V) Main Bleach pulse amplitude
    float refreshBleachingVoltage           { REFRESH_BLEACHING_VOLTAGE };      // (V) Refresh Bleach amplitude
    int   bleachingTime                     { BLEACHING_TIME };                 // (ms) Bleach pulse duration
    int   refreshBleachPulseTime            { REFRESH_BLEACH_PULSE_TIME };      // (ms) Refresh Bleach pulse duration
};


// ---------------------------------------------------------------------------
// Main Display Driver Class
// ---------------------------------------------------------------------------

/**
 * @class YNV_ECD
 * @brief Main class implementing electrochromic display control for Ynvisible Driver v5.
 *
 * Provides the public API for setting segment states, executing transitions,
 * updating supply voltage, performing OCP checks, and running adaptive refresh
 * routines. Low-level ADC, DAC, and GPIO operations are managed internally.
 */
class YNV_ECD {
public:
    YNV_ECD(int t_numberOfSegments, int* t_segments); ///< Constructor (segment count + pin list)

    void begin();                                     ///< Initialize display (color all, then bleach all)
    void setConfig(const ECD_Config& t_cfg) { m_cfg = t_cfg; updateRefreshLimits(); } ///< Apply new configuration
    void updateSupplyVoltage(int t_supplyVoltage);    ///< Update supply and recalc thresholds
    void executeDisplay();                            ///< Apply pending state changes + refresh
    void setSegmentState(int t_segment, bool t_state);///< Schedule a segment to be colored/bleached
    void setAllSegmentsBleach();                      ///< Convenience: set all segments to BLEACH
    void setStopDrivingFlag();                        ///< Interrupt driving loops safely
    void clearStopDriving();                          ///< Clear driving interruption flag
    void enableCounterElectrode(float t_voltage);     ///< Drive CE via DAC to given voltage
    void disableCounterElectrode();                   ///< Set CE to High-Z
    
private:
    void execute_bleach(void);                        ///< Apply BLEACH transition pulse
    void execute_color(void);                         ///< Apply COLOR transition pulse
    void check_refresh(void);                         ///< Measure OCP and determine refresh needs
    void execute_refresh(void);                       ///< Dispatcher for refresh routines
    void refreshBleach(void);                         ///< Refresh BLEACHED segments
    void refreshColor(void);                          ///< Refresh COLORED segments
    void updateRefreshLimits(void);                   ///< Recompute thresholds in LSB
    void disableAllSegments(void);                    ///< Set all WE pins to High-Z

    ECD_Config m_cfg;
    int        m_numberOfSegments;
    int        m_counterElectrodePin;
    int        m_segmentPinsList       [MAX_NUMBER_OF_SEGMENTS];
    
    uint8_t    m_currentState          [MAX_NUMBER_OF_SEGMENTS];
    uint8_t    m_nextState             [MAX_NUMBER_OF_SEGMENTS];
    
    bool       m_refreshSegmentNeeded  [MAX_NUMBER_OF_SEGMENTS];
    int        m_minBleachOcpLSB       {0};
    bool       m_bleachRequiredFlag;
    bool       m_refresh_bleach_needed;
    bool       m_colorRequiredFlag;
    bool       m_refresh_color_needed;
    
    static bool m_stopDrivingFlag;

    float      m_supplyVoltage         {SUPPLY_VOLTAGE};
    float      m_refreshColorLimitH, m_refreshColorLimitL, m_refreshColorHalf;
    float      m_refreshBleachLimitH, m_refreshBleachLimitL, m_refreshBleachHalf;
};

#endif // _YNVISIBLE_ECD


/***************************************************************************
 ****************************** END OF FILE ********************************
 ***************************************************************************/