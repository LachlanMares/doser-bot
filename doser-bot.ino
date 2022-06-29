#include <Wire.h>
#include <EEPROM.h>
#include "MotorInterface.h"
#include "AtSerial.h"

#define BAUD_RATE                 250000
#define SERIAL_BUFFER_LENGTH      128
#define STATUS_MESSAGE_LENGTH     52
#define SERIAL_TIMEOUT_MS         100
#define STATUS_UPDATE_INTERVAL    1000000

#define STATUS_MESSAGE_ID         0xFF
#define RESPONSE_MESSAGE_ID       0xFE
#define WHO_AM_I_MESSAGE_ID       0xFD

#define SEND_JOB                  0xEF
#define SEND_JOB_ALL_VARIABLES    0xEE
#define PAUSE_JOB                 0xED
#define RESUME_JOB                0xEC
#define CANCEL_JOB                0xEB
#define ENABLE_MOTOR              0xEA
#define DISABLE_MOTOR             0xE9
#define SLEEP_MOTOR               0xE8
#define WAKE_MOTOR                0xE7
#define RESET_MOTOR               0xE6

#define BAD_JOB_COMMAND           0xDF
#define MOTOR_BUSY_RESPONSE       0xDE
#define UNKNOWN_MOTOR_COMMAND     0xDD

#define MOTOR_0                   0xA0
#define MOTOR_1                   0xA1
#define MOTOR_2                   0xA2
#define MOTOR_3                   0xA3
#define MOTOR_4                   0xA4
#define MOTOR_5                   0xA5
#define MOTOR_6                   0xA6
#define MOTOR_7                   0xA7
#define MOTOR_8                   0xA8
#define MOTOR_9                   0xA9
#define WHO_AM_I                  0xA8

#define m0_step_pin               4
#define m1_step_pin               5
#define m2_step_pin               6
#define m3_step_pin               7
#define m4_step_pin               8
#define m5_step_pin               9
#define m6_step_pin               10
#define m7_step_pin               11
#define m8_step_pin               12
#define m9_step_pin               13

MotorInterface m0, m1, m2, m3, m4, m5, m6, m7, m8, m9;
AtSerial serialport;

boolean toggle_bit = false;
boolean valid_message = false;
unsigned char serial_buffer[SERIAL_BUFFER_LENGTH], sequence_counter = 0;
int bytes_read;
long last_status_update;
uint8_t mcp0_a, mcp1_a, mcp2_a, mcp3_a, mcp4_a;
bool motor_direction = false;

void setup() 
{
  Wire.begin();
  Wire.setClock(400000);
  Serial.begin(115200);
  while (!Serial);
  initialiseSerial();
  initialiseMotors();
  m0.Enable();
}

void loop() 
{
  if(!m0.status_variables.running)
  {
    Serial.println("New Job");
    delay(1000);
    motor_direction = !motor_direction;
    m0.command_variables.direction = motor_direction;
    m0.command_variables.pulses = 800;
    m0.StartJob();
  }
  
  updateMotors();
  updateSerial();
}
