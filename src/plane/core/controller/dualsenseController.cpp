#include "Controller.hpp"
#include <hidapi/hidapi.h>
#include <iostream>


DualSense::DualSense(hid_device_info *openDeviceInfo){
    if(openDeviceInfo == NULL){
        std::cout<< "The given openning device infor pointer is NULL" << std::endl;
        std::abort();
    }

    if(openDeviceInfo->product_id != DUALSENSE_CONTROLLER_PRO_ID){
        std::cout<< "Controller type mismathced" << std::endl;
        std::abort();
    }

    this->dev = hid_open_path(openDeviceInfo->path);


    if(this->dev == NULL){
        hid_error(this->dev);
        std::cout << "Open device failed!" << std::endl;
        std::cout << "Try running this software with higher permission" << std::endl;
        exit(EXIT_FAILURE);
    }

    this->inputReport = (struct bluetoothFullInputReportPacket*)std::calloc(1,sizeof(struct bluetoothFullInputReportPacket));
    this->inputReport->reportID = 0x31;

    this->simpleBTInputReport = (struct simpleBluetoothPacket*)std::calloc(1,sizeof(struct simpleBluetoothPacket));

    this->usbInputReport = (struct usbInputReport*)std::calloc(1, sizeof(struct usbInputReport));
    this->usbInputReport->reportID = 0x01;

    // Initialize output report packet
    this->outputReportPayload = (struct setStatePayload*)std::calloc(1,sizeof(struct setStatePayload));
    std::memset(this->outputReportPayload,0,sizeof(struct setStatePayload));
}


void DualSense::closeDualSense(void) {
    // Controller Reset sequence (for USB only!)
    this->outputReportPayload->allowLEDColor = 0;
    for (int i = 0; i < 11; i++) {
        this->outputReportPayload->leftTriggerFFB[i] = 0;
        this->outputReportPayload->rightTriggerFFB[i] = 0;
    }
    this->send();

    free((void*)this->inputReport);
    free((void*)this->simpleBTInputReport);
    free((void*)this->outputReportPayload);
    free((void*)this->usbInputReport);

    hid_close(this->dev);
}


struct inputReportPayload DualSense::getInputReport(uint8_t isUSB){
    if (isUSB == 1) {
        this->usbInputReport->reportID = 0x01;
        int retVal = hid_read(this->dev, (unsigned char*)(this->usbInputReport), sizeof(struct usbInputReport));
        return (this->usbInputReport)->payload;
    }
    else {
        this->inputReport->reportID = 0x31;
        hid_read(this->dev,(unsigned char *)(this->inputReport),sizeof(struct bluetoothFullInputReportPacket));
        return (this->inputReport)->data.payload;
    }
}

struct simpleBluetoothPayload DualSense::getBTSimpleReport(){
    this->simpleBTInputReport->reportID = 0x01;
    hid_get_input_report(this->dev,(unsigned char *)this->simpleBTInputReport,sizeof(struct simpleBluetoothPacket));
    return this->simpleBTInputReport->payload;
}



DualSense& DualSense::setRumbleEmulationFlag(bool flag) {
    this->outputReportPayload->enableRumbleEmulation = flag;
    return *this;    
}

DualSense& DualSense::setHapticLowPassFlag(bool flag) {
    this->outputReportPayload->allowHapticLowpassFilter = flag;
    return *this;
}

DualSense& DualSense::setAllowMotorPowerReductionFlag(bool flag) {
    this->outputReportPayload->motorLevelPowerReduction = flag;
    return *this;
}

DualSense& DualSense::allowTriggerFFB(uint8_t flag){
    if (flag == 1) {
        this->outputReportPayload->allowLeftTriggerFFB = 1;
    }
    else if(flag == 2){
        this->outputReportPayload->allowRightTriggerFFB = 1;
    }
    else if (flag == 3) {
        this->outputReportPayload->allowLeftTriggerFFB = 1;
        this->outputReportPayload->allowRightTriggerFFB = 1;
    }
    return *this;    
}
DualSense& DualSense::enableLEDColor(void){
    this->outputReportPayload->allowLEDColor = 1;
    return *this;    
}
DualSense& DualSense::ResetLights(void){
    this->outputReportPayload->resetLights = 1;
    return *this;    
}

