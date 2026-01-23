/**
 * Ynvisible Electrochromic Displays driving software for Driver v5 board
 * Created by @BFFonseca - Ynvisible, May 2025
 * Modifyed by JoCFMendes - Ynvisible, January 2026
 */

#include "Arduino.h"
#include "YnvisibleECD.h"


/***************************************************************************/
/**************************** PUBLIC FUNCTIONS *****************************/
/***************************************************************************/

/**
 * @brief Ynvisible's Electrochromic Display driver
 * 
 * @param t_numberOfSegments display's number of segments
 * @param t_segments array of segments' pins
 */
YNV_ECD::YNV_ECD(int t_numberOfSegments, int t_segments[])        // Constructor - Display Initialization
{
  m_counterElectrodePin = PIN_CE;                                 // Configuration of Counter Electrode Pin
  pinMode(m_counterElectrodePin, INPUT);                          // Keep CE in High-Z until driving is active

  analogReadResolution(ADC_DAC_RESOLUTION);                       // Set ADC and DAC resolution to 10 bits operation
  analogWriteResolution(ADC_DAC_RESOLUTION);

  m_numberOfSegments = t_numberOfSegments;

  for (int i = 0; i < m_numberOfSegments; i++)                    // Initialyze driving variables for each segment
  {
    m_segmentPinsList[i]  = t_segments[i];
    pinMode(m_segmentPinsList[i], INPUT);                         // Keep WE (working Electrodes) in High-Z until driving is active
    
    m_currentState[i]     = SEGMENT_STATE_UNDEFINED;              // Initialyze segment state
    m_nextState[i]        = SEGMENT_STATE_UNDEFINED;
  }

  m_bleachRequiredFlag  = false;					                        // Use this flag to indicate that bleaching is required
	m_colorRequiredFlag   = false;					                        // Use this flag to indicate that coloring is required
  m_refresh_color_needed  = false;                                // Flag to enable refresh colored segments routine
  m_refresh_bleach_needed = false;                                // Flag to enable refresh bleached segments routine
}


/***************************************************************************/

/**
 * This method initializes the display by setting all segments to the color state and then bleaching them.
 * It is typically called at the beginning of the program to prepare the display for use.
 */
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
 * Set the State of a Segment before executing the display
 * @param t_segment t_segment to change t_state
 * @param t_state new t_state of the t_segment: SEGMENT_STATE_BLEACH or SEGMENT_STATE_COLOR
*/
void YNV_ECD::setSegmentState(int t_segment, bool t_state)
{
  if (m_currentState[t_segment] != t_state){          // Check if the state of the segment is diferent from previous state
    m_nextState[t_segment] = t_state;                 // If positive, allow segment state change

    if(t_state){                                      // Segment to be changed to Color
      m_colorRequiredFlag      = true;                // Enable flag to indicate that a color change is required
    }else{                                            // Segment to be changed to Bleach
      m_bleachRequiredFlag     = true;                // Enable flag to indicate that a bleach change is required  
    }
  }
}


/***************************************************************************/

/**
 * Colors and bleaches display segments, depending on their state can execute
 * the necessary state transition or a refresh of it's state.
 * Change the segments' state with YNV_ECD.setSegmentState() and
 * then call this method to apply the new state.
*/
void YNV_ECD::executeDisplay()
{
  execute_bleach();                   // Change segments states to bleach
  execute_color();                    // Change segments states to color
  execute_refresh();                  // Executes the refresh routine if are segments that need to be refreshed
  disableCounterElectrode();          // Set CE also to High-Z to improve the display bi-stability time
}


/***************************************************************************/

