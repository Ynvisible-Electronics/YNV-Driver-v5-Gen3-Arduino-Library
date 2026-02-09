
/**
 * @file YnvisibleECD.cpp
 * @brief Core electrochromic display (ECD) driving engine for the Ynvisible Driver v5 platform.
 *
 * This file implements the full runtime logic required to drive Ynvisible 
 * electrochromic displays using the Driver v5 hardware. It contains the core 
 * state‑transition routines (coloring and bleaching), refresh handling, 
 * open‑circuit potential (OCP) measurement logic, amplitude safety control, 
 * and supply‑dependent threshold calculation.
 *
 * Responsibilities:
 *  - Handle segment state transitions (Color/Bleach) with correct CE voltages 
 *    and pulse timings based on configuration parameters.
 *  - Measure open‑circuit potentials (OCP) for all segments and determine when
 *    refresh action is required.
 *  - Execute refresh routines for both Color and Bleach states, including:
 *       • Adaptive CE amplitude limiting based on worst‑case OCP measurement.
 *       • Multi‑retry refresh cycles until thresholds are reached.
 *       • Per‑segment refresh flags and retry control.
 *  - Maintain amplitude thresholds in LSB units and dynamically update them
 *    whenever supply voltage or configuration parameters change.
 *  - Expose a high‑level interface (begin, setSegmentState, executeDisplay)
 *    used by higher‑level modules.
 *
 * Notes:
 *  - All driving logic is written using LSB units internally for improved 
 *    performance and to ensure MCU‑safe absolute voltage control.
 *  - CE (Counter Electrode) is driven using the on‑board DAC. Segment electrodes
 *    are controlled through digital GPIOs and measured via ADC.
 *  - This module is independent of LED feedback logic (YnvisibleDriverV5.cpp).
 *
 * Created by @BFFonseca - Ynvisible (May 2025)
 * Updated by JoCFMendes - Ynvisible (Jan 2026)
 */


#include "Arduino.h"
#include "YnvisibleECD.h"


/***************************************************************************/
/**************************** PUBLIC FUNCTIONS *****************************/
/***************************************************************************/

/***************************************************************************/
/**
 * @brief Ynvisible's Electrochromic Display driver
 * 
 * @param t_numberOfSegments display's number of segments
 * @param t_segments array of segments' pins
 */
/***************************************************************************/

 YNV_ECD::YNV_ECD(int t_numberOfSegments, int t_segments[])       // Constructor - Display Initialization
{
  m_counterElectrodePin = PIN_CE;                                 // Configuration of Counter Electrode Pin
  pinMode(m_counterElectrodePin, INPUT);                          // Keep CE in High-Z until driving is active

  analogReadResolution(ADC_DAC_RESOLUTION);                       // Set ADC and DAC resolution to 10 bits operation
  analogWriteResolution(ADC_DAC_RESOLUTION);

  m_numberOfSegments          = t_numberOfSegments;

  for (int i = 0; i < m_numberOfSegments; i++)                    // Initialyze driving variables for each segment
  {
    pinMode(t_segments[i], INPUT);                                // Keep WE (working Electrodes) in High-Z until driving is active 
    m_refreshSegmentNeeded[i] = false;                            // Initialyze segment state
    m_segmentPinsList[i]      = t_segments[i];                        
    m_currentState[i]         = SEGMENT_STATE_UNDEFINED;              
    m_nextState[i]            = SEGMENT_STATE_UNDEFINED;
  }

  m_minBleachOcpLSB           = 0;                                // Variable to store the most negative OCP for bleached segments
  m_bleachRequiredFlag        = false;					                  // Use this flag to indicate that bleaching is required
	m_colorRequiredFlag         = false;					                  // Use this flag to indicate that coloring is required
  m_refresh_color_needed      = false;                            // Flag to enable refresh colored segments routine
  m_refresh_bleach_needed     = false;                            // Flag to enable refresh bleached segments routine
}


/***************************************************************************/
/**
 * @brief Initialise the display (color all segments then bleach).
 */
/***************************************************************************/

 void YNV_ECD::begin()
{ 
  for(int i = 0; i < m_numberOfSegments; i++){                  // Color (turn ON) all segments
    setSegmentState(i, true);
  }
  executeDisplay();
  
  for(int i = 0; i < m_numberOfSegments; i++){                  // Bleach (Turn Off) all segments
    setSegmentState(i, false);
  }
  executeDisplay();
}


