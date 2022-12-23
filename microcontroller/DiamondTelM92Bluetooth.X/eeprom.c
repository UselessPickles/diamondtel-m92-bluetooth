/** 
 * @file
 * @author Jeff Lau
 * 
 * Convenience functions for reading/writing data from/to EEPROM.
 */

#include "eeprom.h"
#include <xc.h>
#include <stdio.h>

/**
 * Total size of the EEPROM write buffer in bytes.
 * 
 * NOTE: Each buffer write consumes an extra `EEPROM_WRITE_BUFFER_HEADER_SIZE` 
 * bytes of this buffer for address and size information.
 */
#define EEPROM_WRITE_BUFFER_SIZE (512)

/**
 * Header structure for an entry in the write buffer.
 * 
 * NOTE: Bit fields are used to pack the header data into a minimal size
 *       to allow more data to be stored in the buffer.
 */
typedef struct {
  /**
   * Low byte of the target address.
   */
  uint8_t addressLow: 8;
  /**
   * Low byte of the size of data to write.
   */
  uint8_t sizeLow: 8;
  /**
   * The repeating byte value to be written.
   * 
   * Only relevant if `isRepeatingByte` is true.
   */
  uint8_t repeatingByte: 8;  
  /**
   * High byte of the target address.
   * 
   * NOTE: Max valid address is 0x03FF.
   */
  uint8_t addressHigh: 2;
  /**
   * High byte of the size of data to write.
   * 
   * NOTE: Max valid size is 0x0400.
   */
  uint8_t sizeHigh: 3;
  /**
   * True if this entry is for writing a repeating byte.
   * False if this entry is for writing subsequent bytes in the buffer.
   */
  bool isRepeatingByte: 1;
} write_buffer_header_t;

/**
 * Current state of the EEPROM async write buffer.
 */
