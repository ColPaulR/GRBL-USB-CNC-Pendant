#include <Arduino.h>
#include <string.h>
#include "GrblCode.h"

// Create static structure to store current state
GRBLSTATUS GrblStatus;

// Data parsed from <...> status reports
// void  show_limits(bool probe, const bool* limits, size_t n_axis) {};
void show_state(const char *state)
{
    // Update current GRBL status with new state
    strcpy(GrblStatus.cStatus, state);
};

void show_dro(const pos_t *axes, const pos_t *wcos, bool isMpos, bool *limits, size_t n_axis)
{
    GrblStatus.isMpos = isMpos;
    GrblStatus.nAxis = n_axis;
    for (int i = 0; i < n_axis; i++)
    {
        GrblStatus.axis_Position[i] = axes[i];
        // GrblStatus.axis_WCO[i] = wcos[i];
    }
}

// void  show_file(const char* filename, file_percent_t percent) {}

void show_feed_spindle(uint32_t feedrate, uint32_t spindle_speed)
{
    GrblStatus.feedrate = feedrate;
    GrblStatus.spindle_speed = spindle_speed;
}

void show_spindle_coolant(int spindle, bool flood, bool mist)
{
    GrblStatus.spindle = spindle;
    GrblStatus.flood = flood;
    GrblStatus.mist = mist;
}

// void  show_overrides(override_percent_t feed_ovr, override_percent_t rapid_ovr, override_percent_t spindle_ovr) {}
// [GC: messages
// void  show_gcode_modes(struct gcode_modes* modes) {}

// Called before and after parsing a status report; useful for
// clearing and updating display screens
// void begin_status_report()
// {
//     GrblStatus = new GRBLSTATUS;
// }

void end_status_report()
{
    // Send a newly allocated structure that is initialize with current status
    rp2040.fifo.push_nb((uint32_t)new GRBLSTATUS(GrblStatus));
}

// [GC: messages
void show_gcode_modes(struct gcode_modes* modes) {
    // update GrblStatus with new modes
   
    // Spindle values
    if (!strcmp((modes->spindle),"Off"))
        GrblStatus.spindle = 0;
    if (!strcmp((modes->spindle),"CW"))
        GrblStatus.spindle = 1;
    if (!strcmp((modes->spindle),"CCW"))
        GrblStatus.spindle = 2;

    // Coolant values
    if (!strcmp(modes->coolant,"Off"))
    {
        GrblStatus.mist = 0;
        GrblStatus.flood = 0;
    }
    if (!strcmp(modes->coolant,"Mist"))
    {
        GrblStatus.mist = 1;
    }
    if (!strcmp(modes->coolant,"Flood"))
    {
        GrblStatus.flood = 1;
    }

    // Send a newly allocated structure that is initialize with current status
    rp2040.fifo.push_nb((uint32_t)new GRBLSTATUS(GrblStatus));
}

