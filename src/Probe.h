#ifndef PROBE_H
#define PROBE_H

#include "GrblCode.h"

struct SAVEDPROBE
{
    bool savedG91, savedG21;
    double saved_coordinates[MAX_N_AXIS];
    double existing_tool_z, new_tool_z;
};

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

extern void ProbeZ(enum ProbeState probe_state, GRBLSTATUS* GrblStatus, SAVEDPROBE* SavedProbe);
extern void EndProbeZ(enum ProbeState probe_state, GRBLSTATUS* GrblStatus, SAVEDPROBE* SavedProbe);
extern void ProcessPRB(enum ProbeState probe_state, GRBLSTATUS* GrblStatus, SAVEDPROBE* SavedProbe);
extern void send_command_strings(String * );
extern void send_command(const char cCmd[]);

#endif