/** 
 * @file
 * @author Jeff Lau
 * 
 * Convenience functions for reading/writing data from/to EEPROM.
 */

#include "eeprom.h"
#include <xc.h>

/**
 * Total size of the EEPROM write buffer in bytes.
 * 
 * NOTE: Each buffer write consumes an extra `EEPROM_WRITE_BUFFER_HEADER_SIZE` 
 * bytes of this buffer for address and size information.
 */
#define EEPROM_WRITE_BUFFER_SIZE (512)

/**
 * Current state of buffered EEPROM writing.
 */
static struct {
  /**
   * The buffer of EEPROM writes.
   * 
   * Each buffered write is stored in this buffer as follows:
   * - `writeAddress`: 2-byte target address.
   * - `writeSize`: 1-byte length of data to be written.
   * - `data`: The data to be written (number of bytes == `writeSize`).
   */
  uint8_t buffer[EEPROM_WRITE_BUFFER_SIZE];
  /**
   * The buffer index of the next byte to be WRITTEN TO the buffer.
   */
  uint16_t head;
  /**
   * The buffer index of the next byte to be READ FROM the buffer.
   */
  uint16_t tail;
  /**
   * Number of unused bytes remaining in the buffer.
   */
  uint16_t remaining;
  /**
   * The EEPROM address where the next byte from the buffer will be written.
   */
  uint16_t writeAddress;
  /**
   * The number of bytes remaining in the current "run" of bytes in the buffer
   * to be written to EEPROM.
   */
  uint8_t writeSize;
} writeBufferState;

/**
 * The size (in bytes) of the header for a single buffered write.
 */
#define EEPROM_WRITE_BUFFER_HEADER_SIZE (sizeof(writeBufferState.writeAddress) + sizeof(writeBufferState.writeSize))

/**
 * Read the next byte out of the EEPROM write buffer.
 * 
 * WARNING: Assumes that there is a byte to be read from the buffer. There is no 
 *          buffer underrun protection here.
 * 
 * @return The value of the next byte in the EEPROM write buffer.
 */
static uint8_t readByteFromWriteBuffer(void) {
  ++writeBufferState.remaining;
  
  uint8_t result = writeBufferState.buffer[writeBufferState.tail++];

  if (writeBufferState.tail == EEPROM_WRITE_BUFFER_SIZE) {
    writeBufferState.tail = 0;
  }
  
  return result;
}

/**
 * Writes a byte to the EEPROM write buffer.
 * 
 * WARNING: Assumes that there is room to write a byte to the buffer. There is no 
 *          buffer overrun protection here.
 * 
 * @param value - The byte value to be written to the buffer.
 */
static void writeByteToWriteBuffer(uint8_t value) {
  --writeBufferState.remaining;
  
  writeBufferState.buffer[writeBufferState.head++] = value;

  if (writeBufferState.head == EEPROM_WRITE_BUFFER_SIZE) {
    writeBufferState.head = 0;
  }
}

/**
 * Write a single byte to EERPOM, but do not wait for the write to complete 
 * before returning.
 * 
 * To avoid unnecessary writes, this function first reads the value at the 
 * specified address and aborts the write if the current value already matches 
 * the value to be written.
 * 
 * WARNING: Assumes that there is no EEPROM write currently in progress.
 * 
 * @param address - The EEPROM address tow write.
 * @param value - The value to write to EEPROM.
 * @return true if the write was started; false if the desired value already 
 *         exists in EEPROM.
 */
