/**
 * Ynvisible Electrochromic Displays driving software for Driver v5 board
 * Created by @BFFonseca - Ynvisible, May 2025
 */
#include "Arduino.h"
#include "YnvisibleECD.h"


/*********************** PUBLIC FUNCTIONS ************************/

/**
 * @brief Ynvisible's Electrochromic Display driver
 * 
 * @param t_numberOfSegments display's number of segments
 * @param t_segments array of segments' pins
 */
YNV_ECD::YNV_ECD(int t_numberOfSegments, int t_segments[])        // Constructor
{
  m_counterElectrodePin = PIN_CE;
  pinMode(m_counterElectrodePin, OUTPUT);

  m_numberOfSegments = t_numberOfSegments;

  analogReadResolution(ADC_DAC_RESOLUTION);
  analogWriteResolution(ADC_DAC_RESOLUTION);

  for (int i = 0; i < m_numberOfSegments; i++)
  {
    m_segmentPinsList[i] = t_segments[i];

    pinMode(m_segmentPinsList[i], INPUT);

    m_currentState[i] = SEGMENT_STATE_UNDEFINED;
    m_nextState[i] = SEGMENT_STATE_UNDEFINED;
  }
}

/**
 * This method initializes the display by setting all segments to the color state and then bleaching them.
 * It is typically called at the beginning of the program to prepare the display for use.
 */
void YNV_ECD::begin()
{
  //Color all segments
  for(int i = 0; i < m_numberOfSegments; i++){
    setSegmentState(i, true);
  }
  executeDisplay();
  
  //Bleach all segments
  for(int i = 0; i < m_numberOfSegments; i++){
    setSegmentState(i, false);
  }
  executeDisplay();

}


/**
 * Set the State of a Segment before executing the display
 * @param t_segment t_segment to change t_state
 * @param t_state new t_state of the t_segment: SEGMENT_STATE_BLEACH or SEGMENT_STATE_COLOR
*/
void YNV_ECD::setSegmentState(int t_segment, bool t_state){
  m_nextState[t_segment] = t_state;
}

/**
 * Colors and bleaches segments, depending on their state.
 * Change the segments' state with YNV_ECD.setSegmentState() and
 * then call this method to apply the new state.
*/
void YNV_ECD::executeDisplay(){
  bool isDelayRequired = false;

  enableCounterElectrode(m_cfg.bleachingVoltage);

  for (int i = 0; i < m_numberOfSegments; i++)
  {
    if(m_stopDrivingFlag == true){
      return;
    }

    if(m_nextState[i] != m_currentState[i] && m_nextState[i] == SEGMENT_STATE_BLEACH)
    {
      pinMode(m_segmentPinsList[i], OUTPUT);
      digitalWrite(m_segmentPinsList[i], m_nextState[i]);
      
      m_currentState[i] = m_nextState[i];
      isDelayRequired = true;
    }
  }
  if(isDelayRequired == true){
    if(m_stopDrivingFlag == true){
      return;
    }
    delay(m_cfg.bleachingTime);
  }
  
  disableAllSegments();

  enableCounterElectrode(m_supplyVoltage - m_cfg.coloringVoltage);
  isDelayRequired = false;

  for (int i = 0; i < m_numberOfSegments; i++)
  {
    if(m_stopDrivingFlag == true){
      return;
    }
    if(m_nextState[i] != m_currentState[i] && m_nextState[i] == SEGMENT_STATE_COLOR)
    {
      pinMode(m_segmentPinsList[i], OUTPUT);
      digitalWrite(m_segmentPinsList[i], m_nextState[i]);

      m_currentState[i] = m_nextState[i];
      isDelayRequired = true;
    }
  }
  if(isDelayRequired == true){
    if(m_stopDrivingFlag == true){
      return;
    }
    delay(m_cfg.coloringTime);
  }
  disableAllSegments();
  disableCounterElectrode();

  refreshDisplay();
  if(m_stopDrivingFlag == true){
    return;
  }
}

