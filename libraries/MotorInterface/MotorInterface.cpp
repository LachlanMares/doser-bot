#include "MotorInterface.h"

MotorInterface::MotorInterface()
{

}

void MotorInterface::Initialise(int i2c_address, int pin, uint8_t bank)
{
    _i2c_addr = i2c_address;
    _step_pin = pin;

    if(bank == 'A')
    {
        _direction_register = MCP23017_IODIRA;
        _io_register = MCP23017_GPIOA;
        _pull_up_register = MCP23017_GPPUA;

    } else if(bank == 'B')
        {
            _direction_register = MCP23017_IODIRB;
            _io_register = MCP23017_GPIOB;
            _pull_up_register = MCP23017_GPPUB;
        }

    _output_state = false;
    _enabled = false;
    _sleep = false;
    _fault_check_interval = 1000000;
    _last_fault_check_micros = micros();

    variables.direction = false;
    variables.running = false;
    variables.fault = false;
    variables.microstep = 1;
    variables.pulses_remaining = 0;
    variables.pulse_interval = DEFAULT_PULSE_INTERVAL;
    variables.pulse_on_period = DEFAULT_PULSE_ON_PERIOD;

    SetMicroStep(variables.microstep);

    pinMode(_step_pin, OUTPUT);
    digitalWrite(_step_pin, LOW);

    WriteDirectionRegister(IO_DIRECTION);
    WriteIORegister(IO_STATE);
}

void MotorInterface::Enable()
{
    _enabled = true;
    ReadIORegister();

    if(!bitRead(_current_io, ENABLE_BIT))
    {
        bitSet(_current_io, ENABLE_BIT);
        WriteIORegister(_current_io);
    }
}

void MotorInterface::Disable()
{
    _enabled = false;
    ReadIORegister();

    if(bitRead(_current_io, ENABLE_BIT))
    {
        bitClear(_current_io, ENABLE_BIT);
        WriteIORegister(_current_io);
    }
}

void MotorInterface::Wake()
{
    _sleep = false;
    ReadIORegister();

    if(bitRead(_current_io, SLEEP_BIT))
    {
        bitClear(_current_io, SLEEP_BIT);
        WriteIORegister(_current_io);
    }
}

void MotorInterface::Sleep()
{
    _sleep = true;
    ReadIORegister();

    if(!bitRead(_current_io, SLEEP_BIT))
    {
        bitSet(_current_io, SLEEP_BIT);
        WriteIORegister(_current_io);
    }
}

void MotorInterface::Reset()
{
    ReadIORegister();
    bitClear(_current_io, RESET_BIT);
    WriteIORegister(_current_io);
    delay(1);
    bitSet(_current_io, RESET_BIT);
    WriteIORegister(_current_io);
}

boolean MotorInterface::FaultStatus()
{
    ReadIORegister();
    return bitRead(_current_io, FAULT_BIT);
}

void MotorInterface::Direction(uint8_t dir)
{
    ReadIORegister();

    if(dir == 0 || dir == CW)
    {
        variables.direction = false;
        bitClear(_current_io, DIRECTION_BIT);
        WriteIORegister(_current_io);

    } else if(dir == 1 || dir == CCW)
        {
            variables.direction = true;
            bitSet(_current_io, DIRECTION_BIT);
            WriteIORegister(_current_io);
        }
}

void MotorInterface::SetMicroStep(uint8_t step_divisor)
{
    ReadIORegister();
    variables.microstep = step_divisor;
    _current_io = DecodeMicroStep(_current_io);
    WriteIORegister(_current_io);
}

uint8_t MotorInterface::DecodeMicroStep(uint8_t start_value)
{
    switch(variables.microstep)
    {
        case 1:
            bitClear(start_value, M0_BIT);
            bitClear(start_value, M1_BIT);
            bitClear(start_value, M2_BIT);
            break;

        case 2:
            bitSet(start_value, M0_BIT);
            bitClear(start_value, M1_BIT);
            bitClear(start_value, M2_BIT);
            break;

        case 4:
            bitClear(start_value, M0_BIT);
            bitSet(start_value, M1_BIT);
            bitClear(start_value, M2_BIT);
            break;

        case 8:
            bitSet(start_value, M0_BIT);
            bitSet(start_value, M1_BIT);
            bitClear(start_value, M2_BIT);
            break;

        case 16:
            bitClear(start_value, M0_BIT);
            bitClear(start_value, M1_BIT);
            bitSet(start_value, M2_BIT);
            break;

        case 32:
            bitSet(start_value, M0_BIT);
            bitClear(start_value, M1_BIT);
            bitSet(start_value, M2_BIT);
            break;

        default:
            bitClear(start_value, M0_BIT);
            bitClear(start_value, M1_BIT);
            bitClear(start_value, M2_BIT);
            variables.microstep = 1;
            break;
    }
    return start_value;
}

