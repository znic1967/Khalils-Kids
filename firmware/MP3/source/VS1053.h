#pragma once

//#include "vs10xx_uc.h"
#include "SPIDriver.h"
#include "GPIOdriver.h"
#include <FreeRTOS.h>
#include "task.h"
//#include "storage.hpp"
#include <stdio.h>
#include <string.h>


class VS1053 {
public:
    void Initialize(uint8_t dreqPin, uint8_t dreqPort, 
    uint8_t csPort, uint8_t csPin, uint8_t dcsPort, uint8_t dcsPin,
    uint8_t rstPort, uint8_t rstPin);
    void spiwrite(uint8_t c);
    void spiwrite(uint8_t *c, uint16_t num);
    uint8_t spiread(void);
    void sciWrite(uint8_t addr, uint16_t data);
    uint16_t sciRead(uint8_t addr);
    void sineTest(uint8_t n, uint16_t ms);
    void soft_reset(void);
    void sendVolume(uint8_t left, uint8_t right);
    void setVolume(uint8_t vol);
    void setBalance(uint8_t bal);
    void hardReset();
    virtual ~VS1053();
    VS1053();
    LabGPIO* DREQ;
    LabGPIO* xCS;
    LabGPIO* xDCS;
    LabGPIO* RST;
    LabSpi SPI;
    uint8_t volume;
};