/**
 * Refresh the display. Either after a Display Execute or when the CPU wakes up from sleep.
 * @note the method will check if a refresh is required and return if it isn't.
*/
void YNV_ECD::refreshDisplay() //Refreshes the display to maintain the current t_state.
{
  int analog_val = 0;
  bool refresh_color_needed = false, refresh_bleach_needed = false;
  float counterElecVal = 0;
  float currentLimit = 0;
  int retries = 0;

  if(m_stopDrivingFlag == true){
    return;
  }

  enableCounterElectrode(m_supplyVoltage / 2);
  disableAllSegments(); // Put all pins in Input mode

  for (int i = 0; i < m_numberOfSegments; i++) {
    if(m_stopDrivingFlag == true){
      return;
    }
    pinMode(m_segmentPinsList[i], INPUT);
    analog_val = analogRead(m_segmentPinsList[i]);

    if (m_currentState[i] == SEGMENT_STATE_COLOR && (analog_val < m_refreshColorLimitL)) {
      m_refreshSegmentNeeded[i] = true;
      refresh_color_needed = true;
    } 
    else if (m_currentState[i] == SEGMENT_STATE_BLEACH && (analog_val > m_refreshBleachLimitH)) {
      m_refreshSegmentNeeded[i] = true;
      refresh_bleach_needed = true;
    }
  }

  if(m_stopDrivingFlag == true){
    return;
    }

  if ((refresh_bleach_needed || refresh_color_needed) == false) {
    return; // Return if Refresh isn't needed.
  }

  counterElecVal = m_cfg.refreshBleachingVoltage;
  
  enableCounterElectrode(counterElecVal);

  while (refresh_bleach_needed) {
    if(m_stopDrivingFlag == true){
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

    if(m_stopDrivingFlag == true){
      return;
    }


    // Check which Segments still need bleach refresh
    for (int i = 0; i < m_numberOfSegments; i++) {
      if(m_stopDrivingFlag == true){
        return;
      }
      if (m_currentState[i] == SEGMENT_STATE_BLEACH && retries < MAX_REFRESH_RETRIES) {
        pinMode(m_segmentPinsList[i], INPUT);
        analog_val = analogRead(m_segmentPinsList[i]);
        if (analog_val > m_refreshBleachLimitL) {
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
    if(m_stopDrivingFlag == true){
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
    if(m_stopDrivingFlag == true){
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
        if (analog_val < currentLimit) {
          m_refreshSegmentNeeded[i] = true;
          refresh_color_needed = true;
        }
      }
    }
    retries++;
  }

  disableCounterElectrode();
}

/**
 * Update the Supply Voltage value.
 * @param t_supplyVoltage new supply voltage value
*/
void YNV_ECD::updateSupplyVoltage(int t_supplyVoltage){
  m_supplyVoltage = t_supplyVoltage;
  updateRefreshLimits();
}

/**
 * @brief Set the stopDrivingFlag to true
 * 
 * Use this method to stop the current driving and return
 * to where the displayExecute() method was called
 */
void YNV_ECD::setStopDrivingFlag(){
  m_stopDrivingFlag = true;
}

/**
 * @brief Set the stopDrivingFlag to false
 * 
 * Clear the stopDrivingFlag so that the display can be driven again
 */
void YNV_ECD::clearStopDriving(){
  m_stopDrivingFlag = false;
}

/** Set all segments to bleach */
void YNV_ECD::setAllSegmentsBleach(){
  for(int i = 0; i < m_numberOfSegments; i++){
    setSegmentState(i, SEGMENT_STATE_BLEACH);
  }
}

/********************* END PUBLIC FUNCTIONS **********************/


/*********************** PRIVATE FUNCTIONS ***********************/

/**
 * Update the Refresh Limits for driving
 * Call this method whenever a parameter that influences the limits changes. e.g. Supply Voltage or Coloring Voltage.
 */
void YNV_ECD::updateRefreshLimits(void){
  m_refreshColorLimitH = ((m_supplyVoltage - m_cfg.refreshColoringVoltage) + m_cfg.refreshColorLimitHVoltage) * ADC_DAC_MAX_LSB / m_supplyVoltage;
  m_refreshColorLimitL = (m_supplyVoltage/2 +  m_cfg.refreshColorLimitLVoltage) * ADC_DAC_MAX_LSB / m_supplyVoltage;

  m_refreshBleachLimitH = (m_supplyVoltage/2 - m_cfg.refreshBleachLimitHVoltage) * ADC_DAC_MAX_LSB / m_supplyVoltage;
  m_refreshBleachLimitL = (m_cfg.refreshBleachingVoltage - m_cfg.refreshBleachLimitLVoltage) * ADC_DAC_MAX_LSB / m_supplyVoltage;
}

/**
 * @brief Disable all the segments
 * 
 * Sets all the segments' pins to High-Impedance
 * 
 * @note this is not the same as bleaching all the segments
 */
void YNV_ECD::disableAllSegments(){ //Put all work electrodes in High-Z mode.
  for (int i = 0; i < m_numberOfSegments; i++)
  {
    pinMode(m_segmentPinsList[i], INPUT);
  }
}

/**
 * @brief Enable the Counter Electrode's pin
 * 
 * @param t_voltage Voltage with which to drive 
 * the Counter Electrode analog pin.
 */
void YNV_ECD::enableCounterElectrode(float t_voltage) //Enable counter electrode
{
  analogWrite(m_counterElectrodePin, int(ADC_DAC_MAX_LSB*(t_voltage/m_supplyVoltage)));
  delay(1);
}

/**
 * @brief Disable the Counter Electrode's pin
 * 
 * Sets the Counter Electrode's pin to High-Impedance
 */
void YNV_ECD::disableCounterElectrode() //Set counter electrode in High-Z.
{
  analogWrite(m_counterElectrodePin, 0);
}
/********************* END PRIVATE FUNCTIONS **********************/