DualSense& DualSense::setRumblePower(uint8_t left, uint8_t right){
    this->outputReportPayload->useRumbleNotHaptics = 1;
    this->outputReportPayload->rumbleEmulationLeft = left;
    this->outputReportPayload->rumbleEmulationLeft = right;
    return *this;    
}
DualSense& DualSense::setHapticFeedBack(uint8_t left, uint8_t right){
    this->outputReportPayload->useRumbleNotHaptics = 0;
    this->outputReportPayload->rumbleEmulationLeft = left;
    this->outputReportPayload->rumbleEmulationLeft = right;
    return *this;    
}
DualSense& DualSense::setMotorPowerReduction(uint8_t reductionFactor,uint8_t useRumbleNotHaptic){
    if(useRumbleNotHaptic & 0x01){
        this->outputReportPayload->rumbleMotorPowerReduction = reductionFactor;
    }else{
        this->outputReportPayload->triggerMotorPowerReduction = reductionFactor;
    }
    return *this;    
}
DualSense& DualSense::setLEDColor(uint8_t r, uint8_t g, uint8_t b){
    return *this;    
}

DualSense& DualSense::setRightTriggerProperty() {
    uint8_t strengthValue = (uint8_t)((8 - 1) & 0x07);
    uint32_t amplitudeZones = 0;
    uint16_t activeZones = 0;
    for (int i = 4; i < 10; i++)
    {
        amplitudeZones |= (uint32_t)(strengthValue << (3 * i));
        activeZones |= (uint16_t)(1 << i);
    }
    
    this->outputReportPayload->rightTriggerFFB[0] = 0x26;
    this->outputReportPayload->rightTriggerFFB[1] = (uint8_t)((activeZones >> 0) & 0xff);
    this->outputReportPayload->rightTriggerFFB[2] = (uint8_t)((activeZones >> 8) & 0xff);
    this->outputReportPayload->rightTriggerFFB[3] = (uint8_t)((amplitudeZones >> 0) & 0xff);
    this->outputReportPayload->rightTriggerFFB[4] = (uint8_t)((amplitudeZones >> 8) & 0xff);
    this->outputReportPayload->rightTriggerFFB[5] = (uint8_t)((amplitudeZones >> 16) & 0xff);
    this->outputReportPayload->rightTriggerFFB[6] = (uint8_t)((amplitudeZones >> 24) & 0xff);
    this->outputReportPayload->rightTriggerFFB[7] = 0x00; 
    this->outputReportPayload->rightTriggerFFB[8] = 0x00; 
    this->outputReportPayload->rightTriggerFFB[9] = 8;
    this->outputReportPayload->rightTriggerFFB[10] = 0x00;
    return *this;
}

DualSense& DualSense::setLeftTriggerProperty() {
    uint8_t     forceValue = (uint8_t)((4 - 1) & 0x07);
    uint32_t    forceZones = 0;
    uint16_t    activeZones = 0;
    for (int i = 4; i < 10; i++)
    {
        forceZones |= (uint32_t)(forceValue << (3 * i));
        activeZones |= (uint16_t)(1 << i);
    }

    
    this->outputReportPayload->leftTriggerFFB[0] = 0x21;
    this->outputReportPayload->leftTriggerFFB[1] = (uint8_t)((activeZones >> 0) & 0xff);
    this->outputReportPayload->leftTriggerFFB[2] = (uint8_t)((activeZones >> 8) & 0xff);
    this->outputReportPayload->leftTriggerFFB[3] = (uint8_t)((forceZones >> 0) & 0xff);
    this->outputReportPayload->leftTriggerFFB[4] = (uint8_t)((forceZones >> 8) & 0xff);
    this->outputReportPayload->leftTriggerFFB[5] = (uint8_t)((forceZones >> 16) & 0xff);
    this->outputReportPayload->leftTriggerFFB[6] = (uint8_t)((forceZones >> 24) & 0xff);
    this->outputReportPayload->leftTriggerFFB[7] = 0x00; // (byte)((forceZones >> 32) & 0xff); // need 64bit for this, but we already have enough space
    this->outputReportPayload->leftTriggerFFB[8] = 0x00; // (byte)((forceZones >> 40) & 0xff); // need 64bit for this, but we already have enough space
    this->outputReportPayload->leftTriggerFFB[9] = 0x00;
    this->outputReportPayload->leftTriggerFFB[10] = 0x00;
    return *this;
}

DualSense& DualSense::send(void){
    struct outputReportPacketUsb packet;
    packet.reportID = 0x02;
    std::memcpy(&(packet.payload), this->outputReportPayload, sizeof(struct setStatePayload));
    int retVal = hid_write(this->dev, (unsigned char*)(&packet), sizeof(struct outputReportPacketUsb));
    return *this;
}
