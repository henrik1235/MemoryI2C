#ifndef _MemoryI2C_h
#define _MemoryI2C_h

#include <Arduino.h>
#include <Wire.h>

#define MEMORY_I2C_ADDRESS (0b1010000u)
#define MEMORY_ERROR_WRONG_PARAMETERS (0xFEu)
#define MEMORY_ERROR_READ_ERROR (0xFDu)
#define MEMORY_ERROR_PAGE_OVERFLOW (0xFCu)

#define MEMORY_MIN_WAIT_TIME 10
#define MEMORY_WAIT_TIMEOUT 500
#define MEMORY_PAGE_SIZE 16

// Generic implementation of an EEPROM I2C interface.
class MemoryI2C {
private:
	int addr;
	size_t flashSize;
	size_t pageSize;

	// Asserts the address.
	boolean assertAddress(int address);
	// Asserts the memory region starting at address with the specified size.
	boolean assertAddress(int address, size_t size);

	// Checks the connection to the chip and returns true when the chip is ready.
	boolean checkChip();
	// Waits for the chip to be ready.
	void waitForChip();

	uint8_t readPage(int start, uint8_t* buffer, size_t size);
	uint8_t writePage(int start, uint8_t* buffer, size_t size);
public: 
	// Inits the library with the settings for the chip used and the default address with 0.
	boolean begin(size_t flashSize, size_t pageSize);
	// Inits the library with the settings for the chip used.
	boolean begin(int addr, size_t flashSize, size_t pageSize);

	// Returns the size of the flash memory in bytes.
	size_t getFlashSize();
	// Returns the size of one flash page in bytes.
	size_t getPageSize();

	// Reads one byte at the address specified.
	uint8_t readByte(int address);
	// Reads size bytes starting at the address start. Returns an I2C error code.
	uint8_t readBytes(int start, uint8_t* buffer, size_t size);

	// Writes on the bytes to the address specified. Returns an I2C error code.
	uint8_t writeByte(int address, uint8_t value);
	// Writes size bytes starting at the address start. Returns an I2C error code.
	uint8_t writeBytes(int start, uint8_t* buffer, size_t size);
};

#endif