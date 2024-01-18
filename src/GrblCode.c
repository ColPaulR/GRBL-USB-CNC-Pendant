#include "GrblCode.h"

static GRBLSTATUS GrblStatus;
  
  // extract selected data from JSON message and forward to USB routine on other core
  // DuetStatus * duetstatus = new DuetStatus;
  // for(uint8_t i=0;i<6;i++)
  //   duetstatus->axis_userPosition[i] = (double)jsondoc["result"]["move"]["axes"][i]["userPosition"];
  //   duetstatus->spindle_speed = (uint16_t)jsondoc["result"]["spindles"][0]["current"];
  // rp2040.fifo.push_nb((uint32_t)duetstatus);