/***************************************************************************/
/**
 * @brief Update supply voltage and refresh limits.
 * @param t_supplyVoltage New supply voltage.
 */
/***************************************************************************/

void YNV_ECD::updateSupplyVoltage(int t_supplyVoltage) {
  m_supplyVoltage = t_supplyVoltage;
  updateRefreshLimits();
}


/***************************************************************************/
/**
 * @brief Execute display changes and refresh if needed. 
 * Change the segments' state with YNV_ECD.setSegmentState() and
 * then call this method to apply the new state.
 */
/***************************************************************************/

void YNV_ECD::executeDisplay()
{
  execute_bleach();                                         // Execute state transition to Bleach
  execute_color();                                          // Execute state transition to Color
  check_refresh();                                          // Check if refresh is needed
  execute_refresh();                                        // Execute refresh if necessary
  disableCounterElectrode();                                // Set CE to High-Z for bi-stability
}


/***************************************************************************/
/**
 * @brief Set the state of a segment before execution.
 * @param t_segment Segment index.
 * @param t_state New state of the Segment (t_segment): SEGMENT_STATE_BLEACH (false) or SEGMENT_STATE_COLOR (true)
*/
/***************************************************************************/

void YNV_ECD::setSegmentState(int t_segment, bool t_state)
{
  if (m_currentState[t_segment] != t_state){                // Check if the state of the segment is diferent from previous state
    m_nextState[t_segment] = t_state;                       // If positive, allow segment state change

    if(t_state){                                            // Segment to be changed to Color
      m_colorRequiredFlag      = true;                      // Enable flag to indicate that a color change is required
    }else{                                                  // Segment to be changed to Bleach
      m_bleachRequiredFlag     = true;                      // Enable flag to indicate that a bleach change is required  
    }
  }
}


/***************************************************************************/
/** 
 * @brief Set all segments state to be bleached 
 */
/***************************************************************************/

void YNV_ECD::setAllSegmentsBleach(){
  for(int i = 0; i < m_numberOfSegments; i++){
    setSegmentState(i, SEGMENT_STATE_BLEACH);
  }
}


/***************************************************************************/
/**
 * @brief Set the stopDrivingFlag to true
 * 
 * Use this method to stop the current driving and return
 * to where the displayExecute() method was called
 */
/***************************************************************************/

void YNV_ECD::setStopDrivingFlag() {m_stopDrivingFlag = true;}


/***************************************************************************/
/**
 * @brief Set the stopDrivingFlag to false
 * 
 * Clear the stopDrivingFlag so that the display can be driven again
 */
/***************************************************************************/

void YNV_ECD::clearStopDriving() {m_stopDrivingFlag = false;}


/***************************************************************************/
/**
 * @brief Enable the Counter Electrode's pin
 * 
 * @param t_voltage Voltage with which to drive the Counter Electrode pin (DAC).
 */
/***************************************************************************/

void YNV_ECD::enableCounterElectrode(float t_voltage) {
  
  analogWrite(m_counterElectrodePin, int(ADC_DAC_MAX_LSB*(t_voltage/m_supplyVoltage)));
  delay(50);
}


/***************************************************************************/
/**
 * @brief Disable the Counter Electrode's pin
 * 
 * Sets the Counter Electrode's pin to High-Impedance (High-Z).
 */
/***************************************************************************/
void YNV_ECD::disableCounterElectrode() //Set counter electrode in High-Z.
{
  pinMode(m_counterElectrodePin, INPUT);
}


/***************************************************************************/
/************************** END PUBLIC FUNCTIONS ***************************/
/***************************************************************************/

/***************************************************************************/
/**************************** PRIVATE FUNCTIONS ****************************/
/***************************************************************************/

/***************************************************************************/
/**
 * @brief Routine Block to change Segments state to Bleach
 */
/***************************************************************************/