/**
 * Refresh the display. Either after a Display Execute or when the CPU wakes up from sleep.
 * @note the method will check if a refresh is required and return if it isn't.
*/
void YNV_ECD::execute_refresh() //Refreshes the display to maintain the current t_state.
{
  int analog_val = 0;
  bool refresh_color_needed = false, refresh_bleach_needed = false;
  float counterElecVal = 0;
  float currentLimit = 0;
  int retries = 0;

  if(m_stopDrivingFlag == true){                            // Verify if a driving interruption was requested
    return;
  } 
  
  enableCounterElectrode(m_supplyVoltage / 2);

  for (int i = 0; i < m_numberOfSegments; i++) {
  
    if(m_stopDrivingFlag == true){                          // Verify if a driving interruption was requested
      return;
    } 
    
    analog_val = analogRead(m_segmentPinsList[i]);
  
    if (m_currentState[i] == SEGMENT_STATE_COLOR && (analog_val < m_refreshColorLimitL)) {
      m_refreshSegmentNeeded[i] = true;
      refresh_color_needed = true;
    } 
    else if (m_currentState[i] == SEGMENT_STATE_BLEACH && (analog_val > m_refreshBleachLimitH)) {
      m_refreshSegmentNeeded[i] = true;
      refresh_bleach_needed = true;
    }
    else{
      m_refreshSegmentNeeded[i] = false;
    }
  }

  if ((refresh_bleach_needed || refresh_color_needed) == false) {
    return; // Return if Refresh isn't needed.
  }

  counterElecVal = m_cfg.refreshBleachingVoltage;
  enableCounterElectrode(counterElecVal);

  while (refresh_bleach_needed) {

    if(m_stopDrivingFlag == true){                          // Verify if a driving interruption was requested
      return;
    } 

    for (int i = 0; i < m_numberOfSegments; i++) {
      if(m_stopDrivingFlag == true){
        return;
      }
      if (m_currentState[i] == SEGMENT_STATE_BLEACH && m_refreshSegmentNeeded[i] == true) {
        pinMode(m_segmentPinsList[i], OUTPUT);
        digitalWrite(m_segmentPinsList[i], LOW);
      }
    }

    delay(m_cfg.refreshBleachPulseTime);
    disableAllSegments();
    refresh_bleach_needed = false;

    // Check which Segments still need bleach refresh
    for (int i = 0; i < m_numberOfSegments; i++) {
      
      if(m_stopDrivingFlag == true){                    // Verify if a driving interruption was requested
        return;
      } 
      
      if (m_currentState[i] == SEGMENT_STATE_BLEACH && retries < MAX_REFRESH_RETRIES) {

        analog_val = analogRead(m_segmentPinsList[i]);

        if (analog_val >= m_refreshBleachLimitL) {
          m_refreshSegmentNeeded[i] = true;
          refresh_bleach_needed = true;
        }
      }
    }
    retries++;
  }

  counterElecVal = m_supplyVoltage - m_cfg.refreshColoringVoltage;
  currentLimit = m_refreshColorLimitH;
  retries = 0;

  enableCounterElectrode(counterElecVal);

  while (refresh_color_needed) {
    if(m_stopDrivingFlag == true){                      // Verify if a driving interruption was requested
      return;
    } 

    for (int i = 0; i < m_numberOfSegments; i++) {
      if(m_stopDrivingFlag == true){
        return;
      }
      if (m_currentState[i] == SEGMENT_STATE_COLOR && m_refreshSegmentNeeded[i] == true) {
        pinMode(m_segmentPinsList[i], OUTPUT);
        digitalWrite(m_segmentPinsList[i], HIGH);      
      }
    }

    delay(m_cfg.refreshColorPulseTime);

    if(m_stopDrivingFlag == true){                      // Verify if a driving interruption was requested
      return;
    } 

    disableAllSegments();
    refresh_color_needed = false;

    // Check which Segments still need Color refresh
    for (int i = 0; i < m_numberOfSegments; i++) {
      if(m_stopDrivingFlag == true){
        return;
      }
      if (m_currentState[i] == SEGMENT_STATE_COLOR && retries < MAX_REFRESH_RETRIES) {
        pinMode(m_segmentPinsList[i], INPUT);
        analog_val = analogRead(m_segmentPinsList[i]);

        if (analog_val <= currentLimit) {
          m_refreshSegmentNeeded[i] = true;
          refresh_color_needed = true;
        }
      }
    }
    retries++;
  }
}


/***************************************************************************/

/**
 * Update the Supply Voltage value.
 * @param t_supplyVoltage new supply voltage value
*/
void YNV_ECD::updateSupplyVoltage(int t_supplyVoltage){
  m_supplyVoltage = t_supplyVoltage;
  updateRefreshLimits();
}


/***************************************************************************/

/**
 * @brief Set the stopDrivingFlag to true
 * 
 * Use this method to stop the current driving and return
 * to where the displayExecute() method was called
 */