static struct {
  /**
   * The buffer of EEPROM writes.
   * 
   * Each buffered write is stored in this buffer as follows:
   * - `header`: See the `write_buffer_header_t` type.
   * - `data`: zero or more bytes of data to be written, depending on the content of the header.
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
} writeBufferState;

/**
 * Current state of the in-process buffered EEPROM write.
 */
static struct {
  /**
   * The EEPROM address where the next byte will be written.
   */
  uint16_t writeAddress;
  /**
   * The number of bytes remaining in the current "run" of bytes to be 
   * written to EEPROM.
   */
  uint16_t writeSize;
  /**
   * True if this is a write of repeating bytes.
   * 
   * False if this is a write of bytes from the write buffer.
   */
  bool isRepeatingByte;
  /**
   * The repeating byte value to be written.
   * 
   * Only relevant if `isRepeatingByte` is true.
   */
  uint8_t repeatingByte;
} asyncWriteState;

/**
 * The size (in bytes) of the header for a single buffered write.
 */
#define EEPROM_WRITE_BUFFER_HEADER_SIZE (sizeof(write_buffer_header_t))

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
 * Reads a buffered write header structure out of the next bytes in the 
 * EEPROM write buffer.
 * 
 * WARNING: Assumes that there is a header to be read from the buffer. There is no 
 *          buffer underrun protection here.
 * 
 * @param header - Pointer to destination header that will be updated with the
 *        data from the buffer.
 */
static void readHeaderFromWriteBuffer(write_buffer_header_t* header) {
  uint8_t* headerBytes = header;
  
  for (uint8_t i = 0; i < EEPROM_WRITE_BUFFER_HEADER_SIZE; ++i) {
    *headerBytes++ = readByteFromWriteBuffer();
  }
}

/**
 * Writes a buffered write header to the EEPROM write buffer.
 * 
 * WARNING: Assumes that there is room to write a header to the buffer. There is no 
 *          buffer overrun protection here.
 * 
 * @param header - Pointer to header that will be written to the buffer.
 */
static void writeHeaderToWriteBuffer(write_buffer_header_t const* header) {
  uint8_t const* headerBytes = header;
  
  for (uint8_t i = 0; i < EEPROM_WRITE_BUFFER_HEADER_SIZE; ++i) {
    writeByteToWriteBuffer(*headerBytes++);
  }
}

/**
 * Write a single byte to EEPPOM, but do not wait for the write to complete 
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
  NVMADRH = (uint8_t) ((address & 0x0300) >> 8);
  NVMADRL = (uint8_t) (address & 0x00FF);

  // Set the NVMCMD control bits for DFM Byte Read operation
  NVMCON1bits.NVMCMD = 0b000;
  NVMCON0bits.GO = 1;
  uint8_t oldByte = NVMDATL;

  // Do nothing if the desired value already exists at this address
  if (oldByte == value) {
    return false;
  }  
  
  while(NVMCON0bits.GO);
  
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
  asyncWriteState.writeAddress = 0;
  asyncWriteState.writeSize = 0;
}

void EEPROM_Task(void) {
  if (NVMCON0bits.GO) {
    // There's already an EEPROM write in progress, so return 
    // and allow other tasks to run while it completes.
    return;
  }
  
  if (asyncWriteState.writeSize) {
    // There's more bytes to be written to EEPROM.
    // Start writing the next byte and return so that other tasks
    // can run while the byte is being written.
    uint8_t nextByte = asyncWriteState.isRepeatingByte 
        ? asyncWriteState.repeatingByte 
        : readByteFromWriteBuffer();

    writeByteWithoutWaitingForCompletion(
        asyncWriteState.writeAddress++,
        nextByte
        );
    
    --asyncWriteState.writeSize;
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
    write_buffer_header_t header;
    readHeaderFromWriteBuffer(&header);
    
    asyncWriteState.writeAddress = header.addressHigh;
    asyncWriteState.writeAddress <<= 8;
    asyncWriteState.writeAddress += header.addressLow;

    asyncWriteState.writeSize = header.sizeHigh;
    asyncWriteState.writeSize <<= 8;
    asyncWriteState.writeSize += header.sizeLow;

    asyncWriteState.isRepeatingByte = header.isRepeatingByte;
    asyncWriteState.repeatingByte = header.repeatingByte;
  }
}

uint8_t EEPROM_ReadByte(uint16_t address) {
  // Wait for any previous pending EEPROM write to complete
  while (NVMCON0bits.GO);
  
  // Set NVMADR with the target address (0x380000 - 0x3803FF)
  NVMADRU = 0x38;
  NVMADRH = (uint8_t) ((address & 0x0300) >> 8);
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
  NVMADRH = (uint8_t) ((address & 0x0300) >> 8);
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
  NVMADRH = (uint8_t) ((address & 0x0300) >> 8);
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

    uint8_t oldByte = NVMDATL;
    
    // Check if the existing byte in EEPROM is different than what we want to write
    if (oldByte != nextByte) {
      while(NVMCON0bits.GO);
      
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
  return EEPROM_AsyncWriteByteN(address, value, 1);
}

bool EEPROM_AsyncWriteByteN(uint16_t address, uint8_t value, uint16_t repeat) {
  // Confirm that there's enough room in the buffer for this byte and a header
  if (EEPROM_WRITE_BUFFER_HEADER_SIZE > writeBufferState.remaining) {
    printf("[EEPROM] Buffer Overflow!\r\n");
    return false;
  }

  write_buffer_header_t header;
  
  header.addressLow = address & 0x00FF;
  header.addressHigh = (address & 0x0300) >> 8;
  header.sizeLow = repeat & 0x00FF;
  header.sizeHigh = (repeat & 0x0700) >> 8;
  header.isRepeatingByte = true;
  header.repeatingByte = value;
  
  writeHeaderToWriteBuffer(&header);
  
  return true;
}

bool EEPROM_AsyncWriteBytes(uint16_t address, void const* data, uint8_t size) {
  // Confirm that there's enough room in the buffer for this data and a header
  if (size + EEPROM_WRITE_BUFFER_HEADER_SIZE > writeBufferState.remaining) {
    printf("[EEPROM] Buffer Overflow!\r\n");
    return false;
  }

  write_buffer_header_t header;
  
  header.addressLow = address & 0x00FF;
  header.addressHigh = (address & 0x0300) >> 8;
  header.sizeLow = size;
  header.sizeHigh = 0;
  header.isRepeatingByte = false;
  
  writeHeaderToWriteBuffer(&header);
  
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

void EEPROM_AsyncErase(void) {
  // Reinitialize this EEPROM module to reset/clear all previously pending 
  // buffered writes.
  EEPROM_Initialize();
  // Write 0xFF to every address in EEPROM
  EEPROM_AsyncWriteByteN(0, 0xFF, EEPROM_SIZE);
}

bool EEPROM_IsDoneWriting(void) {
  return (writeBufferState.remaining == EEPROM_WRITE_BUFFER_SIZE) && !NVMCON0bits.GO;
}