#ifndef DUALSENS_TYPE_H
#define DUALSENSE_TYPE_H

#include <cstdint>

#define DUALSENSE_CONTROLLER_PRO_ID                 (unsigned short)3302
#define SONY_INTERACTIVE_ENTERTAINMENT_VENDOR_ID 	(unsigned short)1356

#define SIMPLE_INPUT_REPORT 0
#define FULL_INPUT_REPORT 1

//-----------------------------
//-- CRC seed definition
//-----------------------------
#define INPUT_REPORT_SEED   0xA1
#define OUTPUT_REPORT_SEED  0xA2
#define FEATURE_REPORT_SEED 0xA3


//-----------------------------------------------------------------------
//-- Definition of input report hid data
//-----------------------------------------------------------------------

//------------------------------------------
//-- Enumurations for status and option
//------------------------------------------

    enum direction : uint8_t {
        north = 0,
        northEast,
        east,
        southEast,
        south,
        southWest,
        west,
        northWest,
        none
    }; 

    enum powerState : uint8_t {
        discharging         =   0x00,
        charging            =   0x01,
        complete            =   0x02,
        abnormalVoltage     =   0x0A,
        abnormalTemperature =   0x0B,
        chargingError       =   0x0F
    };

    enum muteLight : uint8_t {
        off = 0,
        on,
        breathing,
        doNothing,

        noAction4,
        noAction5,
        noAction6,
        noAction7 = 7
    };

    enum lightBrightness : uint8_t {
        bright = 0,
        mid,
        dim,

        noLightAction3,
        noLightAction4,
        noLightAction5,
        noLightAction6,
        noLightAction7 = 7
    };


    //-- fadeIn  : fade from black to blue
    //-- fadeOut : fade from blue to black 
    //------------------------------------
    enum lightFadeAnimation : uint8_t {
        nothing = 0 ,
        fadeIn,
        fadeOut
    };

//------------------------------------------
//-- Template to fit the packet to the CRC 
//-- calculation for bluetooth communication
//------------------------------------------
#pragma pack(1)
template<int N> struct bluetoothCRC{
    uint8_t     dataBuffer[N-4];
    uint32_t    crc;
};


//--------------------------------
//-- Input report data type
//--------------------------------
    #pragma pack(1)
    struct touchFingerData {
        uint32_t index          :   7;
        uint32_t notTouching    :   1;
        uint32_t fingerX        :   12;
        uint32_t fingerY        :   12;    
    };

    #pragma pack (1)
    struct touchData {
        struct  touchFingerData finger[2];
        uint8_t timeStamp;
    };


