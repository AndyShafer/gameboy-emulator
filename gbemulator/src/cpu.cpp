#include "cpu.h"

namespace gbemulator {

	Cpu::Cpu() {
		memory = new MemoryMap();
		instructions = new InstructionSet(&registers, memory);
	}

	void Cpu::run() {
		while(true) {
			uint16_t& PC = registers.registers16.PC;
			uint8_t opcode = memory->read8(PC++);
			instructions->exec(opcode);
		}		
	}
}
