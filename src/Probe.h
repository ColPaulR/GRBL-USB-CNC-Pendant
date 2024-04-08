#ifndef PROBE_H
#define PROBE_H

#include <string.h>
#include "GrblCode.h"


// use the Z axis, 3rd (index = 2) coordinate for probing
#define PROBE_AXIS 2
#define PROBE_AXIS_CHAR "Z"
#define CMD_DELAY 1000

const char GRBLAxisLetters[] = {'X', 'Y', 'Z', 'A', 'B', 'C'};
const char CMD_SAFE_Z[] = "G53G0Z0";
const char CMD_MOVE_M_COORD[] = "G53G0";
const char CMD_GOTO_PROBE_XY[] = "G30G91X0Y0";
const char CMD_FAST_PROBE[] = "G53G38.2Z-200F300";
const char CMD_PROBE_LIFT[] = "$J=G91Z2F6000";
const char CMD_SLOW_PROBE[] = "G53G38.2Z-200F75";
const char CMD_RESET_Z[] = "G10L20P0Z";

enum ProbeState
{
    NoProbe,               // Not probing
    MovedToProbeLocation,  // Moving to probe location and awaiting Start/Pause button press to probe existing tool
    ProbeExistingCoarse,   // Coarse probing existing and waiting on probe to complete
    ProbeExistingFine,     // Find probing existing and waiting on probe to complete
    ProbeExistingComplete, // Move to safe Z and wait for toolchange to complete and user to continue
    ProbeNewCoarse,        // Coarse probing existing and waiting on probe to complete
    ProbeNewFine,          // Find probing existing and waiting on probe to complete
    ProbeComplete          // Finished probing; safe to move back to original Z work position
};

struct SAVESTATE
{
    double saved_coordinates[MAX_N_AXIS];
    bool savedG91, savedG21;
    double existing_tool_z, new_tool_z;
};

extern void ProbeZ(uint8_t state, enum ProbeState *probe_state, struct GRBLSTATUS *GrblStatus, struct SAVESTATE *SaveState);
extern void EndProbeZ(enum ProbeState *probe_state, struct GRBLSTATUS *GrblStatus, struct SAVESTATE* SavedState);

extern void send_command(const char cCmd[]);
extern void send_command_strings(String * cmd);

#endif
