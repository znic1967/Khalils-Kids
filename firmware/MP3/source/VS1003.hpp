#pragma once

#include "LabGPIO.hpp"
#include "LabSPI.hpp"

class VS1003 {
    public:
        enum INSTRUCTION : uint8_t
        {
            kWrite = 0x02,
            kRead = 0x03
        };

        enum SCI_REG
        {
            kMODE       = 0x0,
            kSTATUS     = 0x1,
            kBASS       = 0x2,
            kCLOCKF     = 0x3,
            kDECODETIME = 0x4,
            kAUDATA     = 0x5,
            kWRAM       = 0x6,
            kWRAMADDR   = 0x7,
            kAIADDR     = 0xA,
            kVOLUME     = 0xB
        };

        VS1003(LabGPIO* data, LabGPIO* select, LabGPIO* reset, LabGPIO* dreq);
        void Initialize();
        bool init();

        uint16_t sciRead(uint8_t address);
        void sciWrite(uint8_t address, uint16_t data);

        void playSong(char * song_name);

        void sineTest(uint8_t frequency);
    private:
        LabGPIO* XDCS; // Find SPI pin to use
        LabGPIO* XCS; // Find SPI pin to use
        LabGPIO* DREQ;
        LabGPIO* RST;
        LabSpi SPI; 
};