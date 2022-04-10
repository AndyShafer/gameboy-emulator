#pragma once

#include <cstdint>
#include <vector>
#include <functional>

#include "memory-map.h"
#include "cpu-registers.h"

namespace gbemulator {

enum InstructionStatus {
	WRITE_FAIL = -1,
	OK = 0,
	STOP = 1,
	HALT = 2
};

class InstructionSet {
public:
	InstructionSet(CpuRegisters *registers, MemoryMap *memory);
	InstructionStatus exec(uint8_t opcode);
private:
	CpuRegisters *registers;
	MemoryMap *memory;
	std::vector<std::function<InstructionStatus()>> instructions;
};

}
