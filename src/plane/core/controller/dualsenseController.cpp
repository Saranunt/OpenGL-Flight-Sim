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



DualSense& DualSense::enableRumbleEmulation(void){
    this->outputReportPayload->enableRumbleEmulation = 1;
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

DualSense& DualSense::send(void){
    struct outputReportPacketUsb packet;
    packet.reportID = 0x02;
    std::memcpy(&(packet.payload), this->outputReportPayload, sizeof(struct setStatePayload));
    int retVal = hid_write(this->dev, (unsigned char*)(&packet), sizeof(struct outputReportPacketUsb));
    std::cout << retVal << std::endl;
    return *this;
}