//--------------------------------
//-- USB input report payload
//--------------------------------
    #pragma pack(1)
    struct inputReportPayload{
        // analog stick value;
        uint8_t analogLeftX;
        uint8_t analogLeftY;
        uint8_t analogRightX;
        uint8_t analogRightY;

        //Trigger
        uint8_t triggerLeft;
        uint8_t triggerRight;
        uint8_t seqNumber;

        direction   dpad : 4;
        uint8_t     buttonSquare    :  1;
        uint8_t     buttonCross     :  1;
        uint8_t     buttonCircle    :  1;
        uint8_t     buttonTriangle  :  1;

        uint8_t     buttonL1        :  1;
        uint8_t     buttonR1        :  1;
        uint8_t     buttonL2        :  1;
        uint8_t     buttonR2        :  1;
        uint8_t     buttonCreate    :  1;
        uint8_t     buttonOption    :  1;
        uint8_t     buttonL3        :  1;
        uint8_t     buttonR3        :  1;
        
        uint8_t     buttonHome              :  1;
        uint8_t     buttonPad               :  1;
        uint8_t     buttonMute              :  1;
        uint8_t     buttonUNK1              :  1;
        uint8_t     buttonLeftFunction      :  1;
        uint8_t     buttonRightFunction     :  1;
        uint8_t     buttonLeftPaddle        :  1;
        uint8_t     buttonRightPaddle       :  1;


        uint8_t     UNK1;
        uint32_t    UNK_COUNTER1;

        int16_t     angularVelocityX;
        int16_t     angularVelocityY;
        int16_t     angularVelocityZ;
        int16_t     accerlerometerX;
        int16_t     accerlerometerY;
        int16_t     accerlerometerZ;

        uint32_t    sensorTimeStamp;
        int8_t      temperature;

        // Reserved 8 bytes for touch data (Not use for now)
        struct touchData    trackpadTouchData;

        uint8_t     triggerRightStopLocation    :   4;
        uint8_t     triggerRightStatus          :   4;
        uint8_t     triggerLeftStopLocation     :   4;
        uint8_t     triggerLeftStatus           :   4;

        // Vibration effect stuff
        //------------------------
        uint32_t    hostTimeStamp;
        uint8_t     triggerRightEffect          :   4;
        uint8_t     triggerLeftEffect           :   4; 

        // Controller power monitoring data
        //----------------------------
        uint32_t    deviceTimeStamp;
        uint8_t     powerPercent            :   4; 
        powerState  controllerPowerState    :   4;

        // Controller io connectivity data
        //----------------------------
        uint8_t     isHeadphonePlugged      :   1;
        uint8_t     isMicPlugged            :   1;
        uint8_t     isMicMuted              :   1;
        uint8_t     isUSBDataPlugged        :   1;
        uint8_t     isUSBPowerPlugged       :   1;
        uint8_t     isUSBPowerOnBluetooth   :   1;  // Asssert (set to 1) when connect with blutooth while charching the controller
        uint8_t     isDockDetected          :   1;
        uint8_t     unkonwnFlag             :   1;

        uint8_t     isExternalMicPlugged    :   1;
        uint8_t     isHapticLowpassActive   :   1;
        uint8_t     unknownFlag2            :   6;

        uint8_t     aesCmac[8];
    };

    #pragma pack(1)
    struct bluetoothInputReport {
        struct      inputReportPayload payload;
        uint8_t     UNK_RUMBLE_DATA;
        uint8_t     blueToothCrcFailedCount;
        uint8_t     UNK2;
        uint8_t     UNK3;
        uint8_t     padding[9];
    };

    //-- Full bluetooth input report data packet
    #pragma pack(1)
    struct  bluetoothFullInputReportPacket{
        union 
        {
            struct bluetoothCRC<78> crc;
            struct
            {
                uint8_t reportID;
                uint8_t hasHID  : 1;
                uint8_t hasMIC  : 1;
                uint8_t UNK     : 2;
                uint8_t seqNo   : 4;
                struct bluetoothInputReport data;
            };
        };
        
    };


//--------------------------------
//-- simple blutooth payload
//--------------------------------   
#pragma pack(1)
struct simpleBluetoothPayload{
        // analog stick value;
        uint8_t analogLeftX;
        uint8_t analogLeftY;
        uint8_t analogRightX;
        uint8_t analogRightY;

        direction   dpad : 4;
        uint8_t     buttonSquare    :  1;
        uint8_t     buttonCross     :  1;
        uint8_t     buttonCircle    :  1;
        uint8_t     buttonTriangle  :  1;

        uint8_t     buttonL1        :  1;
        uint8_t     buttonR1        :  1;
        uint8_t     buttonL2        :  1;
        uint8_t     buttonR2        :  1;
        uint8_t     buttonCreate    :  1;
        uint8_t     buttonOption    :  1;
        uint8_t     buttonL3        :  1;
        uint8_t     buttonR3        :  1;
        
        uint8_t     buttonHome      :  1;
        uint8_t     buttonPad       :  1;
        uint8_t     counter         :  6;

        uint8_t     triggerLeft;
        uint8_t     triggerRight;
};

#pragma pack(1)
struct simpleBluetoothPacket {
    uint8_t reportID;
    struct simpleBluetoothPayload payload;
};

