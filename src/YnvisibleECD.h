/*
	Ynvisible_ECD.h - Library for driving Ynvisible's Electrochromic Displays
	Created by @BFFonseca - Ynvisible, October 10 2024
	For Driver 5.x Hardware
*/

#ifndef _YNVISIBLE_ECD
#define _YNVISIBLE_ECD

#include "Arduino.h"

#define MAX_NUMBER_OF_SEGMENTS 15
#define MAX_REFRESH_RETRIES 30

#define SUPPLY_VOLTAGE 3.0				// V - Supply voltage of the board
#define ADC_DAC_RESOLUTION 10			// 10 bit resolution. This is the common resolution between the ADC and the DAC.
#define ADC_DAC_MAX_LSB 1023			// Max value of the analogRead and for the analogWrite() functions. 10 bits = 1024 values of Analog read/write

#define REFRESH_COLOR_LIMIT_H_REL_AMP		1.1
#define REFRESH_COLOR_LIMIT_L_REL_AMP		0.9
#define REFRESH_BLEACH_LIMIT_H_REL_AMP 		0.2
#define REFRESH_BLEACH_LIMIT_L_REL_AMP		0.4

#define REFRESH_COLORING_VOLTAGE 			1.3
#define REFRESH_BLEACHING_VOLTAGE 			0.6

#define COLORING_VOLTAGE 					1.4			// V - Absolute value for Color Pulse Voltage
#define COLORING_TIME 						200			// ms - Color time
#define BLEACHING_VOLTAGE 					1.2			// V - Absolute value for Bleach Pulse Voltage
#define BLEACHING_TIME 						500			// ms - Bleach time

#define REFRESH_COLOR_PULSE_TIME 			75
#define REFRESH_BLEACH_PULSE_TIME 			30
#define REFRESH_SLEEP_INTERVAL 				10

enum ecdSegmentState_e{
	SEGMENT_STATE_UNDEFINED = -1,
	SEGMENT_STATE_BLEACH = 0,
	SEGMENT_STATE_COLOR = 1
};

struct ECD_Config{
	//Color & Bleach Configs
	float 	coloringVoltage 						{ COLORING_VOLTAGE };			// V - Absolute value for Color Pulse Voltage
	int 		coloringTime 								{ COLORING_TIME };				// ms - Color time

	float 	bleachingVoltage 						{ BLEACHING_VOLTAGE };			// V - Absolute value for Bleach Pulse Voltage
	int 		bleachingTime 							{ BLEACHING_TIME };				// ms - Bleach time
	
	//Refresh Configs
	float 	refreshColoringVoltage 			{ REFRESH_COLORING_VOLTAGE };					// V - Absolute value for Refresh Coloring Pulses Voltage
	float 	refreshBleachingVoltage 		{ REFRESH_BLEACHING_VOLTAGE };					// V - Absolute value for Refresh Bleching Pulses Voltage

	float 	refreshColorLimitHVoltage			{ REFRESH_COLOR_LIMIT_H_REL_AMP };
	float 	refreshColorLimitLVoltage			{ REFRESH_COLOR_LIMIT_L_REL_AMP };
	float 	refreshBleachLimitHVoltage		{ REFRESH_BLEACH_LIMIT_H_REL_AMP };
	float 	refreshBleachLimitLVoltage		{ REFRESH_BLEACH_LIMIT_L_REL_AMP };

	int 		refreshColorPulseTime				{ REFRESH_COLOR_PULSE_TIME };		// ms - Delay between each Refresh Pulse
	int			refreshBleachPulseTime			{ REFRESH_BLEACH_PULSE_TIME};
};

class YNV_ECD
{
	public:
		YNV_ECD(int t_numberOfSegments, int* t_segments);
		void begin();
		void setConfig(const ECD_Config& t_cfg) { m_cfg = t_cfg; updateRefreshLimits(); }

		void setSegmentState(int t_segment, bool t_state);
		void executeDisplay();
		void refreshDisplay();

		void updateSupplyVoltage(int t_supplyVoltage);
		
		void setStopDrivingFlag();
		void clearStopDriving();
		
		void setAllSegmentsBleach();

		void enableCounterElectrode(float t_voltage);

		void disableCounterElectrode();
		

	private:
		ECD_Config m_cfg;

		int m_numberOfSegments;
		
		uint8_t m_currentState[MAX_NUMBER_OF_SEGMENTS];
		uint8_t	m_nextState[MAX_NUMBER_OF_SEGMENTS];
		float 	m_supplyVoltage {SUPPLY_VOLTAGE};

		float 	m_refreshColorLimitH 					{ ((SUPPLY_VOLTAGE - REFRESH_COLORING_VOLTAGE) + REFRESH_COLOR_LIMIT_H_REL_AMP) * ADC_DAC_MAX_LSB / SUPPLY_VOLTAGE };		//Refresh Color Limit High [LSB]
		float 	m_refreshColorLimitL 					{ (SUPPLY_VOLTAGE/2 +  REFRESH_COLOR_LIMIT_L_REL_AMP) * ADC_DAC_MAX_LSB / SUPPLY_VOLTAGE };															//Refresh Color Limit Low [LSB]
		
		float 	m_refreshBleachLimitH  				{ (SUPPLY_VOLTAGE/2 - REFRESH_BLEACH_LIMIT_H_REL_AMP) * ADC_DAC_MAX_LSB / SUPPLY_VOLTAGE };															//Refresh Bleach Limit High [LSB]
		float 	m_refreshBleachLimitL 				{ (REFRESH_BLEACHING_VOLTAGE - REFRESH_BLEACH_LIMIT_L_REL_AMP) * ADC_DAC_MAX_LSB / SUPPLY_VOLTAGE };										//Refresh Bleach Limit Low [LSB]

		bool 		m_refreshSegmentNeeded[MAX_NUMBER_OF_SEGMENTS];

		//PINS
		int 		m_counterElectrodePin; 													// Counter Electrode Pin
		int 		m_segmentPinsList[MAX_NUMBER_OF_SEGMENTS];			// Pin list for the Displays' Segments

		static bool m_stopDrivingFlag;		// Use this flag to stop driving in any display

		//FUNCTIONS
		void updateRefreshLimits(void);
		
		void disableAllSegments();


		
};

#endif	// _YNVISIBLE_ECD