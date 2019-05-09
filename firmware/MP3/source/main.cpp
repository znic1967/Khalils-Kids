#include <project_config.hpp>
// #include <cstdint>
// #include <fstream>
// #include <iostream>

#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "utility/rtos.hpp"
#include "utility/log.hpp"
#include "utility/time.hpp"
#include "L0_LowLevel/interrupt.hpp"
#include "third_party/FreeRTOS/Source/include/task.h"
#include "third_party/FreeRTOS/Source/include/semphr.h"
#include <cinttypes>
#include <iterator>
#include "L3_Application/commandline.hpp"
#include "L3_Application/commands/rtos_command.hpp"
#include "L3_Application/commands/common.hpp"
#include "L3_Application/commands/lpc_system_command.hpp"
#include "../../library/third_party/fatfs/source/ff.h"

#include "VS1053.h"

LabSpi SpKaiKai;

//SCLK = 1_0 ch0
//MOSI = 1_1 ch1
//MISO = 1_4 ch2
//xDREQ = 4_28 ch3
//xCS = 0_6 ch4
//xDCS = 0_8 ch5

int main(void)
{
  //Play Music
  VS1053 Decoder;
  LabSpi SPIObj;
  LabGPIO CS(0,6);

  Decoder.Initialize(4,28,0,6,0,8);
  Decoder.sineTest(200,200);
  LOG_INFO("Done with the goods");

  // SPIObj.Initialize(8, LabSpi::SPI, 8);
  // CS.SetAsOutput();
  // CS.SetLow();
  // SPIObj.Transfer(0x12);
  // CS.SetHigh();
  // Delay(1);
  // CS.SetLow();
  // SPIObj.Transfer(0x34);
  // CS.SetHigh();
  return 0;
}
