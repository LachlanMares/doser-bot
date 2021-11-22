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

    _fault_check_interval = 1000000;
    _last_fault_check_micros = micros();


    status_variables.enabled = false;
    status_variables.output_state = false;

    status_variables.direction = false;
    status_variables.running = false;
    status_variables.fault = false;
    status_variables.microstep = 1;
    status_variables.pulses_remaining = 0;

    command_variables.direction = false;
    command_variables.microstep = 1;
    command_variables.pulses = 0;
    command_variables.pulse_interval = DEFAULT_PULSE_INTERVAL;
    command_variables.pulse_on_period = DEFAULT_PULSE_ON_PERIOD;

    SetMicroStep(status_variables.microstep);

    pinMode(_step_pin, OUTPUT);
    digitalWrite(_step_pin, LOW);

    WriteDirectionRegister(IO_DIRECTION);
    WriteIORegister(IO_STATE);
}

void MotorInterface::Enable()
{
    status_variables.enabled = true;

    ReadIORegister();

    if(!bitRead(_current_io, ENABLE_BIT))
    {
        bitSet(_current_io, ENABLE_BIT);
        WriteIORegister(_current_io);
    }
}

void MotorInterface::Disable()
{
    status_variables.enabled = false;

    ReadIORegister();

    if(bitRead(_current_io, ENABLE_BIT))
    {
        bitClear(_current_io, ENABLE_BIT);
        WriteIORegister(_current_io);
    }
}

void MotorInterface::Wake()
{
    status_variables.sleep = false;

    ReadIORegister();

    if(bitRead(_current_io, SLEEP_BIT))
    {
        bitClear(_current_io, SLEEP_BIT);
        WriteIORegister(_current_io);
    }
}

void MotorInterface::Sleep()
{
    status_variables.sleep = true;

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
        command_variables.direction = false;
        bitClear(_current_io, DIRECTION_BIT);
        WriteIORegister(_current_io);

    } else if(dir == 1 || dir == CCW)
        {
            command_variables.direction = true;
            bitSet(_current_io, DIRECTION_BIT);
            WriteIORegister(_current_io);
        }
}

void MotorInterface::SetMicroStep(uint8_t step_divisor)
{
    command_variables.microstep = step_divisor;
}

uint8_t MotorInterface::DecodeMicroStep(uint8_t start_value)
{
    switch(status_variables.microstep)
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
            status_variables.microstep = 1;
            break;
    }
    return start_value;
}

void MotorInterface::StartJob()
{
    ReadIORegister();

    if(bitRead(_current_io, FAULT_BIT))
    {
        status_variables.fault = true;
        status_variables.running = false;

        Sleep();
        Disable();

    } else
        {
            status_variables.fault = false;
            status_variables.running = true;
            status_variables.output_state = false;

            _pulse_on_micros = micros();
            _last_micros = _pulse_on_micros;

            bitClear(_current_io, SLEEP_BIT);
            bitSet(_current_io, ENABLE_BIT);
            bitWrite(_current_io, DIRECTION_BIT, status_variables.direction);

            _current_io = DecodeMicroStep(_current_io);

            command_variables.pulse_interval = (command_variables.pulse_interval > MINIMUM_PULSE_INTERVAL && command_variables.pulse_interval < MAXIMUM_PULSE_INTERVAL) ? command_variables.pulse_interval : DEFAULT_PULSE_INTERVAL;
            command_variables.pulse_on_period = (command_variables.pulse_on_period < command_variables.pulse_interval) ? command_variables.pulse_on_period : (unsigned long)(command_variables.pulse_interval/2);

            digitalWrite(_step_pin, LOW);
        }
}

void MotorInterface::PauseJob()
{
    status_variables.running = false;
}

void MotorInterface::ResumeJob()
{
    status_variables.running = true;
    _pulse_on_micros = micros();
    _last_micros = _pulse_on_micros;
}

void MotorInterface::StopJob()
{
    status_variables.running = false;
    status_variables.output_state = false;
    status_variables.pulses_remaining = 0;
    digitalWrite(_step_pin, LOW);
}

void MotorInterface::Update(unsigned long micros_now)
{
    if(status_variables.enabled)
    {
        if(status_variables.running)
        {
            if(status_variables.pulses_remaining > 0)
            {
                if(!status_variables.output_state)
                {
                    if(micros_now - _last_micros >= _pulse_on_micros)
                    {
                        digitalWrite(_step_pin, HIGH);
                        status_variables.output_state = true;
                        _pulse_off_micros = micros_now + command_variables.pulse_on_period;
                        _pulse_on_micros = micros_now + command_variables.pulse_interval;
                    }

                } else
                    {
                        digitalWrite(_step_pin, HIGH);
                        status_variables.output_state = false;
                        status_variables.pulses_remaining--;
                    }
            } else
                {
                    status_variables.running = false;
                }
        }

    }

    _last_micros = micros_now;

    if(micros_now - _last_fault_check_micros >= _fault_check_interval)
    {
        _last_fault_check_micros = micros_now;

        if(FaultStatus())
        {
            status_variables.fault = true;
            status_variables.running = false;
            Disable();

        } else
            {
                status_variables.fault = false;
            }
    }
}

uint8_t MotorInterface::Status()
{
    uint8_t status_byte = DecodeMicroStep(0); // Bit 2, 3 & 4
    bitWrite(status_byte, DIRECTION_BIT, status_variables.direction); // Bit 0
    bitWrite(status_byte, FAULT_BIT, status_variables.fault); // Bit 1
    bitWrite(status_byte, ENABLE_BIT, status_variables.enabled); // Bit 5
    bitWrite(status_byte, RUNNING_BIT, status_variables.running); // Bit 6
    bitWrite(status_byte, SLEEP_BIT, status_variables.sleep); // Bit 7
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

