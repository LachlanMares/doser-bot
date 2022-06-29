
void initialiseSerial() 
{
  last_status_update = micros(); 
  serialport.setInitial(1, BAUD_RATE, SERIAL_TIMEOUT_MS);  
}

boolean processSerial(int _bytes_read) 
{
  if(serial_buffer[0] == WHO_AM_I)
  {
    if(_bytes_read == 1)
    {
      unsigned char who_buffer[10] = {WHO_AM_I_MESSAGE_ID, 0x44, 0x6F, 0x73, 0x65, 0x72, 0x2d, 0x42, 0x6F, 0x74};
      serialport.sendMessage(&who_buffer[0], 10);
    }
  } 
  
  else if(serial_buffer[0] == MOTOR_0)
  {  
    unsigned char m0_buffer[5] = {RESPONSE_MESSAGE_ID, serial_buffer[0], serial_buffer[1], 0x00, ACK};
    
    if (serial_buffer[1] == SEND_JOB || serial_buffer[1] == SEND_JOB_ALL_VARIABLES)
    {
      if(m0.status_variables.running)
      {
        m0_buffer[3] = MOTOR_BUSY_RESPONSE;
        m0_buffer[4] = NAK;
        
      } else if (_bytes_read == 8 || _bytes_read == 16)
        {
          m0.command_variables.direction = serial_buffer[2];
          m0.command_variables.microstep = serial_buffer[3];
          m0.command_variables.pulses = unsignedLongFromBytes(&serial_buffer[4]);
          
          if(serial_buffer[1] == SEND_JOB_ALL_VARIABLES)
          {
             m0.command_variables.pulse_interval = unsignedLongFromBytes(&serial_buffer[8]);
             m0.command_variables.pulse_on_period = unsignedLongFromBytes(&serial_buffer[12]);
          } else
            {
              m0.command_variables.pulse_interval = 0;
              m0.command_variables.pulse_on_period = 0;
            }
            
          m0.StartJob();
           
        } else
          {
            m0_buffer[3] = BAD_JOB_COMMAND;
            m0_buffer[4] = NAK;
          }
    } else if (serial_buffer[1] == PAUSE_JOB)
      {
        m0.PauseJob();
      } else if (serial_buffer[1] == RESUME_JOB)
        {
          m0.ResumeJob();
        } else if (serial_buffer[1] == CANCEL_JOB)
         {
           m0.CancelJob();
         } else if (serial_buffer[1] == ENABLE_MOTOR)
           {
             m0.Enable();
           } else if (serial_buffer[1] == DISABLE_MOTOR)
             {
               m0.Disable();
             } else if (serial_buffer[1] == SLEEP_MOTOR)
               {
                 m0.Sleep();
               } else if (serial_buffer[1] == WAKE_MOTOR)
                 {
                   m0.Wake();
                 } else if (serial_buffer[1] == RESET_MOTOR)
                   {
                     m0.Reset();
                   } else
                     {
                       m0_buffer[3] = UNKNOWN_MOTOR_COMMAND;
                       m0_buffer[4] = NAK;
                     }

    serialport.sendMessage(&m0_buffer[0], 5);
  }
}

void statusUpdate()
{
  unsigned char status_buffer[STATUS_MESSAGE_LENGTH];
  (sequence_counter == 255) ? sequence_counter = 0 : sequence_counter++;
  
  status_buffer[0] = STATUS_MESSAGE_ID;
  status_buffer[1] = sequence_counter;
   
  status_buffer[2] = m0.Status();
  bytesFromUnsignedLong(m0.status_variables.pulses_remaining, &status_buffer[3]);

  status_buffer[7] = m1.Status();
  bytesFromUnsignedLong(m1.status_variables.pulses_remaining, &status_buffer[8]);

  status_buffer[12] = m2.Status();
  bytesFromUnsignedLong(m2.status_variables.pulses_remaining, &status_buffer[13]);

  status_buffer[17] = m3.Status();
  bytesFromUnsignedLong(m3.status_variables.pulses_remaining, &status_buffer[18]);

  status_buffer[22] = m4.Status();
  bytesFromUnsignedLong(m4.status_variables.pulses_remaining, &status_buffer[23]);

  status_buffer[27] = m5.Status();
  bytesFromUnsignedLong(m5.status_variables.pulses_remaining, &status_buffer[28]);

  status_buffer[32] = m6.Status();
  bytesFromUnsignedLong(m6.status_variables.pulses_remaining, &status_buffer[33]);

  status_buffer[37] = m7.Status();
  bytesFromUnsignedLong(m7.status_variables.pulses_remaining, &status_buffer[38]);

  status_buffer[42] = m8.Status();
  bytesFromUnsignedLong(m8.status_variables.pulses_remaining, &status_buffer[43]);

  status_buffer[47] = m9.Status();
  bytesFromUnsignedLong(m9.status_variables.pulses_remaining, &status_buffer[48]);

  serialport.sendMessage(&status_buffer[0], STATUS_MESSAGE_LENGTH);
}


void updateSerial()
{
  bytes_read = serialport.update(&serial_buffer[0]);

  if(bytes_read > 0)
  {
    valid_message = processSerial(bytes_read);
  }

  if(micros() - last_status_update > STATUS_UPDATE_INTERVAL)  
  {
    statusUpdate();
    last_status_update = micros();
  }
  
}
