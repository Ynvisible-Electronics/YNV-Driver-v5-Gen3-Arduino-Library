#include "Arduino.h"
#include "Wire.h"
#include "YnvisibleECD.h"
#include "YnvisibleSignageKit.h"


YNV_SIGNAGE_I2C_MESSAGE::YNV_SIGNAGE_I2C_MESSAGE(void){
    m_length = 2;                                   // Minimum length, without display data
    m_messageMode = 1;                                     // Default mode is ASCII
    m_numDisplays = 0;                              // Initialize with no displays
    m_messageBufferTX = (uint8_t*) malloc(8);       // Minimum message length if there's no display data (this will be added after)
    m_messageBufferTX[0] = 0x02;
}

bool YNV_SIGNAGE_I2C_MESSAGE::setLength(uint16_t t_length){
    uint8_t * backUpDataPtr = m_messageBufferTX;

    m_messageBufferTX = (uint8_t*)realloc(m_messageBufferTX, t_length + 6); // Try to reallocate memory

    if(!m_messageBufferTX){                                 // If reallocation fails,
        m_messageBufferTX = backUpDataPtr;                  // Restore pointer
        return false;                                       // Return false
    }

    m_length = t_length;
    
    this->calculateTotalSize();
    return true;
}

bool YNV_SIGNAGE_I2C_MESSAGE::setMessageMode(int t_messageMode){
    if(t_messageMode == SIGN_MODE_SEGMENTS || t_messageMode == SIGN_MODE_ASCII){
        m_messageMode = t_messageMode;
        return true;
    }
    return false;
}
bool YNV_SIGNAGE_I2C_MESSAGE::setInputMode(int t_inputMode){
    if(t_inputMode == SIGN_INPUT_KEYBOARD || t_inputMode == SIGN_INPUT_ASCII || t_inputMode == SIGN_INPUT_SEGMENTS || t_inputMode == SIGN_INPUT_CLEAR_ALL){
        m_inputMode = t_inputMode;
        return true;
    }
    return false;
}

bool YNV_SIGNAGE_I2C_MESSAGE::setNumberOfDisplays(int t_numDisplays){
    if(t_numDisplays >= 1 || t_numDisplays <= 255){
        m_numDisplays = t_numDisplays;
        return true;
    }
    return false;
}

bool YNV_SIGNAGE_I2C_MESSAGE::setDisplayData(char t_data, int t_position){
    this->calculateTotalSize();                 // Calculate current total size, just to be sure

    if(t_position < 5 || ((t_position) >= (m_length+3))){     //TODO: Double-check that these values are correct
        return false;
    }

    m_messageBufferTX[t_position] = t_data;

    return true;

}
bool YNV_SIGNAGE_I2C_MESSAGE::setDisplayData(char * t_data, int t_position, int t_size){

    this->calculateTotalSize();                 // Calculate current total size, just to be sure

    if(t_position < 5 || ((t_position + t_size) >= (m_length+3))){     //TODO: Double-check that these values are correct
        return false;
    }

    // If the new Display Data is in a correct position, add it to the Message's Display Data
    for(uint8_t i = 0; i < t_size; i++){
        m_messageBufferTX[t_position + i] = t_data[i];
    }

    return true;
}

uint16_t YNV_SIGNAGE_I2C_MESSAGE::getMessageLength(void){
    return m_length;
}

uint8_t YNV_SIGNAGE_I2C_MESSAGE::getMessageMode(void){
    return m_messageMode;
}

uint8_t YNV_SIGNAGE_I2C_MESSAGE::getInputMode(void){
    return m_inputMode;
}

uint8_t YNV_SIGNAGE_I2C_MESSAGE::getNumberOfDisplays(void){
    return m_numDisplays;
}

uint8_t * YNV_SIGNAGE_I2C_MESSAGE::getMessage(void){
    uint16_t checkSum = 0;

    // Re-calculate Total Size
    this->calculateTotalSize();

    m_messageBufferTX[0] = 0x02;
    m_messageBufferTX[1] = m_length >> 8;
    m_messageBufferTX[2] = m_length & 0xFF;
    
    m_messageBufferTX[3] = m_numDisplays;
    m_messageBufferTX[4] = m_messageMode;
    // Calculate Checksum
    for(uint16_t i = 0; i < m_length; i++){
        checkSum += m_messageBufferTX[i+3];
    }

    // Add CheckSum and End TX Bytes to the message
    m_messageBufferTX[m_totalSize-3] = (checkSum >> 8);     // Add CheckSum MSB
    m_messageBufferTX[m_totalSize-2] = checkSum & 0xFF;     // Add CheckSum LSB
    
    m_messageBufferTX[m_totalSize-1] = 0x03;                // Add End TX byte
    
    return m_messageBufferTX; 
}

uint16_t YNV_SIGNAGE_I2C_MESSAGE::getTotalSize(void){
    this->calculateTotalSize();
    return m_totalSize;
}


void YNV_SIGNAGE_I2C_MESSAGE::calculateTotalSize(void){
    m_totalSize = m_length + 6;     // Only the lenght is variable because of the Display Data Bytes
}