void YNV_ECD::execute_bleach() {

  if(m_bleachRequiredFlag){

    if(m_stopDrivingFlag == true){                            // Verify if a driving interruption was requested
      return;
    } 

    // Set Virtual ground voltage at CE to provide the Bleach Amplitude Voltage
    enableCounterElectrode(m_cfg.bleachingVoltage);             

    for (int i = 0; i < m_numberOfSegments; i++) {    
      // If the segment state is to change to bleach
      if(m_nextState[i] != m_currentState[i] && m_nextState[i] == SEGMENT_STATE_BLEACH)
      {
        digitalWrite(m_segmentPinsList[i], LOW);              // Drive the segments to Bleach state
        pinMode(m_segmentPinsList[i], OUTPUT);                 
        m_currentState[i] = m_nextState[i];                   // Update current segment state (Bleached / Off) 
      }
    }
    delay(m_cfg.bleachingTime);                               // Execute the defined pulse time for Bleach Transition
    disableAllSegments();                                     // Place all segments in High-Z
    m_bleachRequiredFlag = false;                             // Disable Flag to change the state of segment to Bleach state
  }
}


/***************************************************************************/
/**
 * @brief Routine Block to change Segments state to Color state
 */
/***************************************************************************/

void YNV_ECD::execute_color() {

  if(m_colorRequiredFlag){

    if(m_stopDrivingFlag == true){                          // Verify if a driving interruption was requested
      return;
    }  

    // Set Virtual ground voltage at CE to provide the Color Amplitude Voltage
    enableCounterElectrode(m_supplyVoltage - m_cfg.coloringVoltage);                                  

    for (int i = 0; i < m_numberOfSegments; i++) {
      // If the segment state is to change to color
      if(m_nextState[i] != m_currentState[i] && m_nextState[i] == SEGMENT_STATE_COLOR)
      {
        digitalWrite(m_segmentPinsList[i], HIGH);           // Drive the segments to Color state
        pinMode(m_segmentPinsList[i], OUTPUT);
        m_currentState[i] = m_nextState[i];                 // Update current segment state (Colored / On)
      }
    }
    delay(m_cfg.coloringTime);                              // Execute the defined pulse time for Color Transition
    disableAllSegments();                                   // Place all segments in High-Z
    m_colorRequiredFlag = false;                            // Disable Flag to change the state of segment to Color state
  }
}


/***************************************************************************/
/**
 * @brief Meaure the OCP (Open Circuit Voltage) of the segments and verify 
 * if they need or not to be refreshed.
 */
/***************************************************************************/
 
void YNV_ECD::check_refresh() {
    
  m_minBleachOcpLSB     = 1024;
  m_refresh_color_needed  = false;
  m_refresh_bleach_needed = false;
  int analog_val          = 0;
  // Convert Bleach half amplitude (LSB) to absolute WE threshold (LSB) for check logic
  int bleachHalfAbsLSB    = ((ADC_DAC_MAX_LSB / 2) - (int)m_refreshBleachHalf);
  
  if(m_stopDrivingFlag == true){                          // Verify if a driving interruption was requested
    return;
  }

  enableCounterElectrode(m_supplyVoltage / 2);            // Set CE for half voltage scale to measure color and bleach segments at same reference level

  for (int i = 0; i < m_numberOfSegments; i++) {          // Measure the OCP off all active segments
  
    analog_val = analogRead(m_segmentPinsList[i]);
  
    if (m_currentState[i] == SEGMENT_STATE_COLOR) {       // Check for Color Segments

      if (analog_val > m_refreshColorHalf) {              // No refresh required
        m_refreshSegmentNeeded[i] = false;
      }
      else if (analog_val >= m_refreshColorLimitL) {      // Place in the refesh List in case another segment needs refresh, this one will also be refreshed
        m_refreshSegmentNeeded[i] = true;
      }
      else {                                              // analog_val <= m_refreshColorLimit -> Needs refresh
        m_refreshSegmentNeeded[i] = true;
        m_refresh_color_needed    = true;
      }  
    }
    else if (m_currentState[i] == SEGMENT_STATE_BLEACH) { // Check for Bleached Segments

      if(analog_val <  m_minBleachOcpLSB) {               // Stores lowest OCP value of bleached segments
        m_minBleachOcpLSB = analog_val;
      }

      if (analog_val > m_refreshBleachLimitH) {           // Needs refresh (closest to CE, smallest amplitude)
        m_refreshSegmentNeeded[i] = true;
        m_refresh_bleach_needed   = true;
      }
      else if (analog_val >= bleachHalfAbsLSB) {          // Place in the refesh List in case another segment needs refresh, this one will also be refreshed
        m_refreshSegmentNeeded[i] = true;
      }
      else {                                              // analog_val <= bleachHalfAbsLSB -> No refresh required
        m_refreshSegmentNeeded[i] = false;
      }    
    }
    else {                                                // UNDEFINED OR OTHER STATE
      m_refreshSegmentNeeded[i] = false;
    }
  }
  disableAllSegments();   // Place all segments in High-Z
}


