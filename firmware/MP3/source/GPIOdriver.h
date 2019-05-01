#pragma once 

#include <cstdint>
#include "../../library/L0_LowLevel/LPC40xx.h"
#include "../../library/L0_LowLevel/interrupt.hpp"
  
class LabGPIO
{
    public:
        enum class Direction : uint8_t
        {
            kInput  = 0,
            kOutput = 1
        };
        enum class State : uint8_t
        {
            kLow  = 0,
            kHigh = 1
        };
        enum class Edge
        {
            kNone = 0,
            kRising,
            kFalling,
            kBoth
        };


        /// You should not modify any hardware registers at this point
        /// You should store the port and pin using the constructor.
        ///
        /// @param port - port number between 0 and 5
        /// @param pin - pin number between 0 and 32
        /*constexpr*/ LabGPIO(uint8_t portGPIO, uint8_t pinGPIO);
        /// Sets this GPIO as an input
        void SetAsInput();
        /// Sets this GPIO as an output
        void SetAsOutput();
        /// Sets this GPIO as an input
        /// @param output - true => output, false => set pin to input
        void SetDirection(Direction direction);
        /// Set voltage of pin to HIGH
        void SetHigh();
        /// Set voltage of pin to LOW 
        void SetLow();
        /// Set pin state to high or low depending on the input state parameter.
        /// Has no effect if the pin is set as "input".
        ///
        /// @param state - State::kHigh => set pin high, State::kLow => set pin low
        void set(State state);
        /// Should return the state of the pin (input or output, doesn't matter)
        ///
        /// @return level of pin high => true, low => false
        State Read();
        /// Should return the state of the pin (input or output, doesn't matter)
        ///
        /// @return level of pin high => true, low => false
        bool ReadBool();
        uint8_t getPort();
        uint8_t getPin();

        static constexpr size_t kPorts = 2;
        static constexpr size_t kPins = 32; 
        // This handler should place a function pointer within the lookup table for 
        // the GpioInterruptHandler() to find.
        //
        // @param isr  - function to run when the interrupt event occurs.
        // @param edge - condition for the interrupt to occur on.
        void AttachInterruptHandler(IsrPointer isr, Edge edge);
        static void EnableInterrupts();

        void AdestoSelect();
        void AdestoDeselect();

    private:
        uint8_t port;
     	uint8_t pin;
     	LPC_GPIO_TypeDef *gpioDef;
        /// port, pin and any other variables should be placed here.
        /// NOTE: Pin state should NEVER be cached! Always check the hardware
        ///       registers for the actual value of the pin.
        // Statically allocated a lookup table matrix here of function pointers 
        // to avoid dynamic allocation.
        // 
        // Upon AttachInterruptHandler(), you will store the user's function callback
        // in this matrix.
        //
        // Upon the GPIO interrupt, you will use this matrix to find and invoke the
        // appropriate callback.
        //
        // Initialize everything to nullptr.
        static IsrPointer pin_isr_map[kPorts][kPins];
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
        static void GpioInterruptHandler();
        // Register GPIO_IRQn here
        
};