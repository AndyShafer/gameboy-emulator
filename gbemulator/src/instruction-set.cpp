#include "instruction-set.h"

#define INSTRUCTIONS 0x100

namespace gbemulator {

	InstructionSet::InstructionSet(CpuRegisters *registers, MemoryMap *memory)
       		: registers(registers), memory(memory) {
		instructions = std::vector<std::function<InstructionStatus()>>(0x100);

		// NOP
		instructions[0x00] = [](){ return OK; };

		// STOP
		instructions[0x10] = [](){ return STOP; };

		// HALT
		instructions[0x76] = [](){ return HALT; };

		// LD
		// 8-bit Loads
		instructions[0x02] = [&registers, &memory]() {
			if(memory->write8(registers->get16BitReg(BC), registers->get8BitReg(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0x12] = [&registers, &memory]() {
			if(memory->write8(registers->get16BitReg(DE), registers->get8BitReg(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0x22] = [&registers, &memory]() {
			if(memory->write8(registers->get16BitReg(HL)++, registers->get8BitReg(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0x32] = [&registers, &memory]() {
			if(memory->write8(registers->get16BitReg(HL)--, registers->get8BitReg(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0x0A] = [&registers, &memory]() {
			registers->get8BitReg(A) = memory->read8(registers->get16BitReg(BC));
			return OK;
		};
		instructions[0x1A] = [&registers, &memory]() {
			registers->get8BitReg(A) = memory->read8(registers->get16BitReg(DE));
			return OK;
		};
		instructions[0x2A] = [&registers, &memory]() {
			registers->get8BitReg(A) = memory->read8(registers->get16BitReg(HL)++);
			return OK;
		};
		instructions[0x3A] = [&registers, &memory]() {
			registers->get8BitReg(A) = memory->read8(registers->get16BitReg(HL)--);
			return OK;
		};
		for(int i = 0x0; i <= 0x3; i++) {
			if(i != 0x3) {
				instructions[(i << 4) + 0x6] = [&registers, &memory, i]() {
					registers->get8BitReg(i*2) = memory->read8(++registers->get16BitReg(PC));
					return OK;
				};
			}
			instructions[(i << 4) + 0x6] = [&registers, &memory, i]() {
				registers->get8BitReg(i*2+1) = memory->read8(++registers->get16BitReg(PC));
				return OK;
			};
		}
		instructions[0x36] = [&registers, &memory]() {
			if(memory->write8(registers->get16BitReg(HL), memory->read8(++registers->get16BitReg(PC)))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		for(int i = 0x4; i <= 0x6; ++i) {
			for(int j = 0x0; j <= 0x7; j++) {
				if(j == 0x6) continue;
				instructions[(i << 4) + j] = [&registers, i, j]() {
					registers->get8BitReg((i-0x4)*2) = registers->get8BitReg(j);
					return OK;
				};
			}
			for(int j = 0x8; j <= 0xF; j++) {
				if(j == 0xE) continue;
				instructions[(i << 4) + j] = [&registers, i, j]() {
					registers->get8BitReg((i-0x4)*2+1) = registers->get8BitReg(j-0x8);
					return OK;
				};
			}
			for(int j = 0; j <= 1; j++) {
				instructions[(i << 4) + j * 0x8 + 0x6] = [&registers, &memory, i, j]() {
					registers->get8BitReg((i-0x4)*2+j) = memory->read8(registers->get16BitReg(HL));
					return OK;
				};
			}
		}
		for(int i = 0x0; i <= 0x7; i++) {
			if(i == 0x6) continue;
			instructions[0x70 + i] = [&registers, &memory, i]() {
				if(memory->write8(registers->get16BitReg(HL), registers->get8BitReg(i))) {
					return OK;
				}
				return WRITE_FAIL;
			};
		}
		for(int i = 0x8; i <= 0xF; i++) {
			if(i == 0xE) continue;
			instructions[0x70 + i] = [&registers, i]() {
				registers->get8BitReg(A) = registers->get8BitReg(i-0x8);
				return OK;
			};
		}
		instructions[0x7E] = [&registers, &memory]() {
			registers->get8BitReg(A) = memory->read8(registers->get16BitReg(HL));
			return OK;
		};
		instructions[0xE0] = [&registers, &memory]() {
			if(memory->write8(0xFF00 + memory->read8(++registers->get16BitReg(PC)), registers->get8BitReg(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0xF0] = [&registers, &memory]() {
			registers->get8BitReg(A) = memory->read8(0xFF00 + memory->read8(++registers->get16BitReg(PC)));
			return OK;
		};
		instructions[0xE2] = [&registers, &memory]() {
			if(memory->write8(0xFF00 + registers->get8BitReg(C), registers->get8BitReg(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0xF2] = [&registers, &memory]() {
			registers->get8BitReg(A) = memory->read8(0xFF00 + registers->get8BitReg(C));
			return OK;
		};
		instructions[0xEA] = [&registers, &memory]() {
			if(memory->write8(memory->read8(++registers->get16BitReg(PC)) + (memory->read8(++registers->get16BitReg(PC) << 8)),
					registers->get8BitReg(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0xFA] = [&registers, &memory]() {
			registers->get8BitReg(A) = memory->read8(memory->read8(++registers->get16BitReg(PC)) +
					(memory->read8(++registers->get16BitReg(PC) << 8)));
			return OK;
		};
	}

	// Returns a status code.
	InstructionStatus InstructionSet::exec(uint8_t opcode) {
		return instructions[opcode]();
	}

}
