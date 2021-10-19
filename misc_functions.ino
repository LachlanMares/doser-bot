unsigned int loadUnsignedInt(int addr)
{ 
  // Function for loading a long from EEPROM
  unsigned int uint_value = 0;
  uint_value = ((unsigned int)EEPROM.read(addr) << 8);
  uint_value +=  (unsigned int)EEPROM.read(addr+1); 
  return(uint_value);
}

long loadLong(int addr)
{ 
  // Function for loading a long from EEPROM
  long long_value = 0;
  long_value =  ((long)EEPROM.read(addr) << 24); 
  long_value += ((long)EEPROM.read(addr+1) << 16); 
  long_value += ((long)EEPROM.read(addr+2) << 8);
  long_value +=  (long)EEPROM.read(addr+3); 
  return(long_value);
}

unsigned long loadUnsignedLong(int addr)
{ 
  // Function for loading a long from EEPROM
  unsigned long long_value = 0;
  long_value =  ((unsigned long)EEPROM.read(addr) << 24); 
  long_value += ((unsigned long)EEPROM.read(addr+1) << 16); 
  long_value += ((unsigned long)EEPROM.read(addr+2) << 8);
  long_value +=  (unsigned long)EEPROM.read(addr+3); 
  return(long_value);
}

void storeLong(long long_value, int addr)
{                                          
  // Function for storing a long in EEPROM                                        
  EEPROM.write(addr, long_value >> 24); 
  EEPROM.write(addr+1, long_value >> 16); 
  EEPROM.write(addr+2, long_value >> 8);
  EEPROM.write(addr+3, long_value);  
}

void storeUnsignedLong(unsigned long long_value, int addr)
{                                          
  // Function for storing a long in EEPROM                                        
  EEPROM.write(addr, long_value >> 24); 
  EEPROM.write(addr+1, long_value >> 16); 
  EEPROM.write(addr+2, long_value >> 8);
  EEPROM.write(addr+3, long_value);  
}

unsigned long unsignedLongFromBytes(unsigned char* byte_ptr)
{                                          
  unsigned long long_value = 0;                                        
  long_value =  ((unsigned long)*byte_ptr++ << 24); 
  long_value += ((unsigned long)*byte_ptr++ << 16); 
  long_value += ((unsigned long)*byte_ptr++ << 8);
  long_value +=  (unsigned long)*byte_ptr;  
  return long_value;
}

unsigned long bytesFromUnsignedLong(unsigned long long_value, unsigned char* byte_ptr)
{                                          
  *byte_ptr++ = long_value >> 24;                                      
  *byte_ptr++ = long_value >> 16;   
  *byte_ptr++ = long_value >> 8;   
  *byte_ptr = long_value;  
}