/***************************************************************************/
/**
 * @brief Refresh the display when required.
*/
/***************************************************************************/

void YNV_ECD::execute_refresh() {
  
  if (m_refresh_color_needed) {     // Handle COLOR refresh if required
    refreshColor();
  }

  if (m_refresh_bleach_needed) {    // Handle BLEACH refresh if required
    refreshBleach();
  }
}


/***************************************************************************/
/**
 * @brief Executes the BLEACH refresh routine.
 *
 * Drives CE to a safe refresh voltage based on m_minBleachOcpLSB and
 * applies low pulses to bleached segments marked in m_refreshSegmentNeeded[].
 * After each pulse, segments are re-checked against m_refreshBleachLimitL
 * until the target is reached or MAX_REFRESH_RETRIES is exceeded.
 */
/***************************************************************************/

void YNV_ECD::refreshBleach() {       
  
  int   analog_val      = 0;
  int   retries         = 0;
  float counterElecVal  = 0;

  if (m_stopDrivingFlag) {          // Verify if a driving interruption was requested
    return;
  }

  // Compute safe CE refresh voltage based on minimum BLEACH OCP:
  // m_minBleachOcpLSB holds the lowest WE voltage (in LSB) measured at CE = Vsupply/2 in check_refresh().
  float minAmpV = (m_supplyVoltage / 2.0f) - (m_minBleachOcpLSB * LSB_TO_VOLT_CONV);  // Min OCP voltage on a Bleached Segment in V

  // If measured amplitude is larger than the configured pulse, use the measured amplitude as CE voltage to avoid driving
  // any segment into a negative potential region.
  if (minAmpV > m_cfg.refreshBleachingVoltage) {
    counterElecVal = minAmpV;
  }
  else {
    counterElecVal = m_cfg.refreshBleachingVoltage;
  }

  enableCounterElectrode(counterElecVal);

  while (m_refresh_bleach_needed && (retries < MAX_REFRESH_RETRIES)) {

    if (m_stopDrivingFlag) {                              // Verify if a driving interruption was requested
      return;
    }
      
    for (int i = 0; i < m_numberOfSegments; i++) {        // Refresh the necessary segments

      if (m_currentState[i] == SEGMENT_STATE_BLEACH && m_refreshSegmentNeeded[i] == true) {
        digitalWrite(m_segmentPinsList[i], LOW);
        pinMode(m_segmentPinsList[i], OUTPUT);
      }      
    }

    delay(m_cfg.refreshBleachPulseTime);
    disableAllSegments();
    m_refresh_bleach_needed = false;
    
    for (int i = 0; i < m_numberOfSegments; i++) {        // check if segments still need refresh 

      if (m_currentState[i] == SEGMENT_STATE_BLEACH && m_refreshSegmentNeeded[i] == true) {
        
        analog_val = analogRead(m_segmentPinsList[i]);

        if (analog_val > m_refreshBleachLimitL) {
          m_refresh_bleach_needed   = true;
        } 
        else {
          m_refreshSegmentNeeded[i] = false;              // Segment reached target OCP, no more refresh required
        }
      }
    }

    retries++;
  }
}


/***************************************************************************/
/**
 * @brief Executes the COLOR refresh routine.
 *
 * Drives CE to the configured refresh coloring voltage and applies high
 * pulses to colored segments marked in m_refreshSegmentNeeded[]. After each
 * pulse, segments are re-checked against m_refreshColorLimitH until the
 * target is reached or MAX_REFRESH_RETRIES is exceeded.
 */
