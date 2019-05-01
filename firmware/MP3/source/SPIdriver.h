#pragma once 
#include <cstdint>
#include "utility/log.hpp"
#include "utility/time.hpp"
#include "../../library/L0_LowLevel/LPC40xx.h"
#include "../../library/L0_LowLevel/interrupt.hpp"

class LabSpi
{
 public:
    enum FrameModes
    {
        SPI,
        TI,
        Microwire,
        not_supported
    };

    typedef union
    {
        uint8_t byte;
        struct 
        {
            uint8_t ready_busy: 1; 
            uint8_t write_enable_latch: 1;
            uint8_t block_protection: 1;
            uint8_t: 1; //reserved for future use
            uint8_t write_protect: 1;
            uint8_t erase_program_error: 1; 
            uint8_t: 1; //reserved for future use
            uint8_t block_protection_locked: 1;
        } __attribute__((packed));
    } statusReg1;

    typedef union
    {
        uint8_t byte;
        struct 
        {
            uint8_t ready_busy: 1; 
            uint8_t: 1; //reserved for future use
            uint8_t: 1; //reserved for future use
            uint8_t: 1; //reserved for future use
            uint8_t reset_enabled: 1;
            uint8_t: 1; //reserved for future use
            uint8_t: 1; //reserved for future use
            uint8_t: 1; //reserved for future use
        } __attribute__((packed));
    } statusReg2;
    /**
     * 1) Powers on SPPn peripheral
     * 2) Set peripheral clock
     * 3) Sets pins for specified peripheral to MOSI, MISO, and SCK
     *
     * @param data_size_select transfer size data width; To optimize the code, look for a pattern in the datasheet
     * @param format is the code format for which synchronous serial protocol you want to use.
     * @param divide is the how much to divide the clock for SSP; take care of error cases such as the value of 0, 1, and odd numbers
     *
     * @return true if initialization was successful
     */
    static bool Initialize(uint8_t data_size_select, FrameModes format, uint8_t divide);

    /**
     * Transfers a byte via SSP to an external device using the SSP data register.
     * This region must be protected by a mutex static to this class.
     *
     * @return received byte from external device via SSP data register.
     */
    static uint8_t Transfer(uint8_t send);
 
 private:
	// Fill in as needed  
};