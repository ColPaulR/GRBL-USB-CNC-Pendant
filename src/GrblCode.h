#ifndef GRBLCODE_H
#define GRBLCODE_H

#include "GrblParserC.h"

#define MAX_STATUS_LEN 10

// extern void printtousb(char * cBuffer);

void PrintGrblStatusMsg(struct GRBLSTATUS *GrblStatus);

struct GRBLSTATUS
{
  char      cStatus[MAX_STATUS_LEN];
  int       isMpos;
  int       nAxis;
  double    axis_Position[MAX_N_AXIS];
  double    axis_WCO[MAX_N_AXIS];
  int       spindle;
  uint32_t  spindle_speed;
  bool      flood;
  bool      mist;
  uint32_t  feedrate;
};

#endif
