#include <Arduino.h>
// #include "Pendant_WHB04B6.h"
// #include "Adafruit_TinyUSB.h"
#include "Probe.h"
#include "GrblCode.h"

void EndProbeZ(enum ProbeState *probe_state, struct GRBLSTATUS *GrblStatus, struct SAVESTATE* SavedState)
{
    // Don't take any action unless there is a probe in progress
    if (*probe_state != NoProbe)
    {
        // Currently probing. Raise Z to safe move height
        send_command(CMD_SAFE_Z);

        // Goto position saved at start of probe. Assume default feedrate
        String *cmd = new String(CMD_MOVE_M_COORD);
        for (int i = 0; i < GrblStatus->nAxis; i++)
        {
            // Stay at Safe Z unless probing completed successfully
            if ((i != PROBE_AXIS) || (*probe_state == ProbeComplete))
            {
                cmd->concat(GRBLAxisLetters[i]);
                cmd->concat(SavedState->saved_coordinates[i]);
            }
        }
        send_command_strings(cmd);

        // Switch G90/G91 if required
        if (GrblStatus->isG91 != SavedState->savedG91)
            send_command((SavedState->savedG91 ? "G91" : "G90"));

        // Switch G20/G21 if required
        if (GrblStatus->isG21 != SavedState->savedG21)
            send_command((SavedState->savedG21 ? "G21" : "G20"));

        // Done with probing. Set state to NOPROBE
        *probe_state = NoProbe;
    }
}


void ProbeZ(uint8_t state, enum ProbeState *probe_state, struct GRBLSTATUS *GrblStatus, struct SAVESTATE* SavedState)
{
#if (PROBE_STATE_ECHO)
    Serial.printf("Probe State was %d\r\n", this->probe_state);
#endif
    switch (*probe_state)
    {
    case NoProbe:
        // Only probe if controller is not executing something else
        // Entered this state by ProbeZ button press
        if ((state == Idle) || (state == Hold))
        {
            // Start probing
            // Save coordinates
            memcpy(SavedState->saved_coordinates, GrblStatus->axis_Position, GrblStatus->nAxis * sizeof(double));

            // Save modal states
            SavedState->savedG21 = GrblStatus->isG21;
            SavedState->savedG91 = GrblStatus->isG91;

            // Switch to relative move mode if required
            if (!SavedState->savedG91)
                send_command("G91");

            // Switch to mm if required
            if (!SavedState->savedG21)
                send_command("G21");

            // Raise Z to safe move height
            send_command(CMD_SAFE_Z);

            // Goto probe position saved in G30
            send_command(CMD_GOTO_PROBE_XY);

            // Wait for start/pause key press
            *probe_state = MovedToProbeLocation;
            break;
        }
    case MovedToProbeLocation:
        // User pressed Start/Pause button after moving to probe position
        *probe_state = ProbeExistingCoarse;
        send_command(CMD_FAST_PROBE);
        break;
    case ProbeExistingCoarse:
        // Probe has completed existing coarse probe
        *probe_state = ProbeExistingFine;
        send_command(CMD_PROBE_LIFT);
        delay(CMD_DELAY);
        send_command(CMD_SLOW_PROBE);
        break;
    case ProbeExistingFine:
        // Probe has completed existing fine probe
        *probe_state = ProbeExistingComplete;
        // Probe has completed existing probe
        send_command(CMD_SAFE_Z);
        break;
    case ProbeExistingComplete:
        // User pressed Start/Pause button after moving to probe position
        *probe_state = ProbeNewCoarse;
        send_command(CMD_FAST_PROBE);
        break;
    case ProbeNewCoarse:
        // Probe has completed existing coarse probe
        *probe_state = ProbeNewFine;
        send_command(CMD_PROBE_LIFT);
        delay(CMD_DELAY);
        send_command(CMD_SLOW_PROBE);
        break;
    case ProbeNewFine:
        // Probe has completed new probe
        // *************** Do something here *********************
        // WPos = MPos - WCS - G92 - TLO
        //
        // WPos.new should equal WPos.old after toolchange. G92 and TLO should not change
        // WPos.old = MPos.old - WCO

        // Move Z to the location of the probe trigger/remove overshoot
        String *cmd = new String(CMD_MOVE_M_COORD);
        cmd->concat(PROBE_AXIS_CHAR);
        cmd->concat(SavedState->new_tool_z);
        send_command_strings(cmd);
        delay(CMD_DELAY);

        cmd = new String(CMD_RESET_Z);
        // Calculate new WCS for Z
        cmd->concat(SavedState->existing_tool_z - GrblStatus->axis_WCO[PROBE_AXIS]);
        send_command_strings(cmd);

        // Update probe_state
        *probe_state = ProbeComplete;

        // Cleanup probing
        EndProbeZ(probe_state, GrblStatus, SavedState);
        // *************** Do something here *********************
    }
#if (PROBE_STATE_ECHO)
    Serial.printf("Probe State is %d\r\n", *probe_state);
#endif
}

void send_command(const char cCmd[])
{
  String *cmd = new String(cCmd);
  cmd->concat("\n");
  rp2040.fifo.push_nb((uint32_t)cmd);
}

void send_command_strings(String * cmd)
{
  cmd->concat("\n");
  rp2040.fifo.push_nb((uint32_t)cmd);
}