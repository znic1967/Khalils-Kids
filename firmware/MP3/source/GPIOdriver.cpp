#include "LabGPIO.h"
#include "utility/log.hpp"
#include "utility/time.hpp"
#include "../../library/L0_LowLevel/LPC40xx.h"
#include "../../library/L0_LowLevel/interrupt.hpp"

IsrPointer LabGPIO::pin_isr_map[LabGPIO::kPorts][LabGPIO::kPins] = {nullptr};

/*constexpr*/ LabGPIO::LabGPIO(uint8_t portGPIO, uint8_t pinGPIO)
{
	port= portGPIO;
	pin = pinGPIO;
	if (port > 5) 
	{
		LOG_INFO("Invalid Port");

	}
	if (pin > 32) 
	{
		LOG_INFO("Invalid Pin");
	}
	else
	{
		//IsrPointer pin_isr_map[kPorts][kPins] = {nullptr};
		switch (port)
		{
			case 0: 
			{
				gpioDef = LPC_GPIO0; 
				//Set internal pulldowns for switch 0
				LPC_IOCON->P1_19 &= ~((1 << 4) | (1 << 3));  
      			LPC_IOCON->P1_19 |= (1 << 3);
				break;
			}
			case 1:
			{
				gpioDef = LPC_GPIO1; 
				//Set internal pulldowns for switch 1
				LPC_IOCON->P1_15 &= ~((1 << 4) | (1 << 3)); 
      			LPC_IOCON->P1_15 |= (1 << 3);
				break;
			}
			case 2: 
			{
				gpioDef = LPC_GPIO2; 
				break;
			}
			case 3: 
			{
				gpioDef = LPC_GPIO3; 
				break;
			}
			case 4: 
			{
				gpioDef = LPC_GPIO4;
				break;
			}
			case 5: 
			{
				gpioDef = LPC_GPIO5; 
				break;
			}
		} 
	}
}
void LabGPIO::SetAsInput()
{
	gpioDef->DIR &= ~(1 << pin);
}
void LabGPIO::SetAsOutput()
{
	gpioDef->DIR |= (1 << pin);
}
void LabGPIO::SetDirection(Direction direction)
{
	if (direction==Direction::kInput) 
	{
		SetAsInput();
	}
	else 
	{
		SetAsOutput();
	}
}
void LabGPIO::SetHigh()
{
	gpioDef->SET = (1 << pin);
}
void LabGPIO::SetLow()
{
	gpioDef->CLR = (1 << pin);
}
void LabGPIO::set(State state)
{
	if(state==State::kLow)
	{
		SetLow();
	}
	else 
	{
		SetHigh();
	}
}
LabGPIO::State LabGPIO::Read()
{
	State currentState = State::kLow;
	bool statusBit = gpioDef->PIN & (1 << pin);
	if(statusBit) 
	{
		currentState = State::kHigh;
	}
	return currentState;
}
bool LabGPIO::ReadBool()
{
	bool boolState = 0;
	State currentState = Read();
	if (currentState == State::kHigh) 
	{
		boolState = 1;
	}
	return boolState;
}

uint8_t LabGPIO::getPort()
{ 
	return port;
}

uint8_t LabGPIO::getPin() 
{
	return pin;
}
void LabGPIO::AttachInterruptHandler(IsrPointer isr, Edge edge)
{
	pin_isr_map[port][pin] = isr;
	switch(edge)
	{
		case (Edge::kRising):
			LOG_INFO("Rising Edge Interrupts Enabled");
			if(port == 0)
			{
				LPC_GPIOINT->IO0IntEnR |= (1 << pin);
				LPC_GPIOINT->IO0IntEnF &= ~(1 << pin); //Clear Falling
			}
			else
			{
				LPC_GPIOINT->IO2IntEnF &= ~(1 << pin); //Clear Falling
				LPC_GPIOINT->IO2IntEnR |= (1 << pin);
			}
			
			break;
		
		case (Edge::kFalling):
			LOG_INFO("Falling Edge Interrupts Enabled");
			if(port == 0)
			{
				LPC_GPIOINT->IO0IntEnR &= ~(1 << pin); //Clear Rising
				LPC_GPIOINT->IO0IntEnF |= (1 << pin);
			}
			else
			{
				LPC_GPIOINT->IO2IntEnR &= ~(1 << pin); //Clear Rising
				LPC_GPIOINT->IO2IntEnF |= (1 << pin);
			}
			break;
		
		case (Edge::kBoth):
		
			LOG_INFO("Lots of Edges!!!");
			if(port == 0)
			{
				LPC_GPIOINT->IO0IntEnR |= (1 << pin);
				LPC_GPIOINT->IO0IntEnF |= (1 << pin);
			}
			else
			{
				LPC_GPIOINT->IO2IntEnR |= (1 << pin);
				LPC_GPIOINT->IO2IntEnF |= (1 << pin);
			}
			break;
		
		case (Edge::kNone):
			LPC_GPIOINT->IO0IntEnR &= ~(1 << pin);
			LPC_GPIOINT->IO0IntEnF &= ~(1 << pin);
			LPC_GPIOINT->IO2IntEnR &= ~(1 << pin);
			LPC_GPIOINT->IO2IntEnF &= ~(1 << pin);
			break;
	}
}
// This function is invoked by NVIC via the GPIO peripheral asynchronously.
// This ISR should do the following:
//  1) Find the Port and Pin that caused the interrupt via the IO0IntStatF,
//     IO0IntStatR, IO2IntStatF, and IO2IntStatR registers.
//  2) Lookup and invoke the user's registered callback.
//
// VERY IMPORTANT!
//  - Be sure to clear the interrupt flag that caused this interrupt, or this 
//    function will be called repetitively and lock your system.
//  - NOTE that your code needs to be able to handle two GPIO interrupts 
//    occurring at the same time.
void LabGPIO::GpioInterruptHandler()
{
	uint8_t port;
	uint8_t pin;
	if (LPC_GPIOINT->IntStatus & 1)
	{
		port = 0;
		for(pin = 0; pin < 32; pin++)
		{
			if(LPC_GPIOINT->IO0IntStatF & (1 << pin)){
				LPC_GPIOINT->IO0IntClr |= (1 << pin);
				pin_isr_map[port][pin]();
				LOG_INFO("GPIO Falling");

			}
			if(LPC_GPIOINT->IO0IntStatR & (1 << pin)){
				LPC_GPIOINT->IO0IntClr |= (1 << pin);
				pin_isr_map[port][pin]();
				LOG_INFO("GPIO Rising");

			}
		}
	}
	if (LPC_GPIOINT->IntStatus & (1 << 2))
	{
		port = 1; //port map size 2. Port 1 = Port 2
		for(pin = 0; pin < 32; pin++)
		{
			if(LPC_GPIOINT->IO2IntStatF & (1 << pin)){
				LPC_GPIOINT->IO2IntClr |= (1 << pin);
				pin_isr_map[port][pin]();
			}
			if(LPC_GPIOINT->IO2IntStatR & (1 << pin)){
				LPC_GPIOINT->IO2IntClr |= (1 << pin);
				pin_isr_map[port][pin]();
			}
		}
	}
}
void LabGPIO::EnableInterrupts()
{
	RegisterIsr(GPIO_IRQn, GpioInterruptHandler);
	// NVIC_EnableIRQ(GPIO_IRQn);
}

void LabGPIO::AdestoSelect()
{
	gpioDef->CLR = (1 << 10); //Enable Adesto CS from schematic
}
void LabGPIO::AdestoDeselect()
{
  	gpioDef->SET =  (1 << 10); //Enable Adesto CS from schematic
}

