#include <string.h>
#include "GrblCode.h"

static struct GRBLSTATUS GrblStatus;

// Data parsed from <...> status reports
// void  show_limits(bool probe, const bool* limits, size_t n_axis) {};
void show_state(const char *state)
{
  // Update current GRBL status with new state
  strcpy(GrblStatus.cStatus, state);
};

void show_dro(const pos_t *axes, const pos_t *wcos, bool isMpos, bool *limits, size_t n_axis)
{
  // WPOS=MPOS-WCO
  // MPOS=WPOS+WCO

  for (int i = 0; i < n_axis; i++)
  {
    if (isMpos)
    {
      // position is in machine coorindates
      GrblStatus.axis_MachinePosition[i] = axes[i];
      GrblStatus.axis_WorkPosition[i] = axes[i] - wcos[i];
    }
    else
    {
      // position is in workspace coorindates
      GrblStatus.axis_MachinePosition[i] = axes[i] + wcos[i];
      GrblStatus.axis_WorkPosition[i] = axes[i];
    }
  }

  // void  show_file(const char* filename, file_percent_t percent) {}
  // void  show_spindle_coolant(int spindle, bool flood, bool mist) {}
  // void  show_feed_spindle(uint32_t feedrate, uint32_t spindle_speed) {}
  // void  show_overrides(override_percent_t feed_ovr, override_percent_t rapid_ovr, override_percent_t spindle_ovr) {}
  // [GC: messages
  // void  show_gcode_modes(struct gcode_modes* modes) {}

  // Called before and after parsing a status report; useful for
  // clearing and updating display screens
  // void  begin_status_report() {}
  // void  end_status_report() {}


  // extract selected data from JSON message and forward to USB routine on other core
  // DuetStatus * duetstatus = new DuetStatus;
  // for(uint8_t i=0;i<6;i++)
  //   duetstatus->axis_userPosition[i] = (double)jsondoc["result"]["move"]["axes"][i]["userPosition"];
  //   duetstatus->spindle_speed = (uint16_t)jsondoc["result"]["spindles"][0]["current"];
  // rp2040.fifo.push_nb((uint32_t)duetstatus);
