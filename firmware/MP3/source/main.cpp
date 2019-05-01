#include <project_config.hpp>
#include <cstdint>
#include <fstream>
#include <iostream>

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

int main(void)
{
  //Play Music
  return 0;
}
