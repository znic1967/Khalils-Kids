#include "L1_Drivers/gpio.hpp"
#include "utility/log.hpp"

int main(void)
{
  // This example code uses the on-board LED and switch, labeled LED0
  // and SW0 respectively on the schematic.  In this source file, there is a
  // pdf labled "SW&LED_Ckts", showing how the circuits that are being used
  // are constructed.
  LOG_INFO("Gpio application starting...");
  Gpio p1_00(1, 0);
  Gpio p1_01(1, 1);

  p1_00.SetAsInput();
  LOG_INFO("Configured port 1, pin 0 as input");

  // Another way to configure the input or output of a pin is to use the
  // method, SetDirection.
  p1_01.SetDirection(GpioInterface::Direction::kOutput);
  LOG_INFO("Configure port 1, pin 1 as output");

  LOG_INFO(
      "Observe how the output of LED0 changes when the input of SW0 is "
      "pressed");
  while (true)
  {
    if (p1_00.Read())
    {
      p1_01.SetLow();
    }
    else
    {
      // Another way to set the output of a pin high or low would be to
      // use the Set method, as demonstrated below.
      p1_01.Set(GpioInterface::State::kHigh);
    }
  }

  return 0;
}
