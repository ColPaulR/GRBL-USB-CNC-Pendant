#ifndef GRBLCODE_H
#define GRBLCODE_H

#include "GrblParserC.h"

#define MAX_STATUS_LEN 10

struct GRBLSTATUS
{
  char      cStatus[MAX_STATUS_LEN];
  pos_t     axis_WorkPosition[MAX_N_AXIS];
  pos_t     axis_MachinePosition[MAX_N_AXIS];
  int       spindle;
  uint32_t  spindle_speed;
  uint32_t  feedrate;
};

#endif
