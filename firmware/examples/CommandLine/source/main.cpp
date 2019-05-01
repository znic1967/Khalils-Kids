#include <FreeRTOS.h>
#include <task.h>

#include <cstdint>
#include <iterator>

#include "L3_Application/commandline.hpp"
#include "L3_Application/commands/common.hpp"
#include "L3_Application/commands/i2c_command.hpp"
#include "L3_Application/commands/lpc_system_command.hpp"
#include "L3_Application/commands/rtos_command.hpp"
#include "utility/log.hpp"
#include "utility/rtos.hpp"

namespace
{
// In order to create a command line, you will need to create a command list
// object. The value between the <> is the maximum number of commands you want
// the commandline to support. If you are not going to use many commands you may
// reduce this number to reduce the size of the program.
//
// NOTE: This list must be of at least size 2 in order to fit the list and quit
// commands that are already added by the command line class.
CommandList_t<32> command_list;
// This is an i2c command object which can be added to a CommandLine object and
// become apart of the list of commands you can run.
I2cCommand i2c_command;
RtosCommand rtos_command;
LpcSystemInfoCommand system_command;
CommandLine<command_list> ci;
void TerminalTask([[maybe_unused]] void * ptr)
{
  LOG_INFO("Press Enter to Start Command Line!");

  ci.WaitForInput();

  LOG_WARNING("\nUser has quit from terminal!");
  vTaskDelete(nullptr);
}

void BusyTask([[maybe_unused]] void * ptr)
{
  while (true)
  {
    for (int i = 0; i < 200; i++)
    {
      // Busy loop for a while
    }
    vTaskDelay(1);
  }
}

}  // namespace

int main(void)
{
  LOG_INFO("Staring Command Line Application");

  LOG_INFO("Adding common SJTwo commands to command line...");
  AddCommonCommands(ci);

  LOG_INFO("Adding i2c command to command line...");
  i2c_command.Initialize();
  ci.AddCommand(&i2c_command);

  LOG_INFO("Adding system command to command line...");
  ci.AddCommand(&system_command);

  LOG_INFO("Adding rtos command to command line...");
  ci.AddCommand(&rtos_command);

  LOG_INFO("Initializing CommandLine object...");
  ci.Initialize();

  xTaskCreate(TerminalTask, "Terminal", 501, nullptr, rtos::kLow, nullptr);
  xTaskCreate(BusyTask, "BusyTask", 512, nullptr, rtos::kMedium, nullptr);

  vTaskStartScheduler();
  return 0;
}
