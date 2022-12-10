/** 
 * @file
 * @author Jeff Lau
 * 
 * Convenience functions for reading/writing data from/to EEPROM.
 */

#ifndef EEPROM_H
#define	EEPROM_H

#include <stdint.h>
#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

/**
 * Initialize this module.
 */ 
void EEPROM_Initialize(void);

/**
 * The main task implementation of this module.
 * 
 * This must be called from the main task loop of the application.
 */
void EEPROM_Task(void);

/**
 * Read a single byte from EEPROM.
 * 
 * @param address - The EEPROM address to read.
 * @return The byte value from EEPROM.
 */
uint8_t EEPROM_ReadByte(uint16_t address);

/**
 * Read multiple bytes of data from EEPROM.
 * 
 * @param address - The EEPROM address to start reading from.
 * @param dest - A pointer to the destination buffer where data will be written.
 * @param size - The number of bytes to read from EEPROM.
 * @return The `dest` pointer for convenience.
 */
void* EEPROM_ReadBytes(uint16_t address, void *dest, uint16_t size);

/**
 * Write a single byte to EEPROM right now, and wait for the write to complete.
 * 
 * @param address - The EEPROM address to write.
 * @param value - The value to write to EEPROM.
 */
void EEPROM_WriteByte(uint16_t address, uint8_t value);

/**
 * Write multiple bytes of data to EEPROM right now, and wait for the write 
 * of all bytes to complete.
 * 
 * @param address - The EEPROM address to start writing at.
 * @param data - A pointer to the data to write to EEPROM.
 * @param size - The number of bytes of data to write to EEPROM.
 */
void EEPROM_WriteBytes(uint16_t address, void const* data, uint16_t size);

/**
 * Write a single byte of data to EEPROM asynchronously while allowing the 
 * main task loop of the application to continue running now.
 * 
 * @param address - The EEPROM address to write.
 * @param value - The value to write to EEPROM.
 * @return True if the byte was successfully added to the write buffer. 
 *         False if there was not sufficient room in the buffer.
 */
bool EEPROM_AsyncWriteByte(uint16_t address, uint8_t value);

/**
 * Write multiple bytes of data to EEPROM asynchronously while allowing the 
 * main task loop of the application to continue running now.
 * 
 * @param address - The EEPROM address to start writing at.
 * @param data - A pointer to the data to write to EEPROM.
 * @param size - The number of bytes of data to write to EEPROM.
 * @return True if the data was successfully added to the write buffer. 
 *         False if there was not sufficient room in the buffer.
 */
bool EEPROM_AsyncWriteBytes(uint16_t address, void const* data, uint8_t size);

/**
 * Test if all current/pending EEPROM writes are complete.
 * @return True if all current/pending EEPROM writes are complete.
 */
bool EEPROM_IsDoneWriting(void);

#ifdef	__cplusplus
}
#endif

#endif	/* EEPROM_H */

