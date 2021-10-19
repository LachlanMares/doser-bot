
#include <Wire.h>
#include <EEPROM.h>
#include "MotorInterface.h"
#include "AtSerial.h"

#define BAUD_RATE 250000
#define SERIAL_BUFFER_LENGTH 128
#define SERIAL_TIMEOUT_MS 100
#define STATUS_UPDATE_INTERVAL 1000000

#define STATUS_MESSAGE_ID 0xFF
#define RESPONSE_MESSAGE_ID 0xFE
#define WHO_AM_I_MESSAGE_ID 0xFD

#define m0_step_pin 4
#define m1_step_pin 5
#define m2_step_pin 6
#define m3_step_pin 7
#define m4_step_pin 8
#define m5_step_pin 9
#define m6_step_pin 10
#define m7_step_pin 11
#define m8_step_pin 12
#define m9_step_pin 13

MotorInterface m0, m1, m2, m3, m4, m5, m6, m7, m8, m9;
AtSerial serialport;

boolean valid_message = false;
unsigned char serial_buffer[SERIAL_BUFFER_LENGTH];
int bytes_read;
unsigned long last_status_update, current_micros;

void setup() 
{
  initialiseSerial();
  m0.Initialise(MCP23017_ADDR_0, m0_step_pin, 'A');
  m1.Initialise(MCP23017_ADDR_1, m1_step_pin, 'A');
  m2.Initialise(MCP23017_ADDR_2, m2_step_pin, 'A');
  m3.Initialise(MCP23017_ADDR_3, m3_step_pin, 'A');
  m4.Initialise(MCP23017_ADDR_4, m4_step_pin, 'A');
  m5.Initialise(MCP23017_ADDR_4, m5_step_pin, 'B');
  m6.Initialise(MCP23017_ADDR_3, m6_step_pin, 'B');
  m7.Initialise(MCP23017_ADDR_2, m7_step_pin, 'B');
  m8.Initialise(MCP23017_ADDR_1, m8_step_pin, 'B');
  m9.Initialise(MCP23017_ADDR_0, m9_step_pin, 'B');

}

void loop() 
{
  current_micros = micros();
  m0.Update(current_micros);
  m1.Update(current_micros);
  m2.Update(current_micros);
  m3.Update(current_micros);
  m4.Update(current_micros);
  m5.Update(current_micros);
  m6.Update(current_micros);
  m7.Update(current_micros);
  m8.Update(current_micros);
  m9.Update(current_micros);
  updateSerial();
}
