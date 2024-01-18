#ifndef GRBLCODE_H
#define GRBLCODE_H

#include "GrblParserC.h"

#define MAX_STATUS_LEN 10
struct DuetStatus
{
  char  cStatus[MAX_STATUS_LEN];
  double axis_WorkPosition[6];
  double axis_MachinePosition[6];
  uint16_t spindle_speed;
};

#endif
