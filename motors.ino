
void initialiseMotors()
{
  m0.Initialise(MCP23017_ADDR_0, m0_step_pin, 'A');
  /*
  m1.Initialise(MCP23017_ADDR_1, m1_step_pin, 'A');
  m2.Initialise(MCP23017_ADDR_2, m2_step_pin, 'A');
  m3.Initialise(MCP23017_ADDR_3, m3_step_pin, 'A');
  m4.Initialise(MCP23017_ADDR_4, m4_step_pin, 'A');
  m5.Initialise(MCP23017_ADDR_4, m5_step_pin, 'B');
  m6.Initialise(MCP23017_ADDR_3, m6_step_pin, 'B');
  m7.Initialise(MCP23017_ADDR_2, m7_step_pin, 'B');
  m8.Initialise(MCP23017_ADDR_1, m8_step_pin, 'B');
  m9.Initialise(MCP23017_ADDR_0, m9_step_pin, 'B');
  */
}

void updateMotors()
{
  long current_micros = micros();
  m0.Update(current_micros);
  /*
  m1.Update(current_micros);
  m2.Update(current_micros);
  m3.Update(current_micros);
  m4.Update(current_micros);
  m5.Update(current_micros);
  m6.Update(current_micros);
  m7.Update(current_micros);
  m8.Update(current_micros);
  m9.Update(current_micros);
  */
}