void YNV_ECD::setStopDrivingFlag(){
  m_stopDrivingFlag = true;
}


/***************************************************************************/

/**
 * @brief Set the stopDrivingFlag to false
 * 
 * Clear the stopDrivingFlag so that the display can be driven again
 */
void YNV_ECD::clearStopDriving(){
  m_stopDrivingFlag = false;
}


/***************************************************************************/

/** 
 * @brief Set all segments state to be bleached 
 */
void YNV_ECD::setAllSegmentsBleach(){
  for(int i = 0; i < m_numberOfSegments; i++){
    setSegmentState(i, SEGMENT_STATE_BLEACH);
  }
}

/***************************************************************************/
/************************** END PUBLIC FUNCTIONS ***************************/
/***************************************************************************/

/***************************************************************************/
/**************************** PRIVATE FUNCTIONS ****************************/
/***************************************************************************/

/**
 * @brief Routine Block to change Segments state to Bleach
 */
void YNV_ECD::execute_bleach(){

  if(m_bleachRequiredFlag){

    // Set Virtual ground voltage at CE to provide the Bleach Amplitude Voltage
    enableCounterElectrode(m_cfg.bleachingVoltage);             

    if(m_stopDrivingFlag == true){                            // Verify if a driving interruption was requested
      return;
    }                                       

    for (int i = 0; i < m_numberOfSegments; i++)
    {
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
 * @brief Routine Block to change Segments state to Color
 */
void YNV_ECD::execute_color(){

  if(m_colorRequiredFlag){

    // Set Virtual ground voltage at CE to provide the Color Amplitude Voltage
    enableCounterElectrode(m_supplyVoltage - m_cfg.coloringVoltage);

    if(m_stopDrivingFlag == true){                          // Verify if a driving interruption was requested
    return;
    }                                     
    
    for (int i = 0; i < m_numberOfSegments; i++)                        
    {
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
 * Update the Refresh Limits for driving
 * Call this method whenever a parameter that influences the limits changes. e.g. Supply Voltage or Coloring Voltage.
 */
void YNV_ECD::updateRefreshLimits(void){
  m_refreshColorLimitH = ((m_supplyVoltage - m_cfg.refreshColoringVoltage) + m_cfg.refreshColorLimitHVoltage) * (ADC_DAC_MAX_LSB / m_supplyVoltage);
  m_refreshColorLimitL = (m_supplyVoltage/2 +  m_cfg.refreshColorLimitLVoltage) * (ADC_DAC_MAX_LSB / m_supplyVoltage);

  m_refreshBleachLimitH = (m_supplyVoltage/2 - m_cfg.refreshBleachLimitHVoltage) * (ADC_DAC_MAX_LSB / m_supplyVoltage);
  m_refreshBleachLimitL = (m_cfg.refreshBleachingVoltage - m_cfg.refreshBleachLimitLVoltage) * (ADC_DAC_MAX_LSB / m_supplyVoltage);
}


/***************************************************************************/

/**
 * @brief Disable all the segments
 * 
 * Sets all the segments' pins to High-Impedance
 * 
 * @note this is not the same as bleaching all the segments
 */
void YNV_ECD::disableAllSegments()
{ 
  for (int i = 0; i < m_numberOfSegments; i++)
  {
    pinMode(m_segmentPinsList[i], INPUT);       // Set all work electrodes to High-Z mode.
  }
}


/***************************************************************************/

/**
 * @brief Enable the Counter Electrode's pin
 * 
 * @param t_voltage Voltage with which to drive 
 * the Counter Electrode analog pin.
 */
void YNV_ECD::enableCounterElectrode(float t_voltage) //Enable counter electrode
{
  analogWrite(m_counterElectrodePin, int(ADC_DAC_MAX_LSB*(t_voltage/m_supplyVoltage)));
  delay(50);
}


/***************************************************************************/

/**
 * @brief Disable the Counter Electrode's pin
 * 
 * Sets the Counter Electrode's pin to High-Impedance
 */
void YNV_ECD::disableCounterElectrode() //Set counter electrode in High-Z.
{
  pinMode(m_counterElectrodePin, INPUT);
}


/***************************************************************************/
/************************** END PRIVATE FUNCTIONS **************************/
/***************************************************************************/

/***************************************************************************
 ******************************** END **************************************
 ***************************************************************************/