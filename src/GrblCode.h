#ifndef GRBLCODE_H
#define GRBLCODE_H

#include "GrblParserC.h"

#define MAX_STATUS_LEN 10

struct GRBLSTATUS
{
  char      cStatus[MAX_STATUS_LEN];
  int       isMpos;
  double    axis_Position[MAX_N_AXIS];
  double    axis_WCO[MAX_N_AXIS];
  int       spindle;
  uint32_t  spindle_speed;
  uint32_t  feedrate;
};

#endif
