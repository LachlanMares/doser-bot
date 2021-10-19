// Message options
#define WHO_AM_I 0



void initialiseSerial() 
{
  last_status_update = micros(); 
  serialport.setInitial(1, BAUD_RATE, SERIAL_TIMEOUT_MS);  
}

boolean processSerial(int _bytes_read) 
{

}

void statusUpdate()
{
  
  unsigned char status_buffer[50];
   
  status_buffer[0] = m0.Status();
  bytesFromUnsignedLong(m0.variables.pulses_remaining, &status_buffer[1]);

  status_buffer[5] = m1.Status();
  bytesFromUnsignedLong(m1.variables.pulses_remaining, &status_buffer[6]);

  status_buffer[10] = m2.Status();
  bytesFromUnsignedLong(m2.variables.pulses_remaining, &status_buffer[11]);

  status_buffer[15] = m3.Status();
  bytesFromUnsignedLong(m3.variables.pulses_remaining, &status_buffer[16]);

  status_buffer[20] = m4.Status();
  bytesFromUnsignedLong(m4.variables.pulses_remaining, &status_buffer[21]);

  status_buffer[25] = m5.Status();
  bytesFromUnsignedLong(m5.variables.pulses_remaining, &status_buffer[26]);

  status_buffer[30] = m6.Status();
  bytesFromUnsignedLong(m6.variables.pulses_remaining, &status_buffer[31]);

  status_buffer[35] = m7.Status();
  bytesFromUnsignedLong(m7.variables.pulses_remaining, &status_buffer[36]);

  status_buffer[40] = m8.Status();
  bytesFromUnsignedLong(m8.variables.pulses_remaining, &status_buffer[41]);

  status_buffer[45] = m9.Status();
  bytesFromUnsignedLong(m9.variables.pulses_remaining, &status_buffer[46]);

  serialport.sendMessage(&status_buffer[0], 50);
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
