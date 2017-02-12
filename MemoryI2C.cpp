#include "MemoryI2C.h"

boolean MemoryI2C::begin(size_t flashSize, size_t pageSize) {
	return begin(0, flashSize, pageSize);
}

boolean MemoryI2C::begin(int addr, size_t flashSize, size_t pageSize) {
	this->addr = MEMORY_I2C_ADDRESS | min(0b111, addr);
	this->flashSize = flashSize;
	this->pageSize = pageSize;

	return checkChip();
}

boolean MemoryI2C::assertAddress(int address) {
	return address >= 0 && address < getFlashSize();
}

boolean MemoryI2C::assertAddress(int address, size_t size) {
	return assertAddress(address) && assertAddress(address + size);
}

boolean MemoryI2C::checkChip() {
	Wire.beginTransmission(addr);
	return Wire.endTransmission() == 0;
}

void MemoryI2C::waitForChip() {
	delay(MEMORY_MIN_WAIT_TIME);
	int start = millis();
	while (millis() - start < MEMORY_WAIT_TIMEOUT) {
		if (checkChip())
			return;
		delay(5);
	}
}

size_t MemoryI2C::getFlashSize() {
	return flashSize;
}

size_t MemoryI2C::getPageSize() {
	return pageSize;
}

uint8_t MemoryI2C::readPage(int start, uint8_t* buffer, size_t size) {
	if (!assertAddress(start, size))
		return MEMORY_ERROR_WRONG_PARAMETERS;
	if (size == 0)
		return 0;

	Wire.beginTransmission(addr);
	if (getFlashSize() > 256) 
		Wire.write((uint8_t)(start >> 8));
	Wire.write((uint8_t)(start & 0xFF));
	uint8_t error = Wire.endTransmission();
	if (error != 0)
		return error;

	if (Wire.requestFrom(addr, size) < size)
		return MEMORY_ERROR_READ_ERROR;

	for (int i = 0; i < size; i++)
		buffer[i] = Wire.read();
	return 0;
}

uint8_t MemoryI2C::readByte(int address) {
	uint8_t buffer[1];
	if (readBytes(address, buffer, sizeof(buffer)) != 0)
		return 0;
	return buffer[0];
}

uint8_t MemoryI2C::readBytes(int start, uint8_t* buffer, size_t size) {
	if (!assertAddress(start, size))
		return MEMORY_ERROR_WRONG_PARAMETERS;
	if (buffer == NULL)
		return MEMORY_ERROR_WRONG_PARAMETERS;

	while (size > 0) {
		size_t bytesToRead = min(min(size, getPageSize()), MEMORY_PAGE_SIZE);
		uint8_t error = readPage(start, buffer, bytesToRead);
		if (error != 0)
			return error;

		start += bytesToRead;
		buffer += bytesToRead;
		size -= bytesToRead;
	}
	return 0;
}

uint8_t MemoryI2C::writePage(int start, uint8_t* buffer, size_t size) {
	if (!assertAddress(start, size))
		return MEMORY_ERROR_WRONG_PARAMETERS;
	if (size == 0)
		return 0;

	int bufferPosition = start % getPageSize();
	if (bufferPosition + size > getPageSize())
		return MEMORY_ERROR_PAGE_OVERFLOW;

	Wire.beginTransmission(addr);
	if (getFlashSize() > 256)
		Wire.write((uint8_t)(start >> 8));
	Wire.write((uint8_t)(start & 0xFF));

	for (int i = 0; i < size; i++)
		Wire.write(buffer[i]);
	uint8_t error = Wire.endTransmission();
	if (error == 0)
		waitForChip();
	return error;
}

uint8_t MemoryI2C::writeByte(int address, uint8_t value) {
	uint8_t buffer[1];
	buffer[0] = value;
	return writeBytes(address, buffer, sizeof(buffer));
}

uint8_t MemoryI2C::writeBytes(int start, uint8_t* buffer, size_t size) {
	if (!assertAddress(start, size))
		return MEMORY_ERROR_WRONG_PARAMETERS;
	if (buffer == NULL)
		return MEMORY_ERROR_WRONG_PARAMETERS;

	while (size > 0) {
		int pagePos = start % getPageSize();
		size_t bytesToWrite = min(min(size, getPageSize() - pagePos), MEMORY_PAGE_SIZE);

		uint8_t error = writePage(start, buffer, bytesToWrite);
		if (error != 0)
			return error;

		start += bytesToWrite;
		buffer += bytesToWrite;
		size -= bytesToWrite;
	}
	return 0;
}