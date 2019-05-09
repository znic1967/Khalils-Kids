#include <VS1053.h>

#include "GPIOdriver.h"
#include "SPIdriver.h"

#define SCI_MODE 0x00
#define SCI_VOL  0x08
#define SM_RESET (1 << 2)

//SCLK = 1_0 ch0
//MOSI = 1_1 ch1
//MISO = 1_4 ch2
//xDREQ = 4_28 ch3
//xCS = 0_6 ch4
//xDCS = 0_8 ch5


//Note: s delays converted to normal delays

void VS1053::Initialize(uint8_t dreqPort, uint8_t dreqPin, 
uint8_t csPort, uint8_t csPin, uint8_t dcsPort, uint8_t dcsPin)
{
     DREQ = new LabGPIO(dreqPort, dreqPin);
     DREQ->SetAsInput();
     xCS  = new LabGPIO(csPort, csPin);
     xCS->SetAsOutput();
     xCS->SetHigh();
     xDCS = new LabGPIO(dcsPort, dcsPin);
     xDCS->SetAsOutput();
     xDCS->SetHigh();
     //SPI0.initialize(8, LabSpi0::SPI, 17);
     SPI.Initialize(8, LabSpi::SPI, 8);
     setVolume(100);
}

uint8_t VS1053::spiread(void)
{
    //return SPI0.transfer(0x00);
    return SPI.Transfer(0x00);
}

uint16_t VS1053::sciRead(uint8_t addr) {
  LOG_INFO("SCI Read");
  uint16_t data;
  xCS->SetLow();
  spiwrite(0x03);
  spiwrite(addr);
  //LOG_INFO("SPIwrite death");
  Delay(10);
  data = spiread();
  data <<= 8;
  data |= spiread();
  xCS->SetHigh();
  return data;
}

void VS1053::sciWrite(uint8_t addr, uint16_t data)
{
    xCS->SetLow();
    spiwrite(0x02);
    spiwrite(addr);
    spiwrite(data >> 8);
    spiwrite(data & 0xFF);
    xCS->SetHigh();
}

void VS1053::spiwrite(uint8_t c)
{
  uint8_t x __attribute__ ((aligned (32))) = c;
  spiwrite(&x, 1);
}

void VS1053::spiwrite(uint8_t *c, uint16_t num)
{
    while (num--)
    {
      //SPI0.transfer(c[0]);
      SPI.Transfer(c[0]);
      c++;
    }
}

void VS1053::soft_reset(void){
    uint16_t mode = sciRead(SCI_MODE);
    mode |= SM_RESET;
    LOG_INFO("mode = 0x%X", mode);
    sciWrite(SCI_MODE, mode);
}

void VS1053::sineTest(uint8_t n, uint16_t ms) {
  LOG_INFO("Beginning of SINE TEST BOI");
  soft_reset();
  //LOG_INFO("Death on SCI Read");
  uint16_t mode = sciRead(SCI_MODE);
  mode |= 0x0020; //allowing SDI tests
  sciWrite(SCI_MODE, mode);
  //LOG_INFO("Boutta Hit Loop");
  while (DREQ->Read() == LabGPIO::State::kLow); //Wait until CS is deasserted.
   xDCS->SetLow();
  // LOG_INFO("CS went Low");
  spiwrite(0x53);
  spiwrite(0xEF);
  spiwrite(0x6E);
  spiwrite(n);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  xDCS->SetHigh();
  Delay(500);
  xDCS->SetLow();
  spiwrite(0x45);
  spiwrite(0x78);
  spiwrite(0x69);
  spiwrite(0x74);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  xDCS->SetHigh();
  Delay(500);
  LOG_INFO("SINE TEST COMPLETE");
}

void VS1053::sendVolume(uint8_t left, uint8_t right) {
  uint16_t volume_to_send;
  volume_to_send = left;
  volume_to_send <<= 8;
  volume_to_send |= right;
  sciWrite(SCI_VOL, volume_to_send);
}
void VS1053::setVolume(uint8_t v) //V {0, 100}
{
    volume = v;
    float scaled_volume = 254.0 - 154.0*(volume/100.0) - 100.0;
    uint8_t left = (uint8_t)scaled_volume;
    uint8_t right = (uint8_t)scaled_volume;
    sendVolume(left, right);
}

VS1053::VS1053(){}

VS1053::~VS1053(){}