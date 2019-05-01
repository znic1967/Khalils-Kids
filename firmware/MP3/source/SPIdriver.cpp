#include "utility/log.hpp"
#include "utility/time.hpp"
#include "../../library/L0_LowLevel/LPC40xx.h"
#include "../../library/L0_LowLevel/interrupt.hpp"
#include "Part1.h"

bool LabSpi::Initialize(uint8_t data_size_select, FrameModes format, uint8_t divide)
{
    //Parts 1,3,5 from datasheet
    LOG_INFO("SPI Begin");
    // Initialize SSP peripheral

    // Enable Power
    LPC_SC->PCONP &= ~(1 << 20);
    LPC_SC->PCONP |= (1 << 20); //powers on SPI (SSP2)copy 
    LOG_INFO("Power is On");

    //set MISO0 fxn
    LPC_IOCON->P1_4 &= ~(0b111 << 0); //clear bit 0-2
    LPC_IOCON->P1_4 |= (0b100 << 0); //set bit 2
    LPC_IOCON->P1_4 &= ~(0b11 << 3); //Disable pullups and downs

    //set MOSI0 fxn
    LPC_IOCON->P1_1 &= ~(0b111 << 0); //clear bit 0-2
    LPC_IOCON->P1_1 |= (0b100 << 0);
    LPC_IOCON->P1_1 &= ~(0b11 << 3); //Disable pullups and downs
    

    // Set serial clock pin
    LPC_IOCON->P1_0 &= ~(0b111 << 0);
    LPC_IOCON->P1_0 |= (0b100 << 0); //clk fxn init

    //Set clock division
    if ((divide%2 == 0) && ((divide > 2) && (divide < 254)))
    {
        LPC_SSP2->CPSR &= ~(0xFF);
        LPC_SSP2->CPSR |= (divide); //Can you set with decimal input
    }

    LPC_SSP2->CR0 &= ~(1 << 6); // is that it ? ? ? ? // CPOL = 0 
    
    LOG_INFO("MISO MOSI and CLK fxn set");

    //Configure SSP Control Register 0

    //LPC_SSP2->CR0 &= ~(0xF); //clear

    switch(data_size_select)
    {
        case 4: 
        {
            LPC_SSP2->CR0 |= (0x3); 
            break;
        }
        case 5: 
        {
            LPC_SSP2->CR0 |= (0x4); 
            break;
        }
        case 6: 
        {
            LPC_SSP2->CR0 |= (0x5);
            break; 
        }
        case 7: 
        {
            LPC_SSP2->CR0 |= (0x6);
            break; 
        }
        case 8: 
        {
            LOG_INFO("8 bit transfer");
            LPC_SSP2->CR0 &= ~(0xF); //clear
            LPC_SSP2->CR0 |= (0x7);
            break; 
        }
        case 9: 
        {
            LPC_SSP2->CR0 |= (0x8); 
            break;        
        }
        case 10: 
        {
            LPC_SSP2->CR0 |= (0x9); 
            break;
        }
        case 11: 
        {
            LPC_SSP2->CR0 |= (0x10); 
            break;
        }
        case 12: 
        {
            LPC_SSP2->CR0 |= (0x11); 
            break;
        }
        case 13: 
        {
            LPC_SSP2->CR0 |= (0x12); 
            break;
        }
        case 14: 
        {
            LPC_SSP2->CR0 |= (0x13); 
            break;
        }
        case 15: 
        {
            LPC_SSP2->CR0 |= (0x14); 
            break;
        }
        case 16: 
        {
            LPC_SSP2->CR0 |= (0x15); 
            break;
        }
    }
    if(format == FrameModes::SPI)
    {
        LPC_SSP2->CR0 &= ~(1 << 5); 
        LPC_SSP2->CR0 &= ~(1 << 4);
    }
    else
    {
        LOG_INFO("Only compatible with SPI fram mode");
        return false;
    }
    
    

    //Maybe mess with SCR

    //Configure SSP Control Register 1
    LPC_SSP2->CR1 &=  ~(1 << 0); //LBM set to normal
    LPC_SSP2->CR1 |= (1<<1); //Enable SSP (SSE)
    LPC_SSP2->CR1 &=  ~(1 << 2); //Set SSP to master

    LOG_INFO("End of Init");
}

uint8_t LabSpi::Transfer(uint8_t send)
{   
  //LPC_GPIO1->CLR = (1 << 8); //SSEL set Low
  // Send data_out and retrieve returned byte
  LPC_SSP2->DR = (send);
  //uint8_t testy = 0x78;

  //LOG_INFO("Data In: %u", send);
  //Enable Adesto?

  while (LPC_SSP2->SR & (1 << 4)) //Check status reg to see if tranfer occuring
  {
    //wait until transfer complete.
  }

  
  //LPC_GPIO1->SET = (1 << 8); //SSEL set High
  uint8_t val;
  val = LPC_SSP2->DR;
  return (val);
 
}