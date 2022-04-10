#pragma once

#include "register-map.h"

#include <cstdint>

#define ADDRESS_SPACE 0x10000

namespace gbemulator {

class MemoryMap {
public:
	MemoryMap();
	uint8_t read8(uint16_t addr) const;
	uint16_t read16(uint16_t addr) const;
	bool write8(uint16_t addr, uint8_t val);
	bool write16(uint16_t addr, uint16_t val);
private:
	RegisterMap *registerMap;
	uint8_t *mem;
};

}
