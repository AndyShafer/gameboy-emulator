#include "instruction-set.h"

#define INSTRUCTIONS 0x100

namespace gbemulator {

	InstructionSet::InstructionSet(CpuRegisters *registers, MemoryMap *memory) {
		instructions.push_back([](){});
	}

	void InstructionSet::exec(uint8_t opcode) {
		instructions[opcode]();
	}

}
