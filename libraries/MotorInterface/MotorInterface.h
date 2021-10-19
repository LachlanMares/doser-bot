#ifndef MotorInterface_h
#define MotorInterface_h

#include "Arduino.h"
#include <Wire.h>

// Bits 0 = Direction (out), 1 = Fault (in), 2,3 & 4 = M0, M1 & M2 (out), 5 = Enable (out), 6 = Reset (out), 7 = Sleep (out)
#define DIRECTION_BIT 0
#define FAULT_BIT 1
#define M0_BIT 2
#define M1_BIT 3
#define M2_BIT 4
#define ENABLE_BIT 5
#define RESET_BIT 6
#define SLEEP_BIT 7

#define IO_DIRECTION 0xFD
#define IO_STATE 0x02

#define CW  0
#define CCW 1

#define DEFAULT_PULSE_PERIOD 250
#define DEFAULT_PULSE_ON_PERIOD 100
#define DEFAULT_PULSE_INTERVAL 1000
#define MINIMUM_PULSE_INTERVAL 100
#define MAXIMUM_PULSE_INTERVAL 1000000

//Address of MCP23017 IO Expander, 8 addresses available
#define MCP23017_ADDR_0	     0x20
#define MCP23017_ADDR_1	     0x21
#define MCP23017_ADDR_2	     0x22
#define MCP23017_ADDR_3	     0x23
#define MCP23017_ADDR_4	     0x24
#define MCP23017_ADDR_5	     0x25
#define MCP23017_ADDR_6	     0x26
#define MCP23017_ADDR_7	     0x27

#define MCP23017_IODIRA      0x00
#define MCP23017_GPIOA       0x12
#define MCP23017_GPPUA       0x0C

#define MCP23017_IODIRB      0x01
#define MCP23017_GPIOB       0x13
#define MCP23017_GPPUB       0x0D


struct motor_struct {

  boolean direction;
  boolean running;
  boolean fault;

  uint8_t microstep;

  unsigned long pulse_interval;
  unsigned long pulses_remaining;
  unsigned long pulse_on_period;
};

class MotorInterface
{
public:
        MotorInterface();
        void Initialise(int, int, uint8_t);
        void Enable();
        void Disable();
        void Reset();
        boolean FaultStatus();
        void StartJob();
        void PauseJob();
        void ResumeJob();
        void StopJob();
        void Update(unsigned long);
        uint8_t Status();
        motor_struct variables;

private:
        void Wake();
        void Sleep();
        void Direction(uint8_t);
        void SetMicroStep(uint8_t);
        uint8_t DecodeMicroStep(uint8_t);
        void ReadIORegister();
        void WriteIORegister(uint8_t);
        void WriteDirectionRegister(uint8_t);
        void WritePullUpRegister(uint8_t);

        boolean _output_state, _enabled, _sleep;
        uint8_t _current_io, _direction_register, _io_register, _pull_up_register, _step_divisor;
        int _i2c_addr, _step_pin;
        unsigned long _pulse_on_micros, _pulse_off_micros, _last_micros, _last_fault_check_micros, _fault_check_interval;
};

#endif
