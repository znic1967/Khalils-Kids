#include "LabGPIO.hpp"
#include "LabSPI.hpp"
#include "VS1003.hpp"
#include "utility/time.hpp"

VS1003::VS1003(LabGPIO* xdcs, LabGPIO* xcs, LabGPIO* rst, LabGPIO* dreq)
{
	XDCS = xdcs;
	XCS = xcs;
	RST = rst;
	DREQ = dreq;
}

bool VS1003::init()
{
	bool status;
  	if((XDCS == NULL) || (XCS == NULL) || (RST == NULL) || (DREQ == NULL))
  	{
  	  	status = false;
  	}
  	else
  	{
	   	XDCS->SetAsOutput();
    	XCS->SetAsOutput();
    	RST->SetAsOutput();
    	DREQ->SetAsInput();

    	XDCS->SetHigh();
    	XCS->SetHigh();
    	RST->SetHigh();

    	// ssp0_init(8);
    	// HardwareReset();
    	// SetMP3Mode();

    	SPI.Initialize(8, LabSpi::FrameModes::kSPI, 48, LabSpi::SPI_Port::kPort1);
    	sciWrite(SCI_REG::kMODE, 0x4800);
    	sciWrite(SCI_REG::kCLOCKF, 0x6000);

    	status = true;
  	}
  	return status;
}

void VS1003::sciWrite(uint8_t address, uint16_t data)
{
	while(DREQ->ReadBool() != 1);
	XCS->SetLow();
	SPI.Transfer(kWrite);
	SPI.Transfer(address);		
	SPI.Transfer(data >> 8);	// Send upper 8 bits
	SPI.Transfer(data & 0xFF);	// Send lower 8 bits
	XCS->SetHigh();
}

uint16_t VS1003::sciRead(uint8_t address)
{
	uint16_t read_data;

	while(DREQ->ReadBool() != 1);
	XCS->SetLow();
	SPI.Transfer(kRead);
	SPI.Transfer(address);

	read_data = SPI.Transfer(0xFF);
	read_data = read_data << 8;
	read_data |= SPI.Transfer(0xFF);
	XCS->SetHigh();

	return read_data;
}

// void VS1003::playSong(char * song_name)
// {
    
// }

void VS1003::sineTest(uint8_t frequency)
{
	XCS->SetLow(); 
    SPI.Transfer(kWrite); 
    SPI.Transfer(kMODE); 
    SPI.Transfer(0x08); 
    SPI.Transfer(0x24);
    XCS->SetHigh();

    Delay(5);

    while(!DREQ->ReadBool());

    Delay(5); 

    XDCS->SetLow();
    SPI.Transfer(0x53); 
    SPI.Transfer(0xef); 
    SPI.Transfer(0x6e); 
    SPI.Transfer(frequency); 
    SPI.Transfer(0x00); 
    SPI.Transfer(0x00); 
    SPI.Transfer(0x00); 
    SPI.Transfer(0x00);
    XDCS->SetHigh();

    Delay(2000); 

    XDCS->SetLow();
    SPI.Transfer(0x45); 
    SPI.Transfer(0x78); 
    SPI.Transfer(0x69); 
    SPI.Transfer(0x74); 
    SPI.Transfer(0x00); 
    SPI.Transfer(0x00); 
    SPI.Transfer(0x00); 
    SPI.Transfer(0x00);
    XDCS->SetHigh();
}

// void MP3_Decoder::SineTest(uint16_t ms, uint8_t frequency)
// {
//   uint8_t sine_test_start[] = {0x53, 0xEF, 0x6E, frequency, 0x00, 0x00, 0x00, 0x00};
//   uint8_t sine_test_stop[] = {0x45, 0x78, 0x69, 0x74, 0x00, 0x00, 0x00, 0x00};
//   uint16_t sm_tests = ReadMode();
//   sm_tests |= 0x0020;

//   SciWrite(SciRegisters::kMode, sm_tests);
//   SendData(sine_test_start, sizeof(sine_test_start));
//   delay_ms(ms);
//   SendData(sine_test_stop, sizeof(sine_test_stop));
// }