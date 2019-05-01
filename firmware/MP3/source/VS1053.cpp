#include <VS1053.h>

#include "GPIOdriver.h"
#include "SPIdriver.h"

void VS1053::Initialize(uint8_t dreqPin, uint8_t dreqPort, 
Luint8_t csPort, uint8_t csPin, uint8_t dcsPort, uint8_t dcsPin,)
{
     DREQ = new LabGPIO(dreqPort, dreqPin);
     DREQ->setAsInput();
     xCS  = new LabGPIO(csPort, csPin);
     xCS->setAsOutput();
     xCS->setHigh();
     xDCS = new LabGPIO(dcsPort, dcsPin);
     xDCS->setAsOutput();
     xDCS->setHigh();
     //SPI0.initialize(8, LabSpi0::SPI, 17);
     SPI0.Initialize(8, LabSpi::SPI, 8);
}

uint8_t VS1053::spiread(void)
{
    //return SPI0.transfer(0x00);
    return SPI0.transfer(0x00);
}

uint16_t VS1053::sciRead(uint8_t addr) {
  uint16_t data;
  xCS->setLow();
  spiwrite(0x03);
  spiwrite(addr);
  vTaskDelay(10);
  data = spiread();
  data <<= 8;
  data |= spiread();
  xCSS->setHigh();
  return data;
}

void VS1053::sciWrite(uint8_t addr, uint16_t data)
{
    _CS->setLow();
    spiwrite(0x02);
    spiwrite(addr);
    spiwrite(data >> 8);
    spiwrite(data & 0xFF);
    _CS->setHigh();
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
      SPI.transfer(c[0]);
      c++;
    }
}

void VS1053::soft_reset(void){
    uint16_t mode = sciRead(SCI_MODE);
    mode |= SM_RESET;
    sciWrite(SCI_MODE, mode);
}

void VS1053::sineTest(uint8_t n, uint16_t ms) {
  soft_reset();
  uint16_t mode = sciRead(SCI_MODE);
  mode |= 0x0020;
  sciWrite(SCI_MODE, mode);

  while (DREQ->read()==0);
  xDCS->setLow();
  spiwrite(0x53);
  spiwrite(0xEF);
  spiwrite(0x6E);
  spiwrite(n);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  xDCS->setHigh();
  vTaskDelay(500);
  xDCS->setLow();
  spiwrite(0x45);
  spiwrite(0x78);
  spiwrite(0x69);
  spiwrite(0x74);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  spiwrite(0x00);
  xDCS->setHigh();
  vTaskDelay(500);
}

void VS1053::sendVolume(uint8_t left, uint8_t right) {
  uint16_t volume_to_send;
  volume_to_send = left;
  volume_to_send <<= 8;
  volume_to_send |= right;
  sciWrite(SCI_VOL, volume_to_send);
}
void VS1053::setVolume(uint8_t v)
{
    volume = v;
    float scaled_volume = 254.0 - 154.0*(volume/100.0) - 100.0;
    uint8_t left = (uint8_t)scaled_volume;
    uint8_t right = (uint8_t)scaled_volume;
    sendVolume(left, right);
}

VS1053::VS1053(){}

VS1053::~VS1053(){}