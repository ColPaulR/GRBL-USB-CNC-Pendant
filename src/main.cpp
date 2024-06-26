#include <Arduino.h>

#include "GrblParserC.h"
#include "USBHIDPendant.h"
#include "SerialDebug.h"

#define GRBLSerial Serial1 // UART0
#define GRBLSerialTXPin 12
#define GRBLSerialRXPin 13

// #define GRBLSerialBaud 57600
#define GRBLSerialBaud 115200

// Autoreporting in not working reliably. Using slow update interval plus autoreporting to enforce response
// Autoreporting seems to work once polling response is processed. 
// Use Polling at defined non-zero interval
#define GRBL_QUERY_INTERVAL 5000
// Expect autoreporting
//       uart_channel1:
//         uart_num: 1
//         report_interval_ms: 125
//         all_messages: false
// Does not work with all_messages:true (maybe overflow buffer?)
// #define GRBL_QUERY_INTERVAL 0

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
  // Wait until GRBLSerial is available
  delay(100);
  // Force status update for initial status
  GRBLSerial.write('?');
  GRBLSerial.println("$G");
}

void loop()
{
  // check for G-Code command from USB routine on other core
  while (rp2040.fifo.available())
  {
    String *cmd;

    cmd = (String *)rp2040.fifo.pop();

#if (GRBL_COMMAND_ECHO)
    // Echo to debug port
    Serial.println(*cmd);
#endif

    // send to GRBL Host
    GRBLSerial.print(*cmd);

    // Cleanup memory
    delete cmd;
  }

  // read serial from GRBL
  while (GRBLSerial.available())
  {
    uint8_t c = GRBLSerial.read();

#if (GRBL_STATUS_ECHO) // Echo
    Serial.write(c);
#endif
    collect(c);
  }

  // Currently using reporting interval so no status request is required
  // Uncomment below if periodic requests are needed

#if (GRBL_QUERY_INTERVAL)
  // Send periodic requests
  static unsigned long last_mount_check = millis();
  unsigned long now = millis();
  if ((now - last_mount_check) > GRBL_QUERY_INTERVAL)
  {
    last_mount_check = millis();
    // Query status
    GRBLSerial.write('?');
    GRBLSerial.println("$G");
  }
#endif
}
