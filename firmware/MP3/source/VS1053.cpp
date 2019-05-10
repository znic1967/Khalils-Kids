#include "VS1053.h"

#include "GPIOdriver.h"
#include "SPIdriver.h"

// #define RegisterMap[MODE] 0x00
// #define RegisterMap[VOL]  0x0B
// #define SM_RESET (1 << 2)

//SCLK = 1_0 ch0
//MOSI = 1_1 ch1
//MISO = 1_4 ch2
//xDREQ = 4_28 ch3
//xCS = 0_6 ch4
//xDCS = 0_8 ch5


//Note: s delays converted to normal delays

typedef union
{
  uint16_t byte;
  struct 
  {
    uint16_t SM_DIFF: 1;
    uint16_t SM_SETTOZERO: 1;
    uint16_t SM_RESET: 1;
    uint16_t SM_OUTOFWAV: 1;
    uint16_t SM_PDOWN: 1;
    uint16_t SM_TESTS: 1;
    uint16_t SM_STREAM: 1;
    uint16_t SM_SETTOZERO2: 1;
    uint16_t SM_DACT: 1;
    uint16_t SM_SDIORD: 1;
    uint16_t SM_SDISHARE: 1;
    uint16_t SM_SDINEW: 1;
    uint16_t SM_ADPCM: 1;
    uint16_t SM_ADPCM_HP: 1;
    uint16_t SM_LINE_IN: 1;
  } __attribute__((packed));
} SMMode_t;

typedef enum 
{
    MODE        = 0x0,
    STATUS      = 0x1,
    BASS        = 0x2,
    CLOCKF      = 0x3,
    DECODE_TIME = 0x4,
    AUDATA      = 0x5,
    WRAM        = 0x6,
    WRAMADDR    = 0x7,
    HDAT0       = 0x8,
    HDAT1       = 0x9,
    AIADDR      = 0xA,
    VOL         = 0xB,
    AICTRL0     = 0xC,
    AICTRL1     = 0xD,
    AICTRL2     = 0xE,
    AICTRL3     = 0xF,
    regSize
} sciReg;

typedef enum 
{
    SM_DIFF        = (1 << 0),
    SM_SETTOZERO   = (1 << 1),
    SM_RESET       = (1 << 2),
    SM_OUTOFWAV    = (1 << 3),
    SM_PDOWN       = (1 << 4),
    SM_TESTS       = (1 << 5),
    SM_STREAM      = (1 << 6),
    SM_SETTOZERO2  = (1 << 7),
    SM_DACT        = (1 << 8),
    SM_SDIORD      = (1 << 9),
    SM_SDISHARE    = (1 << 10),
    SM_SDINEW      = (1 << 11),
    SM_ADPCM       = (1 << 12),
    SM_ADCPM_HP    = (1 << 13),
    SM_LINE_IN     = (1 << 14),
} modeReg;

typedef struct
{
    uint8_t  reg_num;
    bool     can_write;
    uint16_t reset_value;
    uint16_t clock_cycles;
    uint16_t reg_value;
} __attribute__((packed)) sciReg_t;

sciReg_t RegisterMap[sciReg::regSize];