//--------------------------------
//-- output report payload
//--------------------------------   

struct setStatePayload{
    uint8_t enableRumbleEmulation       :   1;
    uint8_t useRumbleNotHaptics         :   1;
    uint8_t allowRightTriggerFFB        :   1;
    uint8_t allowLeftTriggerFFB         :   1;
    uint8_t allowHeadphoneVolume        :   1;
    uint8_t allowSpeakerVolume          :   1;
    uint8_t allowMicVolume              :   1;
    uint8_t allowAudioControl           :   1;

    uint8_t allowMuteLight              :   1;
    uint8_t allowAudioMute              :   1;
    uint8_t allowLEDColor               :   1;
    uint8_t resetLights                 :   1;
    uint8_t allowPlayerIndicators       :   1;
    uint8_t allowHapticLowpassFilter    :   1;
    uint8_t motorLevelPowerReduction    :   1;
    uint8_t allowAudioControl2          :   1;

    uint8_t rumbleEmulationRight;
    uint8_t rumbleEmulationLeft;

    uint8_t volumeHeadPhones;
    uint8_t volumeSpeaker;
    uint8_t volumeMic;

    //Audio control section
    uint8_t micSelect                   :   2;
    // 0 Auto
    // 1 Internal only
    // 2 External only
    // 3 Not properly documented
    uint8_t echoCancelEnable: 1;
    uint8_t noiseCancelEnable: 1;
    uint8_t outputPathSelect: 2; 
    uint8_t inputPathSelect             : 2;

    enum muteLight muteLightMode;

    //Mute control section
    uint8_t touchPowerSave              :   1;
    uint8_t motionPowerSave             :   1;
    uint8_t hapticPowerSave             :   1;
    uint8_t audioPowerSave              :   1;
    uint8_t micMute                     :   1;
    uint8_t speakerMute                 :   1;
    uint8_t headphoneMute               :   1;
    uint8_t hapticMute                  :   1; // This flag called "bullet mute" in the reference document

    uint8_t rightTriggerFFB[11];
    uint8_t leftTriggerFFB[11];
    uint32_t hostTimeStamp;             // mirror into input report
    
    uint8_t triggerMotorPowerReduction  : 4;
    uint8_t rumbleMotorPowerReduction   : 4;

    //Audio control section 2
    uint8_t speakerCompPreGain  : 3; // For boosting the controller speaker
    uint8_t beamFormingEnable   : 1;
    uint8_t unknowAudioControl2  : 4;

    uint8_t allowLightsBrightnessChange :1 ;
    uint8_t allowColorLightFadeAnimation : 1;
    uint8_t enanbleImprovedRumbleEmulation :1;
    uint8_t UNKBITC: 5;

    uint8_t hapticLowpassFilter : 1;
    uint8_t UNKBIT: 7;
    uint8_t UNKBYTE; 

    enum lightFadeAnimation LEDfadeAnimation;
    enum lightBrightness    brightness;

    uint8_t playerLight1    : 1;
    uint8_t playerLight2    : 1;
    uint8_t playerLight3    : 1;
    uint8_t playerLight4    : 1;
    uint8_t playerLight5    : 1;
    uint8_t playerLightFade : 1;
    uint8_t playerLightUNK  : 2;

    uint8_t ledR;
    uint8_t ledG;
    uint8_t ledB;
};

// Report ID must set to 0x02
struct outputReportPacketUsb{
    uint8_t reportID;
    struct setStatePayload payload;
};

//Output report for bluetooth communication
#pragma pack(1)
struct outputReportBT{
    union{
        struct bluetoothCRC<78> crc;
        struct{
            uint8_t reportId;
            uint8_t UNK1 : 1;
            uint8_t enableHID : 1;
            uint8_t UNK2 : 1;
            uint8_t UNK3 : 1;
            uint8_t seqNo: 4;
            struct setStatePayload payload;
        }Data;
    };
};


#endif