static bool writeByteWithoutWaitingForCompletion(uint16_t address, uint8_t value) {
  // Set NVMADR with the target address (0x380000 - 0x3803FF)
  NVMADRU = 0x38;
  NVMADRH = (uint8_t) ((address & 0xFF00) >> 8);
  NVMADRL = (uint8_t) (address & 0x00FF);

  // Set the NVMCMD control bits for DFM Byte Read operation
  NVMCON1bits.NVMCMD = 0b000;
  NVMCON0bits.GO = 1;

  // Do nothing if the desired value already exists at this address
  if (NVMDATL == value) {
    return false;
  }  
  
  // Set the NVMCMD control bits for DFM Byte Write operation
  NVMCON1bits.NVMCMD = 0b011;

  // Load NVMDATL with desired byte
  NVMDATL = value;

  // Disable all interrupts
  uint8_t GIEBitValue = INTCON0bits.GIE;
  INTCON0bits.GIE = 0;

  // Perform the unlock sequence
  NVMLOCK = 0x55;
  NVMLOCK = 0xAA;
  
  // Start DFM write
  NVMCON0bits.GO = 1;

  // Restore all interrupts
  INTCON0bits.GIE = GIEBitValue;
  
  return true;
}

void EEPROM_Initialize(void) {
  writeBufferState.head = 0;
  writeBufferState.tail = 0;
  writeBufferState.remaining = EEPROM_WRITE_BUFFER_SIZE;
  writeBufferState.writeAddress = 0;
  writeBufferState.writeSize = 0;
}

void EEPROM_Task(void) {
  if (NVMCON0bits.GO) {
    // There's already an EEPROM write in progress, so return 
    // and allow other tasks to run while it completes.
    return;
  }
  
  if (writeBufferState.writeSize) {
    // There's more bytes to be written to EEPROM.
    // Start writing the next byte and return so that other tasks
    // can run while the byte is being written.
    writeByteWithoutWaitingForCompletion(
        writeBufferState.writeAddress++,
        readByteFromWriteBuffer()
        );
    
    --writeBufferState.writeSize;
    return;
  }
  
  // At this point, there's no pending EEPROM write, and we haven't started 
  // writing another bye, so set the control bits for reading to help prevent 
  // accidental writes until we start the next write.
  NVMCON1bits.NVMCMD = 0b000;
  
  if (writeBufferState.remaining != EEPROM_WRITE_BUFFER_SIZE) {
    // There's another run of bytes in the buffer to be written to EEPROM.
    // Read the address and size "headers" out of the buffer.
    // The next execution of this task will start writing to EEPROM.
    writeBufferState.writeAddress = readByteFromWriteBuffer();
    writeBufferState.writeAddress <<= 8;
    writeBufferState.writeAddress |= readByteFromWriteBuffer();
    
    writeBufferState.writeSize = readByteFromWriteBuffer();
  }
}

uint8_t EEPROM_ReadByte(uint16_t address) {
  // Wait for any previous pending EEPROM write to complete
  while (NVMCON0bits.GO);
  
  // Set NVMADR with the target address (0x380000 - 0x3803FF)
  NVMADRU = 0x38;
  NVMADRH = (uint8_t) ((address & 0xFF00) >> 8);
  NVMADRL = (uint8_t) (address & 0x00FF);

  // Set the NVMCMD control bits for DFM Byte Read operation
  NVMCON1bits.NVMCMD = 0b000;
  NVMCON0bits.GO = 1;

  return NVMDATL;
}

void* EEPROM_ReadBytes(uint16_t address, void* dest, uint16_t size) {
  // Wait for any previous pending EEPROM write to complete
  while (NVMCON0bits.GO);
  
  // Set NVMADR with the target word address (0x380000 - 0x3803FF)
  NVMADRU = 0x38;
  NVMADRH = (uint8_t) ((address & 0xFF00) >> 8);
  NVMADRL = (uint8_t) (address & 0x00FF);

  // Set the NVMCMD control bits for DFM Byte Read operation, post increment
  NVMCON1bits.NVMCMD = 0b001;

  /**
   * Pointer to where the next byte should be stored after reading out of EEPROM.
   */
  uint8_t* byteDest = dest;
  
  // Read all bytes out of EEPROM into the destination
  while (size--) {
    NVMCON0bits.GO = 1;
    *byteDest++ = NVMDATL;
  } 
  
  return dest;
}

