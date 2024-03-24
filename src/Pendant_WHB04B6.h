#ifndef Pendant_WHB04B6_H
#define Pendant_WHB04B6_H

// For Reference:
// https://github.com/LinuxCNC/linuxcnc/tree/master/src/hal/user_comps/xhc-WHB04B6

#include "USBHIDPendant.h"
#include "GrblCode.h"

#define REPORT_INTERVAL 1000
#define CMD_STEP_INTERVAL 100
#define CMD_CONTINUOUS_CHECK_INTERVAL 100
#define CMD_CONTINUOUS_UPDATE_INTERVAL 500
#define REPORT_PACKET_COUNT 3

#define SEED 0xff

#define R_IDX(x) (x + 1 + (x / 7)) // helper to accommodate report package header for relative offset

#define KEYCODE_RESET 0x01
#define KEYCODE_STOP 0x02
#define KEYCODE_STARTPAUSE 0x03
#define KEYCODE_M1_FEEDPLUS 0x04
#define KEYCODE_M2_FEEDMINUS 0x05
#define KEYCODE_M3_SPINDLEPLUS 0x06
#define KEYCODE_M4_SPINDLEMINUS 0x07
#define KEYCODE_M5_MHOME 0x08
#define KEYCODE_M6_SAFEZ 0x09
#define KEYCODE_M7_WHOME 0x0a
#define KEYCODE_M8_SPINDLEONOFF 0x0b
#define KEYCODE_FN 0x0c
#define KEYCODE_M9_PROBEZ 0x0d
#define KEYCODE_CONTINUOUS 0x0e
#define KEYCODE_STEP 0x0f
#define KEYCODE_M10 0x10

#define AXISSELCTOR_OFF 0x06
#define AXISSELCTOR_X 0x11
#define AXISSELCTOR_Y 0x12
#define AXISSELCTOR_Z 0x13
#define AXISSELCTOR_A 0x14
#define AXISSELCTOR_B 0x15
#define AXISSELCTOR_C 0x16

#define FEEDSELECTOR_2P_0001 0x0D
#define FEEDSELECTOR_5P_001 0x0E
#define FEEDSELECTOR_10P_01 0x0F
#define FEEDSELECTOR_30P_1 0x10
#define FEEDSELECTOR_60P 0x1A
#define FEEDSELECTOR_100P 0x1B
#define FEEDSELECTOR_LEAD1 0x9B
#define FEEDSELECTOR_LEAD2 0x1C
#define FEEDSELECTOR_STEP_STEPS 6
#define FEEDSELECTOR_CONT_STEPS 6

#define FEEDSELECTOR_TO_LINEAR(x) ((x >= FEEDSELECTOR_2P_0001 && x <= FEEDSELECTOR_30P_1) ? (x - FEEDSELECTOR_2P_0001 + 1) : ((x >= FEEDSELECTOR_60P && x <= FEEDSELECTOR_100P) ? (x - FEEDSELECTOR_60P + 5) : 0))
#define FEEDSELECTOR_IS_LEAD(x) (x == FEEDSELECTOR_LEAD1 || x == FEEDSELECTOR_LEAD2)

class Pendant_WHB04B6 : public USBHIDPendant
{
public:
  Pendant_WHB04B6(uint8_t dev_addr, uint8_t instance);
  ~Pendant_WHB04B6();
  void report_received(uint8_t const *report, uint16_t len) override;
  void set_report_complete(uint8_t report_id, uint8_t report_type, uint16_t len) override;
  void grblstatus_received(GRBLSTATUS *grblstatus) override;
  void loop() override;

private:
  void send_display_report();
  void set_report();
  void double_to_report_bytes(double val, uint8_t idx_intval_lower, uint8_t idx_intval_upper, uint8_t idx_frac_lower, uint8_t idx_frac_upper);
  void uint16_to_report_bytes(uint16_t val, uint8_t idx_lower, uint8_t idx_upper);
  bool isG91, isG21;
  bool savedG91, savedG21;
  double axis_coordinates[MAX_N_AXIS];
  double probe_coordinates[MAX_N_AXIS];
  double saved_coordinates[MAX_N_AXIS];
  double existing_tool_z, new_tool_z;
  uint8_t nAxis;
  uint8_t display_report_data[24];
  uint8_t report_packet_next = 0;
  uint8_t selected_axis, display_axis_offset, selected_feed, spindle;
  unsigned long last_display_report;
  int16_t jog;
  unsigned machine_coordinates;
  uint8_t spindle_last;
  void on_key_press(uint8_t keycode) override;
  void on_key_release(uint8_t keycode) override;
  void handle_continuous_check();
  void handle_continuous_update();
  void stop_continuous();
  void StopButton();
  void StartPauseButton();
  void RunMacro(uint8_t MacroNumber);
  void SpindleToggle();
  void ProbeZ();
  void EndProbeZ();
  void ProbeZNext();

  uint8_t continuous_axis = 0;
  bool continuous_direction;
  unsigned long last_continuous_check;
  unsigned long last_continuous_update;
  uint8_t state;
  enum class Mode : uint8_t
  {
    Step = 0,
    Continuous
  } mode = Mode::Step;

  enum class ProbeState : uint8_t
  {
    NoProbe,                // Not probing
    MovedToProbeLocation,   // Moving to probe location and awaiting Start/Pause button press to probe existing tool
    ProbeExistingCoarse,    // Coarse probing existing and waiting on probe to complete
    ProbeExistingFine,      // Find probing existing and waiting on probe to complete
    ProbeExistingComplete,  // Move to safe Z and wait for toolchange to complete and user to continue
    ProbeNewCoarse,         // Coarse probing existing and waiting on probe to complete
    ProbeNewFine           // Find probing existing and waiting on probe to complete
  } probe_state = ProbeState::NoProbe;
};

const uint16_t WHB04B6ContinuousFeeds[] = {6000, 6000, 6000, 6000, 6000, 6000};
const float WHB04B6StepSizes[] = {0.001, 0.01, 0.1, 1.0, 10, 100};
const float WHB04B6ContinuousMultipliers[] = {0.02, 0.05, 0.10, 0.30, 0.60, 1.00};
const char WHB04B6AxisLetters[] = {'X', 'Y', 'Z', 'A', 'B', 'C'};
const uint8_t WHB04B6AxisCount = 6;
const char WHB04B6JogCommands[] = "$J=G91";

const uint16_t WHB04B6FeedRateMax[] =
    {
        10000, // X axis
        10000, // Y axis
        1000,  // Z axis
        10000, // axis 4
        10000, // axis 5
        10000  // axis 6
};
const uint16_t WHB04B6FeedRateStep[] =
    {
        500, // X axis
        500, // Y axis
        50,  // Z axis
        500, // axis 4
        500, // axis 5
        500  // axis 6
};

const char WHB04B6ContinuousRunCommand[] = "M98 P\"pendant-continuous-run.g\" A\"%c\" F%u D%u";
const char WHB04B6ContinuousStopCommand[] = "\x85";
const char WHB04B6MacroRunCommand[] = "$LocalFS/Run=P_Macro";
const char CMD_SAFE_Z[] = "G53G0Z0";
const char CMD_MOVE_M_COORD[] = "G53G0";
const char CMD_GOTO_PROBE_XY[] = "G30G91X0Y0";
const char CMD_FAST_PROBE[] = "G53G38.2Z-200F300";
const char CMD_PROBE_LIFT[] = "$J=G91Z20F6000";
const char CMD_SLOW_PROBE[] = "G53G38.2Z-200F75";
// const char WHB04B6MacroRunCommand[] = "$RM=";
#endif