void MotorInterface::StartJob()
{
    if(FaultStatus())
    {
        variables.fault = true;
        variables.running = false;
        Sleep();
        Disable();

    } else
        {
            variables.fault = false;
            variables.running = true;
            _output_state = false;
            _pulse_on_micros = micros();
            _last_micros = _pulse_on_micros;

            Wake();
            Enable();
            Direction(variables.direction);
            SetMicroStep(variables.microstep);

            variables.pulse_interval = (variables.pulse_interval > MINIMUM_PULSE_INTERVAL && variables.pulse_interval < MAXIMUM_PULSE_INTERVAL) ? variables.pulse_interval : DEFAULT_PULSE_INTERVAL;
            variables.pulse_on_period = (variables.pulse_on_period < variables.pulse_interval) ? variables.pulse_on_period : (unsigned long)(variables.pulse_interval/2);

            digitalWrite(_step_pin, LOW);
        }
}

void MotorInterface::PauseJob()
{
    variables.running = false;
}

void MotorInterface::ResumeJob()
{
    variables.running = true;
    _pulse_on_micros = micros();
    _last_micros = _pulse_on_micros;
}

void MotorInterface::StopJob()
{
    variables.running = false;
    _output_state = false;
    variables.pulses_remaining = 0;
    digitalWrite(_step_pin, LOW);
}

void MotorInterface::Update(unsigned long micros_now)
{
    if(_enabled)
    {
        if(variables.running)
        {
            if(variables.pulses_remaining > 0)
            {
                if(!_output_state)
                {
                    if(micros_now - _last_micros >= _pulse_on_micros)
                    {
                        digitalWrite(_step_pin, HIGH);
                        _output_state = true;
                        _pulse_off_micros = micros_now + variables.pulse_on_period;
                        _pulse_on_micros = micros_now + variables.pulse_interval;
                    }

                } else
                    {
                        digitalWrite(_step_pin, HIGH);
                        _output_state = false;
                        variables.pulses_remaining--;
                    }
            } else
                {
                    variables.running = false;
                }
        }

    }

    _last_micros = micros_now;

    if(micros_now - _last_fault_check_micros >= _fault_check_interval)
    {
        _last_fault_check_micros = micros_now;

        if(FaultStatus())
        {
            variables.fault = true;
            variables.running = false;
            Disable();

        } else
            {
                variables.fault = false;
            }
    }
}

uint8_t MotorInterface::Status()
{
    uint8_t status_byte = DecodeMicroStep(0); // Bit 2, 3 & 4
    bitWrite(status_byte, DIRECTION_BIT, variables.direction); // Bit 0
    bitWrite(status_byte, FAULT_BIT, variables.fault); // Bit 1
    bitWrite(status_byte, ENABLE_BIT, _enabled); // Bit 5
    bitWrite(status_byte, 6, variables.running); // Bit 6
    bitWrite(status_byte, SLEEP_BIT, _sleep); // Bit 7
    return status_byte;
}

void MotorInterface::ReadIORegister()
{
  Wire.beginTransmission(_i2c_addr);
  Wire.write(_io_register);
  Wire.endTransmission();
  Wire.requestFrom(_i2c_addr, 1);
  _current_io = Wire.read();
}

void MotorInterface::WriteIORegister(uint8_t val)
{
  Wire.beginTransmission(_i2c_addr);
  Wire.write(_io_register);
  Wire.write(val);
  Wire.endTransmission();
}

void MotorInterface::WriteDirectionRegister(uint8_t val)
{
  Wire.beginTransmission(_i2c_addr);
  Wire.write(_direction_register);
  Wire.write(val);
  Wire.endTransmission();
}

void MotorInterface::WritePullUpRegister(uint8_t val)
{
  Wire.beginTransmission(_i2c_addr);
  Wire.write(_direction_register);
  Wire.write(_pull_up_register);
  Wire.endTransmission();
}