void EEPROM_WriteByte(uint16_t address, uint8_t value) {
  // Wait for any previous pending EEPROM write to complete
  while (NVMCON0bits.GO);
  
  if (writeByteWithoutWaitingForCompletion(address, value)) {
    // wait for the operation to complete
    while (NVMCON0bits.GO);

    //Set the NVMCMD control bits for Word Read operation to avoid accidental writes
    NVMCON1bits.NVMCMD = 0b000;
  }
}

void EEPROM_WriteBytes(uint16_t address, void const* data, uint16_t size) {
  // Wait for any previous pending EEPROM write to complete
  while (NVMCON0bits.GO);
  
  // Set NVMADR with the target address (0x380000 - 0x3803FF)
  NVMADRU = 0x38;
  NVMADRH = (uint8_t) ((address & 0xFF00) >> 8);
  NVMADRL = (uint8_t) (address & 0x00FF);

  /**
   * Pointer to the next byte of data to be written to EEPROM
   */
  uint8_t const* byteData = data;
  
  while (size--) {
    uint8_t nextByte = *byteData++;
    
    // Set the NVMCMD control bits for DFM Byte Read operation
    NVMCON1bits.NVMCMD = 0b000;
    // Read the current value in EEPROM
    NVMCON0bits.GO = 1;

    // Check if the existing byte in EEPROM is different than what we want to write
    if (NVMDATL != nextByte) {
      // Set the NVMCMD control bits for DFM Byte Write operation, post increment
      NVMCON1bits.NVMCMD = 0b100;

      //Load NVMDATL with desired byte
      NVMDATL = nextByte;

      // Disable all interrupts
      uint8_t GIEBitValue = INTCON0bits.GIE;
      INTCON0bits.GIE = 0;

      // Perform the unlock sequence
      NVMLOCK = 0x55;
      NVMLOCK = 0xAA;

      // Start DFM write
      NVMCON0bits.GO = 1;

      // Restore all interrupts
      INTCON0bits.GIE = GIEBitValue;

      // wait for the operation to complete
      while (NVMCON0bits.GO);
    } else {
      // We didn't need to write the byte because the value in EEPROM was 
      // already what we wanted to write. But we do need to manually increment
      // the EEPROM address for the next byte.
      if (++NVMADRL == 0) {
        ++NVMADRH;
      }
    }
  } 

  // Set the NVMCMD control bits for Word Read operation to avoid accidental writes
  NVMCON1bits.NVMCMD = 0b000;
}

bool EEPROM_AsyncWriteByte(uint16_t address, uint8_t value) {
  // Confirm that there's enough room in the buffer for this byte and a header
  if (1 + EEPROM_WRITE_BUFFER_HEADER_SIZE > writeBufferState.remaining) {
    return false;
  }

  // Write the header to the buffer.
  writeByteToWriteBuffer((address & 0xFF00) >> 8);
  writeByteToWriteBuffer(address & 0x00FF);
  writeByteToWriteBuffer(1); // size is 1 byte
  
  // Write the byte value to the buffer.
  writeByteToWriteBuffer(value);  
  
  return true;
}

bool EEPROM_AsyncWriteBytes(uint16_t address, void const* data, uint8_t size) {
  // Confirm that there's enough room in the buffer for this data and a header
  if (size + EEPROM_WRITE_BUFFER_HEADER_SIZE > writeBufferState.remaining) {
    return false;
  }

  // Write the header to the buffer.
  writeByteToWriteBuffer((address & 0xFF00) >> 8);
  writeByteToWriteBuffer(address & 0x00FF);
  writeByteToWriteBuffer(size);
  
  /**
   * Pointer to the next byte of data to be written to the buffer.
   */
  uint8_t const* byteData = data;
  
  // Write all data bytes to the buffer.
  while (size--) {
    writeByteToWriteBuffer(*byteData++);
  }
  
  return true;
}

bool EEPROM_IsDoneWriting(void) {
  return (writeBufferState.remaining == EEPROM_WRITE_BUFFER_SIZE) && !NVMCON0bits.GO;
}