#pragma once

#include <cstdint>
#include <vector>

#include "memory-map.h"
#include "cpu-registers.h"

namespace gbemulator {

class InstructionSet {
public:
	InstructionSet(CpuRegisters *registers, MemoryMap *memory);
	void exec(uint8_t opcode);
private:
	std::vector<void (*)()> instructions;
};

}
