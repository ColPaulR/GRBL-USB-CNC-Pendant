#ifndef GRBLCODE_H
#define GRBLCODE_H

#include "GrblParserC.h"

#define MAX_STATUS_LEN 10

struct GrbltStatus
{
  char  cStatus[MAX_STATUS_LEN];
  pos_t axis_WorkPosition[MAX_N_AXIS];
  pos_t axis_MachinePosition[MAX_N_AXIS];
  uint16_t spindle_speed;
};

#endif
