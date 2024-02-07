#ifndef GRBLCODE_H
#define GRBLCODE_H

#include "GrblParserC.h"

#define MAX_STATUS_LEN 10

void PrintGrblStatusMsg(struct GRBLSTATUS *GrblStatus);

struct GRBLSTATUS
{
  uint8_t   State;
  int       isMpos;
  int       nAxis;
  double    axis_Position[MAX_N_AXIS];
  // ignore axis_WCO for now as it is not currently used
  // double    axis_WCO[MAX_N_AXIS];
  int       spindle;
  uint32_t  spindle_speed;
  bool      flood;
  bool      mist;
  uint32_t  feedrate;
};

// From FluidNC/FluidNC/src/Types.h with "enum class" changed to "enum"
//
// System states. The state variable primarily tracks the individual functions
// to manage each without overlapping. It is also used as a messaging flag for
// critical events.
enum State : uint8_t {
    Idle = 0,     // Must be zero.
    Alarm,        // In alarm state. Locks out all g-code processes. Allows settings access.
    CheckMode,    // G-code check mode. Locks out planner and motion only.
    Homing,       // Performing homing cycle
    Cycle,        // Cycle is running or motions are being executed.
    Hold,         // Active feed hold
    Jog,          // Jogging mode.
    SafetyDoor,   // Safety door is ajar. Feed holds and de-energizes system.
    Sleep,        // Sleep state.
    ConfigAlarm,  // You can't do anything but fix your config file.
    Critical,     // You can't do anything but reset with CTRL-x or the reset button
    Run,          // Paul added as it was not listed in FluidNC
    Undefined
};


#endif