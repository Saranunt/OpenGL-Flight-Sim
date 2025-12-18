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

    //this->dev = hid_open(openDeviceInfo->vendor_id,
    //                    openDeviceInfo->product_id,
    //                    openDeviceInfo->serial_number);

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
    this->outputReportBT = (struct outputReportBT*)std::calloc(1,sizeof(struct outputReportBT));
    std::memset(this->outputReportBT,0,sizeof(struct outputReportBT));
    this->outputReportBT->Data.reportId = 0x31;
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
    this->outputReportBT->Data.payload.enableRumbleEmulation = 1;
    return *this;    
}
DualSense& DualSense::allowTriggerFFB(uint8_t flag){
    return *this;    
}
DualSense& DualSense::enableLEDColor(void){
    this->outputReportBT->Data.payload.allowLEDColor = 1;
    return *this;    
}
DualSense& DualSense::ResetLights(void){
    this->outputReportBT->Data.payload.resetLights = 1;
    return *this;    
}

DualSense& DualSense::setRumblePower(uint8_t left, uint8_t right){
    this->outputReportBT->Data.payload.useRumbleNotHaptics = 1;
    this->outputReportBT->Data.payload.rumbleEmulationLeft = left;
    this->outputReportBT->Data.payload.rumbleEmulationLeft = right;
    return *this;    
}
DualSense& DualSense::setHapticFeedBack(uint8_t left, uint8_t right){
    this->outputReportBT->Data.payload.useRumbleNotHaptics = 0;
    this->outputReportBT->Data.payload.rumbleEmulationLeft = left;
    this->outputReportBT->Data.payload.rumbleEmulationLeft = right;
    return *this;    
}
DualSense& DualSense::setMotorPowerReduction(uint8_t reductionFactor,uint8_t useRumbleNotHaptic){
    if(useRumbleNotHaptic & 0x01){
        this->outputReportBT->Data.payload.rumbleMotorPowerReduction = reductionFactor;
    }else{
        this->outputReportBT->Data.payload.triggerMotorPowerReduction = reductionFactor;
    }
    return *this;    
}
DualSense& DualSense::setLEDColor(uint8_t r, uint8_t g, uint8_t b){
    return *this;    
}

DualSense& DualSense::send(void){
    this->outputReportBT->Data.reportId = 0x31;
    this->outputReportBT->Data.enableHID = 1;




    const uint8_t seed = 0xA2;

    boost::crc_32_type crc;

    // Preheat with seed byte (same as crc32(0, &seed, 1))
    crc.process_byte(seed);

    // Process the actual output report buffer
    crc.process_bytes(
        this->outputReportBT->crc.dataBuffer,
        sizeof(this->outputReportBT->crc.dataBuffer));
   
    std::cout << std::hex << crc.checksum() << std::endl;

    this->outputReportBT->crc.crc = crc.checksum();
    hid_send_output_report(this->dev,(unsigned char*)this->outputReportBT,sizeof(struct outputReportBT));
    return *this;
}