/***************************************************************************/
void YNV_ECD::refreshColor()
{
  int   analog_val     = 0;
  int   retries        = 0;
  float counterElecVal = 0.0f;

  if (m_stopDrivingFlag) {                                // Verify if a driving interruption was requested
    return;
  }

  counterElecVal = (m_supplyVoltage - m_cfg.refreshColoringVoltage); // CE value for Color refresh:

  enableCounterElectrode(counterElecVal);

  while (m_refresh_color_needed && (retries < MAX_REFRESH_RETRIES)) {

    if (m_stopDrivingFlag) {                              // Verify if a driving interruption was requested
      return;
    }

    // Apply refresh pulse to all colored segments that still need refresh
    for (int i = 0; i < m_numberOfSegments; i++) {
      if ((m_currentState[i] == SEGMENT_STATE_COLOR) && (m_refreshSegmentNeeded[i] == true)) {
        digitalWrite(m_segmentPinsList[i], HIGH);
        pinMode(m_segmentPinsList[i], OUTPUT);
      }
    }

    delay(m_cfg.refreshColorPulseTime);

    if (m_stopDrivingFlag) {                              // Verify if a driving interruption was requested
      return;
    }

    disableAllSegments();
    m_refresh_color_needed = false;

    // Check which segments still need color refresh
    for (int i = 0; i < m_numberOfSegments; i++) {

      if (m_currentState[i] == SEGMENT_STATE_COLOR && m_refreshSegmentNeeded[i] == true) {
        analog_val = analogRead(m_segmentPinsList[i]);

        if (analog_val < m_refreshColorLimitH) {          // Segment OCP is still below target → needs more refresh
          m_refreshSegmentNeeded[i] = true;
          m_refresh_color_needed    = true;
        } 
        else {
          m_refreshSegmentNeeded[i] = false;              // Segment reached target OCP → remove from refresh list
        }
      }
    }
    retries++;
  }
}


/***************************************************************************/
/**
 * @brief Update the Refresh Limits for driving
 * @note  Call this method whenever a parameter that influences the 
 *        limits changes. e.g. Supply Voltage or Coloring Voltage.
 */
/***************************************************************************/

void YNV_ECD::updateRefreshLimits(void) {

  m_refreshColorLimitH = ((m_supplyVoltage - m_cfg.refreshColoringVoltage) + m_cfg.refreshColorLimitHVoltage) * (ADC_DAC_MAX_LSB / m_supplyVoltage);
  m_refreshColorLimitL = (m_supplyVoltage/2 +  m_cfg.refreshColorLimitLVoltage) * (ADC_DAC_MAX_LSB / m_supplyVoltage);
  m_refreshColorHalf   = ((m_refreshColorLimitH + m_refreshColorLimitL) / 2.0f);     

  m_refreshBleachLimitH = (m_supplyVoltage/2 - m_cfg.refreshBleachLimitHVoltage) * (ADC_DAC_MAX_LSB / m_supplyVoltage);
  m_refreshBleachLimitL = (m_cfg.refreshBleachingVoltage - m_cfg.refreshBleachLimitLVoltage) * (ADC_DAC_MAX_LSB / m_supplyVoltage);

  // Compute CE levels in LSB
  int CE_check_lsb   = (ADC_DAC_MAX_LSB / 2);  // CE = Vsupply / 2
  int CE_refresh_lsb = (int)(ADC_DAC_MAX_LSB * (m_cfg.refreshBleachingVoltage / m_supplyVoltage)); // CE = refreshBleachingVoltage

  // Compute amplitudes (in LSB)
  int amp_H_lsb = abs(CE_check_lsb   - (int)m_refreshBleachLimitH);
  int amp_L_lsb = abs(CE_refresh_lsb - (int)m_refreshBleachLimitL);

  // Compute mid amplitude (in LSB)
  m_refreshBleachHalf = (amp_H_lsb + amp_L_lsb) * 0.5f;
}


/***************************************************************************/
/**
 * @brief Disable all the segments
 * Sets all the segments' pins to High-Impedance(High-Z).
 * 
 * @note this is not the same as bleaching all the segments.
 */
/***************************************************************************/

void YNV_ECD::disableAllSegments() { 
  
  for (int i = 0; i < m_numberOfSegments; i++)
  {
    pinMode(m_segmentPinsList[i], INPUT);       // Set all work electrodes to High-Z mode.
  }
}


/***************************************************************************/
/************************** END PRIVATE FUNCTIONS **************************/
/***************************************************************************/

/***************************************************************************
 ****************************** END OF FILE ********************************
 ***************************************************************************/