void VS1053::Initialize(uint8_t dreqPort, uint8_t dreqPin, 
uint8_t csPort, uint8_t csPin, uint8_t dcsPort, uint8_t dcsPin,
uint8_t rstPort, uint8_t rstPin)
{
     DREQ = new LabGPIO(dreqPort, dreqPin);
     DREQ->SetAsInput();
     xCS  = new LabGPIO(csPort, csPin);
     xCS->SetAsOutput();
     xCS->SetHigh();
     xDCS = new LabGPIO(dcsPort, dcsPin);
     xDCS->SetAsOutput();
     xDCS->SetHigh();
     RST = new LabGPIO(rstPort,rstPin);
     RST->SetAsOutput();
     RST->SetHigh();
     hardReset();
     //SPI0.initialize(8, LabSpi0::SPI, 17);
     SPI.Initialize(8, LabSpi::SPI, 8);
     //setVolume(100);


     // Local register default values
    RegisterMap[MODE]        = { .reg_num=MODE,        .can_write=true,  .reset_value=0x4000, .clock_cycles=80,   .reg_value=0 };
    RegisterMap[STATUS]      = { .reg_num=STATUS,      .can_write=true,  .reset_value=0x000C, .clock_cycles=80,   .reg_value=0 };
    RegisterMap[BASS]        = { .reg_num=BASS,        .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    RegisterMap[CLOCKF]      = { .reg_num=CLOCKF,      .can_write=true,  .reset_value=0x0000, .clock_cycles=1200, .reg_value=0 };
    RegisterMap[DECODE_TIME] = { .reg_num=DECODE_TIME, .can_write=true,  .reset_value=0x0000, .clock_cycles=100,  .reg_value=0 };
    RegisterMap[AUDATA]      = { .reg_num=AUDATA,      .can_write=true,  .reset_value=0x0000, .clock_cycles=450,  .reg_value=0 };
    RegisterMap[WRAM]        = { .reg_num=WRAM,        .can_write=true,  .reset_value=0x0000, .clock_cycles=100,  .reg_value=0 };
    RegisterMap[WRAMADDR]    = { .reg_num=WRAMADDR,    .can_write=true,  .reset_value=0x0000, .clock_cycles=100,  .reg_value=0 };
    RegisterMap[HDAT0]       = { .reg_num=HDAT0,       .can_write=false, .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    RegisterMap[HDAT1]       = { .reg_num=HDAT1,       .can_write=false, .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    RegisterMap[AIADDR]      = { .reg_num=AIADDR,      .can_write=true,  .reset_value=0x0000, .clock_cycles=210,  .reg_value=0 };
    RegisterMap[VOL]         = { .reg_num=VOL,         .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    RegisterMap[AICTRL0]     = { .reg_num=AICTRL0,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    RegisterMap[AICTRL1]     = { .reg_num=AICTRL1,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    RegisterMap[AICTRL2]     = { .reg_num=AICTRL2,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };
    RegisterMap[AICTRL3]     = { .reg_num=AICTRL3,     .can_write=true,  .reset_value=0x0000, .clock_cycles=80,   .reg_value=0 };


}

void VS1053::hardReset() 
{
  RST->SetLow();
  Delay(1);
  RST->SetHigh();
  Delay(2);
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
    uint16_t mode = sciRead(RegisterMap[MODE].reg_num);
    mode |= modeReg::SM_RESET;
    LOG_INFO("mode = 0x%X", mode);
    sciWrite(RegisterMap[MODE].reg_num, mode);
}

void VS1053::sineTest(uint8_t n, uint16_t ms) {
  LOG_INFO("Beginning of SINE TEST BOI");
  soft_reset();
//   LOG_INFO("Death on SCI Read");
//   uint16_t mode = sciRead(RegisterMap[MODE].reg_num);
//   mode |= 0x0020; //allowing SDI tests
//  sciWrite(RegisterMap[MODE].reg_num, mode);
  // //LOG_INFO("Boutta Hit Loop");
  // while (DREQ->Read() == LabGPIO::State::kLow); //Wait until CS is deasserted.
  //  xDCS->SetLow();
  // // LOG_INFO("CS went Low");
  // spiwrite(0x53);
  // spiwrite(0xEF);
  // spiwrite(0x6E);
  // spiwrite(n);
  // spiwrite(0x00);
  // spiwrite(0x00);
  // spiwrite(0x00);
  // spiwrite(0x00);
  // xDCS->SetHigh();
  // Delay(500);
  // xDCS->SetLow();
  // spiwrite(0x45);
  // spiwrite(0x78);
  // spiwrite(0x69);
  // spiwrite(0x74);
  // spiwrite(0x00);
  // spiwrite(0x00);
  // spiwrite(0x00);
  // spiwrite(0x00);
  // xDCS->SetHigh();
  // Delay(500);
}

void VS1053::sendVolume(uint8_t left, uint8_t right) {
  uint16_t volume_to_send;
  volume_to_send = left;
  volume_to_send <<= 8;
  volume_to_send |= right;
  sciWrite(RegisterMap[VOL].reg_num, volume_to_send);
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