#include "memory-map.h"

namespace gbemulator {

	MemoryMap::MemoryMap() {
		mem = new uint8_t[ADDRESS_SPACE];
		registerMap = new RegisterMap(mem + 0xFF00);
	}

	uint8_t MemoryMap::read8(uint16_t addr)  const {
		return mem[addr];
	}

	uint16_t MemoryMap::read16(uint16_t addr) const {
		return mem[addr] + (mem[addr+1] << 8);
	}

	bool MemoryMap::write8(uint16_t addr, uint8_t val) {
		mem[addr] = val;
		return true;
	}

	bool MemoryMap::write16(uint16_t addr, uint16_t val) {
		mem[addr] = val;
		mem[addr+1] = val >> 8;
		return true;
	}

}
