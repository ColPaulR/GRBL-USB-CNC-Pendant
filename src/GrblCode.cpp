#include <Arduino.h>
#include <string.h>
#include "GrblCode.h"

// Create temp pointer that initially points to NULL
GRBLSTATUS * GrblStatus = 0;

// Data parsed from <...> status reports
// void  show_limits(bool probe, const bool* limits, size_t n_axis) {};
void show_state(const char *state)
{
    // Update current GRBL status with new state
    strcpy(GrblStatus->cStatus, state);
};

void show_dro(const pos_t *axes, const pos_t *wcos, bool isMpos, bool *limits, size_t n_axis)
{
    GrblStatus->isMpos = isMpos;
    GrblStatus->nAxis = n_axis
    for (int i = 0; i < n_axis; i++)
    {
            GrblStatus->axis_Position[i] = axes[i];
            GrblStatus->axis_WCO[i] = wcos[i];
    }
}

// void  show_file(const char* filename, file_percent_t percent) {}
void show_spindle_coolant(int spindle, bool flood, bool mist)
{
    GrblStatus->spindle = spindle;
}

void show_feed_spindle(uint32_t feedrate, uint32_t spindle_speed)
{
    GrblStatus->feedrate = feedrate;
    GrblStatus->spindle_speed = spindle_speed;
}

// void  show_overrides(override_percent_t feed_ovr, override_percent_t rapid_ovr, override_percent_t spindle_ovr) {}
// [GC: messages
// void  show_gcode_modes(struct gcode_modes* modes) {}

// Called before and after parsing a status report; useful for
// clearing and updating display screens
void  begin_status_report() {
    GrblStatus = new GRBLSTATUS;
}

void end_status_report()
{
    //   Idle|MPos:151.000,149.000,-1.000|Pn:XP|FS:0,0|WCO:12.000,28.000,78.000
    Serial.printf("<%s|",GrblStatus->state);
    if (GrblStatus->isMpos)
        Serial.printf("MPos:%3.3f",GrblStatus->axis_Position[0]);
    else
        Serial.printf("WPos:%3.3f",GrblStatus->axis_Position[0]);
    for (int i=1;i<GrblStatus->nAxis;i++)
        Serial.printf(",%3.3f",GrblStatus->axis_Position[i]);
    if (GrblStatus->spindle)
        Serial.print("|AS");
    Serial.printf("|FS:%d,%d",GrblStatus->feedrate,GrblStatus->spindle_speed);
    Serial.printf("|WCO:%3.3f",GrblStatus->axis_WCO[0]);

    //Send structure address across fifo
    rp2040.fifo.push_nb((uint32_t) GrblStatus);
}
