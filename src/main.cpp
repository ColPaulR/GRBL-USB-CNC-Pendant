#include <Arduino.h>

#include "GrblParserC.h"
#include "USBHIDPendant.h"

#define GRBLSerial Serial1 // UART0
#define GRBLSerialTXPin 12
#define GRBLSerialRXPin 13
#define GRBLSerialBaud 57600

#define DUET_QUERY_INTERVAL 1000
//GRBLSerial output(GRBLSerial);

void setup()
{
  Serial.begin(115200); // USB Serial for debug output

  GRBLSerial.setTX(GRBLSerialTXPin);
  GRBLSerial.setRX(GRBLSerialRXPin);

  // while ( !Serial ) delay(10);   // wait for native usb

  Serial.println("TinyUSB Dual Device Info Example");

  // Send ready state and wait for other core
  Serial.println("Core0 Ready");
  rp2040.fifo.push(0);
  rp2040.fifo.pop();
  Serial.println("Core0 Start");

  GRBLSerial.begin(GRBLSerialBaud);
}

void loop()
{
  // check for G-Code command from USB routine on other core
  while (rp2040.fifo.available())
  {
    String *cmd;
    cmd = (String *)rp2040.fifo.pop();
    // send to GRBL Host
    GRBLSerial.print(*cmd);
    // Echo to debug port
    Serial.print(*cmd);
    delete cmd;
  }

  // read serial from Duet, forward to PanelDue and buffer/read JSON status data
  while (GRBLSerial.available())
  {
    // TODO: Convert to readline and then parse GRBL status
    uint8_t c = GRBLSerial.read();
    collect(c);
  }

  // extract selected data from JSON message and forward to USB routine on other core
  // DuetStatus * duetstatus = new DuetStatus;
  // for(uint8_t i=0;i<6;i++)
  //   duetstatus->axis_userPosition[i] = (double)jsondoc["result"]["move"]["axes"][i]["userPosition"];
  //   duetstatus->spindle_speed = (uint16_t)jsondoc["result"]["spindles"][0]["current"];
  // rp2040.fifo.push_nb((uint32_t)duetstatus);
}

void show_state(const char* state) {
  Serial.printf("State reported as %s\n.", state);
}

void show_dro(const pos_t* axes, const pos_t* wcos, bool isMpos, bool* limits, size_t n_axis) {
  Serial.printf("DRO:X%f,Y%f,Z%f\n",axes[0],axes[1],axes[2]);
}

