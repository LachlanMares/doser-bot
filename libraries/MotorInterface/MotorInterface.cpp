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
    _fault_check_interval = 1000000;
    _last_fault_check_micros = micros();

    status_variables.direction = false;
    status_variables.running = false;
    status_variables.fault = false;
    status_variables.enabled = false;
    status_variables.paused = false;
    status_variables.sleep = false;
    status_variables.microstep = 1;
    status_variables.pulses_remaining = 0;
    status_variables.running = false;

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
    Serial.println("enabled");
    status_variables.enabled = true;
    ReadIORegister();

    if(bitRead(_current_io, ENABLE_BIT))
    {
        bitClear(_current_io, ENABLE_BIT);
        WriteIORegister(_current_io);
    }
}

void MotorInterface::Disable()
{
    Serial.println("disabled");
    status_variables.enabled = false;
    ReadIORegister();

    if(!bitRead(_current_io, ENABLE_BIT))
    {
        bitSet(_current_io, ENABLE_BIT);
        WriteIORegister(_current_io);
    }
}

void MotorInterface::Wake()
{
    Serial.println("wake");
    status_variables.sleep = false;
    ReadIORegister();

    if(!bitRead(_current_io, SLEEP_BIT))
    {
        bitSet(_current_io, SLEEP_BIT);
        WriteIORegister(_current_io);
        
    }
}

void MotorInterface::Sleep()
{
    Serial.println("sleep");
    status_variables.sleep = true;
    ReadIORegister();

    if(bitRead(_current_io, SLEEP_BIT))
    {
        bitClear(_current_io, SLEEP_BIT);
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
    return !bitRead(_current_io, FAULT_BIT);
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
    Serial.println("start job");
    ReadIORegister();

    if(!bitRead(_current_io, FAULT_BIT))
    {
        Serial.println("job fault");
        status_variables.fault = true;
        status_variables.running = false;
        Sleep();
        Disable();

    } else
        {
            Serial.println("job good");
            status_variables.fault = false;
            status_variables.running = true;
            status_variables.direction = command_variables.direction;
            status_variables.microstep = command_variables.microstep;
            status_variables.paused = false;

            _output_state = false;
            _pulse_on_micros = 0;
            _last_micros = micros();

            bitSet(_current_io, SLEEP_BIT);
            bitSet(_current_io, RESET_BIT);
            bitClear(_current_io, ENABLE_BIT);
            bitWrite(_current_io, DIRECTION_BIT, status_variables.direction);

            _current_io = DecodeMicroStep(_current_io);

            WriteIORegister(_current_io);
            Serial.println(_current_io, HEX);
            command_variables.pulse_interval = (command_variables.pulse_interval > MINIMUM_PULSE_INTERVAL && command_variables.pulse_interval < MAXIMUM_PULSE_INTERVAL) ? command_variables.pulse_interval : DEFAULT_PULSE_INTERVAL;
            command_variables.pulse_on_period = (command_variables.pulse_on_period < command_variables.pulse_interval) ? command_variables.pulse_on_period : (unsigned long)(command_variables.pulse_interval/2);
            status_variables.pulses_remaining = command_variables.pulses;
            digitalWrite(_step_pin, LOW);
        }
}

void MotorInterface::PauseJob()
{
    status_variables.paused = true;
}

void MotorInterface::ResumeJob()
{
    status_variables.paused = false;
    _pulse_on_micros = micros();
    _last_micros = _pulse_on_micros;
}

void MotorInterface::CancelJob()
{
    status_variables.running = false;
    _output_state = false;
    status_variables.pulses_remaining = 0;
    digitalWrite(_step_pin, LOW);
}

void MotorInterface::Update(unsigned long micros_now)
{
    if(status_variables.enabled)
    {
        //Serial.println("en");
        if(status_variables.running && !status_variables.paused)
        {
            //Serial.println("r+!p");
            if(status_variables.pulses_remaining > 0)
            {
                Serial.println(status_variables.pulses_remaining);
                if(!_output_state)
                {
                    //Serial.println("on");
                    //Serial.println(_last_micros);
                    //Serial.println(_pulse_on_micros);
                    //Serial.println(abs(micros_now - _last_micros));
                    if(micros_now >= _pulse_on_micros)
                    {
                        digitalWrite(_step_pin, HIGH);
                        _output_state = true;
                        _pulse_off_micros = micros_now + command_variables.pulse_on_period;
                        _pulse_on_micros = micros_now + command_variables.pulse_interval;
                        //Serial.println(_pulse_on_micros);
                        //Serial.println(_pulse_off_micros);

                    }

                } else
                    {
                        //Serial.println("off");
                        if(micros_now >= _pulse_off_micros)
                        {
                            digitalWrite(_step_pin, LOW);
                            _output_state = false;
                            status_variables.pulses_remaining--;
                        }
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
            Serial.println("fault status");
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
    uint8_t status_byte = 0x00;
    bitWrite(status_byte, DIRECTION_BIT, status_variables.direction); // Bit 0
    bitWrite(status_byte, FAULT_BIT, status_variables.fault); // Bit 1
    bitWrite(status_byte, PAUSED_BIT, status_variables.paused); // Bit 2
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

