#include "Pendant_WHB04B6.h"
#include "SerialDebug.h"
// For Reference:
// https://github.com/LinuxCNC/linuxcnc/tree/master/src/hal/user_comps/xhc-WHB04B6

Pendant_WHB04B6::Pendant_WHB04B6(uint8_t dev_addr, uint8_t instance) : USBHIDPendant(dev_addr, instance),
                                                                       axis_coordinates{0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
                                                                       display_report_data{0x06, 0xfe, 0xfd, SEED, 0x81, 0x00, 0x00, 0x00,

                                                                                           0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                                           0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
                                                                       jog(0),
                                                                       selected_axis(0),
                                                                       display_axis_offset(0),
                                                                       selected_feed(0),
                                                                       spindle_last(0)
{
    this->send_display_report();
    // DEMO DATA
    // axis_coordinates[0] = -965.2345678;
    // axis_coordinates[1] = 1155.841333;
    // axis_coordinates[2] = -44.8365;
    // axis_coordinates[3] = 34.224378;
    // axis_coordinates[4] = 0;
    // axis_coordinates[5] = 0;
};

Pendant_WHB04B6::~Pendant_WHB04B6()
{
    this->stop_continuous();
}

void Pendant_WHB04B6::report_received(uint8_t const *report, uint16_t len)
{
    if (len != 8 || report[0] != 0x04)
        return;
    uint8_t const &random = report[1];
    uint8_t const &keycode1 = report[2];
    uint8_t const &keycode2 = report[3];
    uint8_t const &feed = report[4];
    uint8_t const &axis = report[5];
    int8_t const &jogdelta = report[6];
    uint8_t const &checksum = report[7];

    uint8_t checksum_test = (keycode1) ? (random - (keycode1 ^ (~SEED & random))) : (random & SEED);
    if (checksum_test != checksum)
        return;

    bool display_update_needed = (keycode1) ? true : false;

    this->jog += jogdelta;

    if (this->selected_feed != feed)
    {
        this->selected_feed = feed;
        this->jog = 0;
        this->stop_continuous();
    }

    if (this->selected_axis != axis)
    {
        this->selected_axis = axis;
        this->display_axis_offset = (axis > AXISSELCTOR_Z) ? 3 : 0;
        this->jog = 0;
        display_update_needed = true;
        this->stop_continuous();
    }

    this->process_keycodes(&keycode1, 2);

    if (display_update_needed)
    {
        this->send_display_report();
    }
}

void Pendant_WHB04B6::double_to_report_bytes(double val, uint8_t idx_intval_lower, uint8_t idx_intval_upper, uint8_t idx_frac_lower, uint8_t idx_frac_upper)
{
    uint16_t intval;
    uint16_t fraction;
    this->display_report_data[R_IDX(idx_frac_upper)] = 0;
    if (val < 0.0)
    {
        val *= -1;
        this->display_report_data[R_IDX(idx_frac_upper)] = (1 << 7);
    }
    intval = (uint16_t)val;
    fraction = (val - (float)intval) * 10000;
    this->display_report_data[R_IDX(idx_intval_lower)] = intval & 0xff;
    this->display_report_data[R_IDX(idx_intval_upper)] = ((intval & 0xff00) >> 8);
    this->display_report_data[R_IDX(idx_frac_lower)] = fraction & 0xff;
    this->display_report_data[R_IDX(idx_frac_upper)] |= ((fraction & 0x7f00) >> 8);
}
void Pendant_WHB04B6::uint16_to_report_bytes(uint16_t val, uint8_t idx_lower, uint8_t idx_upper)
{
    this->display_report_data[R_IDX(idx_lower)] = (val & 0xff);
    this->display_report_data[R_IDX(idx_upper)] = ((val & 0xff00) >> 8);
}

void Pendant_WHB04B6::send_display_report()
{
    if (this->report_packet_next > 0)
        return; // sending already in progress

    this->last_display_report = millis();

#if SERIALDEBUG > 1
    Serial.printf("X:%f,Y:%f,Z%f,A%f\r\n", this->axis_coordinates[0], this->axis_coordinates[1], this->axis_coordinates[2], this->axis_coordinates[3]);
#endif

    // update axis coordinates in display report data
    this->double_to_report_bytes(axis_coordinates[0 + this->display_axis_offset], 4, 5, 6, 7);
    this->double_to_report_bytes(axis_coordinates[1 + this->display_axis_offset], 8, 9, 10, 11);
    this->double_to_report_bytes(axis_coordinates[2 + this->display_axis_offset], 12, 13, 14, 15);

    // update mode indicator in display report data (mode not used yet!)
    uint8_t mode_bits = 0;
    if (this->mode == Mode::Step)
        mode_bits = 0x01;
    this->display_report_data[R_IDX(3)] = (this->display_report_data[R_IDX(3)] & (~0x3)) | mode_bits;

    // Display work coordinates or machine coordinates based on GRBL status response (WPOS or MPOS). To switch, toggle $10
    if (this->machine_coordinates)
        // Clear work coordiantes bit
        this->display_report_data[R_IDX(3)] = (this->display_report_data[R_IDX(3)] & (~0x80));
    else
        // Set work coordiantes bit
        this->display_report_data[R_IDX(3)] = (this->display_report_data[R_IDX(3)] | 0x80);
    // send first packet of display report data to device
    this->set_report();
}

void Pendant_WHB04B6::set_report()
{
    if (this->report_packet_next >= REPORT_PACKET_COUNT)
    {
        // all packets sent
        this->report_packet_next = 0;
        return;
    }
    this->USBHIDPendant::set_report(0x06, HID_REPORT_TYPE_FEATURE, &(this->display_report_data[this->report_packet_next++ * 8]), 8);
}

void Pendant_WHB04B6::set_report_complete(uint8_t report_id, uint8_t report_type, uint16_t len)
{
    if (report_id != 0x06) // wrong report id?
        return;
    if (len != 8) // send failed ?
        this->report_packet_next = 0;
    else
        this->set_report(); // send next packet
}

void Pendant_WHB04B6::loop()
{
    static unsigned long last_cmd_check = millis();
    unsigned long now = millis();
    if ((now - last_cmd_check) > CMD_STEP_INTERVAL)
    {
        last_cmd_check = now;

        // Process any jog actions needed
        uint8_t feed = FEEDSELECTOR_TO_LINEAR(this->selected_feed);
        if (this->mode == Mode::Step && this->jog != 0 && this->selected_axis >= AXISSELCTOR_X && this->selected_axis <= AXISSELCTOR_C && feed && feed <= FEEDSELECTOR_STEP_STEPS)
        {
            float step_size = WHB04B6StepSizes[feed - 1];
            uint8_t axis = this->selected_axis - AXISSELCTOR_X;

            // New stirng with the base jog command
            String *cmd = new String(WHB04B6JogCommands);

            // select axis
            cmd->concat(WHB04B6AxisLetters[axis]);
            // append disance
            cmd->concat(this->jog * step_size);

            // Append feed rate
            cmd->concat("F");
            cmd->concat(WHB04B6ContinuousFeeds[axis]);

#if GRBL_JOG_CMD_ECHO > 1
            Serial.write(cmd->c_str());
#endif

            this->send_command(cmd);
            this->jog = 0;
        }
    }

    // if (this->mode == Mode::Continuous)
    // {
    //     if ((now - last_continuous_check) > CMD_CONTINUOUS_CHECK_INTERVAL)
    //     {
    //         this->handle_continuous_check();
    //     }
    //     if ((now - last_continuous_update) > CMD_CONTINUOUS_UPDATE_INTERVAL)
    //     {
    //         this->handle_continuous_update();
    //     }
    // }

    // Updated display if required
    if ((now - this->last_display_report) > REPORT_INTERVAL)
    {
        this->send_display_report();
    }
}

void Pendant_WHB04B6::on_key_press(uint8_t keycode)
{
#if (KEYPRESS_ECHO)
    Serial.print("Key Press: ");
    Serial.println(keycode, HEX);
#endif

    // Send ButtonCommands
    // Execute regardless of Function key
    switch (keycode)
    {
    case KEYCODE_RESET:
        // Soft Reset
        this->send_command(new String("$Bye"));
        break;
    case KEYCODE_STOP:
        StopButton();
        break;
    case KEYCODE_STARTPAUSE:
        StartPauseButton();
        break;
    case KEYCODE_CONTINUOUS:
        // this->jog = 0;
        // this->mode = Mode::Continuous;
        // this->last_continuous_check = millis();
        // break;
    case KEYCODE_STEP:
        this->jog = 0;
        this->stop_continuous();
        this->mode = Mode::Step;
        break;
    default:
        if (this->is_key_pressed(KEYCODE_FN))
        {
            switch (keycode)
            {
            case KEYCODE_M1_FEEDPLUS:
                this->send_command(new String("\x91"));
                break;
            case KEYCODE_M2_FEEDMINUS:
                this->send_command(new String("\x92"));
                break;
            case KEYCODE_M3_SPINDLEPLUS:
                this->send_command(new String("\x9A"));
                break;
            case KEYCODE_M4_SPINDLEMINUS:
                this->send_command(new String("\x9B"));
                break;
            case KEYCODE_M5_MHOME:
                this->send_command(new String("$H"));
                break;
            case KEYCODE_M6_SAFEZ:
                this->send_command(new String(CMD_SAFE_Z));
                break;
            case KEYCODE_M7_WHOME:
                // Set work home here ; do not set/reset Z
                this->send_command(new String("G10 L20 P0 X0 Y0"));
                break;
            case KEYCODE_M8_SPINDLEONOFF:
                // Execute spindle toggle here
                SpindleToggle();
                break;
            case KEYCODE_M9_PROBEZ:
                // Execute Probe Z here
                if (this->probe_state == ProbeState::NoProbe)
                    ProbeZ();
                break;
#if SERIALDEBUG > 0
            default:
                Serial.print("Not function defined for key press: ");
                Serial.println(keycode, HEX);
#endif
            }
        }
        else
        {
            switch (keycode)
            {
            // macros 1 through 8 are keycodes 4 through 11
            case KEYCODE_M1_FEEDPLUS ... KEYCODE_M8_SPINDLEONOFF:
                RunMacro(keycode - 3);
                break;
            case KEYCODE_M9_PROBEZ:
                RunMacro(9);
                break;
            case KEYCODE_M10:
                RunMacro(10);
                break;
            }
        }
        break;
    }
}

void Pendant_WHB04B6::on_key_release(uint8_t keycode)
{
#if (KEYRELEASE_ECHO)
    Serial.print("Key Release: ");
    Serial.println(keycode, HEX);
#endif
}

void Pendant_WHB04B6::grblstatus_received(GRBLSTATUS *grblstatus)
{
    // Save number of axis currently reported. Expecting nAxis < MAX_N_AXIS
    this->nAxis = grblstatus->nAxis;

#if (GRBLSTATUS_AXIS_ECHO)
    Serial.printf("X:%f,Y:%f,Z%f,A%f\r\n", grblstatus->axis_Position[0], grblstatus->axis_Position[1], grblstatus->axis_Position[2], grblstatus->axis_Position[3]);
#endif
    // Copy current position
    for (uint8_t i = 0; i < (grblstatus->nAxis); i++)
        this->axis_coordinates[i] = grblstatus->axis_Position[i];

    this->uint16_to_report_bytes(grblstatus->spindle_speed, 18, 19);

    // Indicate coordinates on pendant as machine or work. Work appends "1" to axis name
    if (grblstatus->isMpos)
        this->machine_coordinates = true;
    else
        this->machine_coordinates = false;

    // Save state enumeration
    this->state = grblstatus->State;

    // Save spindle enumeration
    this->state = grblstatus->spindle;

    // Save relative/absolute
    this->isG91=grblstatus->isG91;
#if (G91_PROCESSED_ECHO)
    Serial.println(grblstatus->isG91 ? "G91 Rel" : "G90 Abs");
#endif

    // Save units
    this->isG21=grblstatus->isG21;
#if (G21_PROCESSED_ECHO)
    Serial.println(grblstatus->isG21 ? "G21 mm" : "G20 inches");
#endif

    this->send_display_report();
    // Only process probe data if the new data flag is set (should be 1 time event per G38 command)
    if (grblstatus->NewProbeFlag)
    {
#if (PROBE_STATUS_ECHO)
        Serial.println("New probe status received");
#endif
        if (!grblstatus->ProbeSuccessFlag)
        {
            // Figure out what to do if the probing fails during exisitng tool coarse probe
#if (PROBE_STATUS_ECHO)
            Serial.println("Failed probe reported");
#endif
        }
        else
        {
            switch (this->probe_state)
            {
            case ProbeState::ProbeExistingFine:
                // Fine existing tool successfully completed probing
                // Save probe Z value
                this->existing_tool_z = grblstatus->axis_Probe[2];
                ProbeZ();
                break;
            case ProbeState::ProbeNewFine:
                // Fine existing tool successfully completed probing
                // Save probe Z value
                this->new_tool_z = grblstatus->axis_Probe[2];
                ProbeZ();
                break;
            case ProbeState::ProbeNewCoarse:
            case ProbeState::ProbeExistingCoarse:
                ProbeZ();
                break;
                // default:
                // Unexpected probe response not triggered by pendant. Ignore
            }
        }
    }
}
void Pendant_WHB04B6::handle_continuous_check()
{
    this->last_continuous_check = millis();
    if (this->jog == 0)
    {
        this->stop_continuous();
    }
    else
    {
        bool direction = this->jog > 0;
        if (this->continuous_axis == 0 || this->continuous_direction != direction)
        {
            this->continuous_axis = this->selected_axis - AXISSELCTOR_X + 1;
            this->continuous_direction = direction;
            this->handle_continuous_update();
        }
    }
    this->jog = 0;
}
void Pendant_WHB04B6::handle_continuous_update()
{
    this->last_continuous_update = millis();
    uint8_t feed = FEEDSELECTOR_TO_LINEAR(this->selected_feed);
    if (this->continuous_axis && this->continuous_axis <= WHB04B6AxisCount && feed && feed <= FEEDSELECTOR_CONT_STEPS)
    {
        char cmd[100];
        sprintf(cmd, WHB04B6ContinuousRunCommand, WHB04B6AxisLetters[this->continuous_axis - 1], (uint16_t)(WHB04B6ContinuousFeeds[this->continuous_axis - 1] * WHB04B6ContinuousMultipliers[feed - 1]), this->continuous_direction ? 1 : 0);
        String *cmdstr = new String(cmd);
        this->send_command(cmdstr);
    }
}
void Pendant_WHB04B6::stop_continuous()
{
    if (this->continuous_axis)
    {
        this->send_command(new String(WHB04B6ContinuousStopCommand));
        this->continuous_axis = 0;
    }
}
void Pendant_WHB04B6::StartPauseButton()
{
    switch (this->state)
    {
    case State::Cycle:
    case State::Jog:
    case State::Homing:
        // Hold
        this->send_command(new String("!"));
        break;
    case State::Hold:
        // Resume
        this->send_command(new String("~"));
        break;
    case State::Alarm:
        // clear alarm
        this->send_command(new String("$x"));
        break;
    case State::Idle:
        // Awaiting user input/not running any other actions
        switch (this->probe_state)
        {
        case ProbeState::MovedToProbeLocation:
        case ProbeState::ProbeExistingComplete:
            // Probing is awaiting button press
            ProbeZ();
            break;
        }
#if SERIALDEBUG > 0
    default:
        Serial.printf("Pause/run in GRBL State: %d not defined\r\n", this->state);
#endif
    }
}
void Pendant_WHB04B6::StopButton()
{
    switch (this->state)
    {
    case State::Cycle:
        // Stop
        this->send_command(new String("!"));
        // If probing, stop probing and cleanup
        if (this->probe_state != ProbeState::NoProbe)
            EndProbeZ();
        break;
    case State::Jog:
        // Stop
        this->send_command(new String("!"));
        break;
    case State::Homing:
        // Soft Reset
        this->send_command(new String("0x18"));
        break;
#if SERIALDEBUG > 0
    default:
        Serial.printf("Stop Button in GRBL State: %d not defined\r\n", this->state);
#endif
    }
}
void Pendant_WHB04B6::RunMacro(uint8_t MacroNumber)
{
    // Only run macro if controller is not executing something else
    if ((this->state == State::Idle) || (this->state == State::Hold))
    {
        // New string to execute Macro associated with button press
        String *cmd = new String(WHB04B6MacroRunCommand);

        // Append macro number
        cmd->concat(MacroNumber);

        // append extension if required
        cmd->concat(".nc");

// Echo
#if MACRO_EXEC_ECHO > 0
        Serial.write(cmd->c_str());
#endif

        this->send_command(cmd);
    }
}
void Pendant_WHB04B6::SpindleToggle()
{
    if (this->spindle == 0)
    {
        // spindle is stopped
        switch (this->spindle_last)
        {
        case 1:
            // spindle was spinning clockwise last
            this->send_command(new String("M3"));
            break;
        case 2:
            // spindle was spinning counter clockwise last
            this->send_command(new String("M4"));
            break;
        default:
            // spindle_last is uninitialized; do nothing
            break;
        }
    }
    else
    {
        // Spindle is spinning; save directiong and stop
        this->spindle_last = this->spindle;
        this->send_command(new String("M5"));
    }
}
void Pendant_WHB04B6::ProbeZ()
{
#if (PROBE_STATE_ECHO)
    Serial.printf("Probe State was: %d", this->probe_state);
#endif
    switch (this->probe_state)
    {
    case ProbeState::NoProbe:
        // Only probe if controller is not executing something else
        // Entered this state by ProbeZ button press
        if ((this->state == State::Idle) || (this->state == State::Hold))
        {
            // Start probing
            // Save coordinates
            memcpy(this->saved_coordinates, this->axis_coordinates, this->nAxis * sizeof(double));

            // Save modal states
            this->savedG21 = this->isG21;
            this->savedG91 = this->isG91;

            // Switch to relative move mode if required
            if (!this->savedG91)
                this->send_command("G91");

            // Switch to mm if required
            if (!this->savedG21)
                this->send_command("G21");

            // Raise Z to safe move height
            this->send_command(CMD_SAFE_Z);

            // Goto probe position saved in G30
            this->send_command(CMD_GOTO_PROBE_XY);

            // Wait for start/pause key press
            this->probe_state = ProbeState::MovedToProbeLocation;
            break;
        }
    case ProbeState::MovedToProbeLocation:
        // User pressed Start/Pause button after moving to probe position
        this->probe_state = ProbeState::ProbeExistingCoarse;
        this->send_command(CMD_FAST_PROBE);
        break;
    case ProbeState::ProbeExistingCoarse:
        // Probe has completed existing coarse probe
        this->probe_state = ProbeState::ProbeExistingFine;
        this->send_command(CMD_PROBE_LIFT);
        this->send_command(CMD_SLOW_PROBE);
        break;
    case ProbeState::ProbeExistingFine:
        // Probe has completed existing probe
        this->send_command(CMD_SAFE_Z);
        break;
    case ProbeState::ProbeExistingComplete:
        // User pressed Start/Pause button after moving to probe position
        this->probe_state = ProbeState::ProbeNewCoarse;
        this->send_command(CMD_FAST_PROBE);
        break;
    case ProbeState::ProbeNewCoarse:
        // Probe has completed existing coarse probe
        this->probe_state = ProbeState::ProbeNewFine;
        this->send_command(CMD_PROBE_LIFT);
        this->send_command(CMD_SLOW_PROBE);
        break;
    case ProbeState::ProbeNewFine:
        // Probe has completed new probe
        // Move Z to the location of the probe trigger/remove overshoot
        String *cmd = new String(CMD_MOVE_M_COORD);
        cmd->concat("Z");
        cmd->concat(this->new_tool_z);
        this->send_command(cmd);

        // *************** Do something here *********************
        // Send current Z to this->old_tool_z
        // Uncomment cleanup when satisfied with height resetting
        // EndProbeZ();
        // *************** Do something here *********************
    }
#if (PROBE_STATE_ECHO)
    Serial.printf(" is: %d\r\n", this->probe_state);
#endif
}
void Pendant_WHB04B6::EndProbeZ()
{
    // Don't take any action unless there is a probe in progress
    if (this->probe_state != ProbeState::NoProbe)
    {
        // Currently probing. Raise Z to safe move height
        this->send_command(CMD_SAFE_Z);

        // Goto position saved at start of probe. Assume default feedrate
        String *cmd = new String(CMD_MOVE_M_COORD);
        for (int i = 0; i < this->nAxis; i++)
        {
            cmd->concat(WHB04B6AxisLetters[i]);
            cmd->concat(this->saved_coordinates[i]);
        }
        this->send_command(cmd);

        // Switch G90/G91 if required
        if (this->isG91 != this->savedG91)
            this->send_command((this->savedG91 ? "G91" : "G90"));

        // Switch G20/G21 if required
        if (this->isG21 != this->savedG21)
            this->send_command((this->savedG21 ? "G21" : "G20"));

        // Done with probing. Set state to NOPROBE
        this->probe_state = ProbeState::NoProbe;
    }
}
