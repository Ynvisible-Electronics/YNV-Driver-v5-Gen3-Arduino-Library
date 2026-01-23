#ifndef _YNVISIBLE_SIGNAGE_KIT_
#define _YNVISIBLE_SIGNAGE_KIT_

// TODO: change to 10 on negative counter implementation
#define SIGN_KIT_NUM_ANIMATIONS 12


/**
 * Delays for the animations
 * Delays under two seconds are in miliseconds and are used directly with the `delay()` function
 * Delays above two seconds are in seconds and are used in the `isAnimationCanceled()` function
 * 
 */
#define SIGN_ANIMATION_DELAY_7SEG_DOT_NUMERIC_COUNTER    30000   // ms

#define SIGN_ANIMATION_DELAY_LEFT_RIGHT              1250    // ms
#define SIGN_ANIMATION_DELAY_TOP_BOTTOM              1000    // ms
#define SIGN_ANIMATION_DELAY_ALPHABETIC_COUNTER      5000    // ms
#define SIGN_ANIMATION_DELAY_EMOJI_COUNTER           7000    // ms
#define SIGN_ANIMATION_DELAY_PATTERN_BLINK           4000    // ms
#define SIGN_ANIMATION_DELAY_ARROW_COUNTER           5000    // ms
#define SIGN_ANIMATION_DELAY_NUMERIC_COUNTER         5000    // ms


enum signageKitAnimations_e{
    SIGN_ANIMATION_7SEG_DOT_NUMERIC_COUNTER_UP = 0,
    SIGN_ANIMATION_7SEG_DOT_NUMERIC_COUNTER_DOWN,
    SIGN_ANIMATION_7SEG_DOT_NUMERIC_NEGATIVE,
    SIGN_ANIMATION_LEFT_RIGHT,
    SIGN_ANIMATION_TOP_BOTTOM,
    SIGN_ANIMATION_ALPHABETIC_COUNTER,
    SIGN_ANIMATION_EMOJI_COUNTER,
    SIGN_ANIMATION_PATTERN_BLINK,
    SIGN_ANIMATION_ARROW_COUNTER,
    SIGN_ANIMATION_NUMERIC_COUNTER_UP,
    SIGN_ANIMATION_NUMERIC_COUNTER_DOWN,
    SIGN_ANIMATION_SERIAL_MONITOR
};

enum signageInputModes_e{
    SIGN_INPUT_KEYBOARD = 1,
    SIGN_INPUT_ASCII,
    SIGN_INPUT_SEGMENTS,
    SIGN_INPUT_CLEAR_ALL
};
enum displayType_e{
    SIGN_DISPLAY_MATRIX = 1,
    SIGN_DISPLAY_7SEG,
    SIGN_DISPLAY_MIXED
};
enum signageMessageModes_e{
    SIGN_MODE_SEGMENTS = 0,
    SIGN_MODE_ASCII
};

/**
 * Class to store the I2C Message and all it's data
 *                          Message composition
 * --------------------------------------------------------------------------
 * |  Byte    |     Name            |   Default Value                       |
 * |----------|-------------------------------------------------------------| __              __
 * | Byte 0   | Start TX            |    [0x02]                             |   |               |
 * | Byte 1   | Length MSB          |    -                                  |   | 3 bytes       |
 * | Byte 2   | Length LSB          |    -                                  | __|               |
 * | Byte 3   | Number of Displays  |    [0x00] to [0xFF]                   |   |               |
 * | Byte 4   | Message Mode        |    [0x00] = Segments; [0x01] = ASCII  |   |               |
 * | Byte 5   | Display Data Start  |    -                                  |   | m_length      | m_totalSize
 * | ...      | Display Data        |    -                                  |   |               |
 * | Byte n   | Display Data End    |    -                                  | __|               |
 * | Byte n+1 | Checksum MSB        |    -                                  |   |               |
 * | Byte n+2 | Checksum LSB        |    -                                  |   | 3 bytes       |
 * | Byte n+3 | End TX              |    [0x03]                             | __|             __|
 * --------------------------------------------------------------------------
 * Length is the ammount of bytes from Byte 3 to Byte n. In other words, the size of (Number of Displays + Message Mode + Display Data) which is always 2 + sizeOf(DisplayData).
 * The Checksum is a simple sum of the value of Bytes 3 to Byte n.
 */
class YNV_SIGNAGE_I2C_MESSAGE
{
    public:
        YNV_SIGNAGE_I2C_MESSAGE();
        
        bool    setLength(uint16_t t_length);                                   // Set the Message Length parameter
        bool    setMessageMode(int t_message_mode);                             // Set the Displays' message mode (ASCII or SEGMENT)
        bool    setInputMode(int t_input_mode);                                 // Set the mode of input for the displays' message
        bool    setNumberOfDisplays(int t_numDisplays);                         // Set the Number of Displays in the chain
        bool    setDisplayData(char t_data, int t_position);                  // Set Display Data in a specific position in the message
        bool    setDisplayData(char * t_data, int t_position, int t_size);      // Set Display Data in a specific position in the message

        uint16_t    getMessageLength(void);
        uint8_t     getMessageMode(void);                                           // Set the displays' message mode (ASCII or SEGMENT)
        uint8_t     getInputMode(void);                                             // Set the mode of input for the displays' message
        uint8_t     getNumberOfDisplays(void);                                      // Get the Number of Displays in the chain
        uint8_t *   getMessage(void);
        uint16_t    getTotalSize(void);

    private:

        uint16_t m_length;
        uint8_t m_messageMode;
        uint8_t m_inputMode;
        uint8_t m_numDisplays;
        uint8_t * m_messageBufferTX;
        uint16_t m_totalSize;

        void calculateTotalSize(void);
};

#endif // _YNVISIBLE_SIGNAGE_KIT