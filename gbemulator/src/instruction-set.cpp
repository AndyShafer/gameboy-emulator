#include "instruction-set.h"

#define INSTRUCTIONS 0x100
#define REG8(ID) registers->get8BitReg((ID))
#define REG16(ID) registers->get16BitReg((ID))
#define READ_ADDR8(X) memory->read8(0xFF00 + (X))
#define READ_ADDR16(X) memory->read8((X))
#define WRITE_ADDR8(X, Y) memory->write8(0xFF00 + (X), (Y))
#define WRITE_ADDR16(X, Y) memory->write8((X), (Y))
#define PREINC(REG) ++REG
#define PREDEC(REG) --REG
#define POSTINC(REG) REG++
#define POSTDEC(REG) REG++
#define N READ_ADDR16(POSTINC(REG16(PC)))
#define NN N + (N << 4)

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
			if(WRITE_ADDR16(REG16(BC), REG8(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0x12] = [&registers, &memory]() {
			if(WRITE_ADDR16(REG16(DE), REG8(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0x22] = [&registers, &memory]() {
			if(WRITE_ADDR16(POSTINC(REG16(HL)), REG8(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0x32] = [&registers, &memory]() {
			if(WRITE_ADDR16(POSTDEC(REG16(HL)), REG8(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0x0A] = [&registers, &memory]() {
			REG8(A) = READ_ADDR16(REG16(BC));
			return OK;
		};
		instructions[0x1A] = [&registers, &memory]() {
			REG8(A) = READ_ADDR16(REG16(DE));
			return OK;
		};
		instructions[0x2A] = [&registers, &memory]() {
			REG8(A) = READ_ADDR16(POSTINC(REG16(HL)));
			return OK;
		};
		instructions[0x3A] = [&registers, &memory]() {
			REG8(A) = READ_ADDR16(POSTDEC(REG16(HL)));
			return OK;
		};
		for(int i = 0x0; i <= 0x3; i++) {
			if(i != 0x3) {
				instructions[(i << 4) + 0x6] = [&registers, &memory, i]() {
					REG8(i*2) = N;
					return OK;
				};
			}
			instructions[(i << 4) + 0x6] = [&registers, &memory, i]() {
				REG8(i*2+1) = N;
				return OK;
			};
		}
		instructions[0x36] = [&registers, &memory]() {
			if(WRITE_ADDR16(REG16(HL), N)) {
				return OK;
			}
			return WRITE_FAIL;
		};
		for(int i = 0x4; i <= 0x6; ++i) {
			for(int j = 0x0; j <= 0x7; j++) {
				if(j == 0x6) continue;
				instructions[(i << 4) + j] = [&registers, i, j]() {
					REG8((i-0x4)*2) = REG8(j);
					return OK;
				};
			}
			for(int j = 0x8; j <= 0xF; j++) {
				if(j == 0xE) continue;
				instructions[(i << 4) + j] = [&registers, i, j]() {
					REG8((i-0x4)*2+1) = REG8(j-0x8);
					return OK;
				};
			}
			for(int j = 0; j <= 1; j++) {
				instructions[(i << 4) + j * 0x8 + 0x6] = [&registers, &memory, i, j]() {
					REG8((i-0x4)*2+j) = READ_ADDR16(REG16(HL));
					return OK;
				};
			}
		}
		for(int i = 0x0; i <= 0x7; i++) {
			if(i == 0x6) continue;
			instructions[0x70 + i] = [&registers, &memory, i]() {
				if(WRITE_ADDR16(REG16(HL), REG8(i))) {
					return OK;
				}
				return WRITE_FAIL;
			};
		}
		for(int i = 0x8; i <= 0xF; i++) {
			if(i == 0xE) continue;
			instructions[0x70 + i] = [&registers, i]() {
				REG8(A) = REG8(i-0x8);
				return OK;
			};
		}
		instructions[0x7E] = [&registers, &memory]() {
			REG8(A) = READ_ADDR8(REG16(HL));
			return OK;
		};
		instructions[0xE0] = [&registers, &memory]() {
			if(WRITE_ADDR8(N, REG8(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0xF0] = [&registers, &memory]() {
			REG8(A) = READ_ADDR8(N);
			return OK;
		};
		instructions[0xE2] = [&registers, &memory]() {
			if(WRITE_ADDR8(REG8(C), REG8(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0xF2] = [&registers, &memory]() {
			REG8(A) = READ_ADDR8(REG8(C));
			return OK;
		};
		instructions[0xEA] = [&registers, &memory]() {
			if(WRITE_ADDR16(NN, REG8(A))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		instructions[0xFA] = [&registers, &memory]() {
			REG8(A) = READ_ADDR8(NN);
			return OK;
		};
	}

	// Returns a status code.
	InstructionStatus InstructionSet::exec(uint8_t opcode) {
		return instructions[opcode]();
	}

}
