#include "instruction-set.h"

#define INSTRUCTIONS 0x100
#define REG8(ID) registers->get8BitReg((ID))
#define REG16(ID) registers->get16BitReg((ID))
#define READ_ADDR8(X) memory->read8(0xFF00 + (X))
#define READ_ADDR16(X) memory->read8((X))
#define WRITE_ADDR8(X, Y) memory->write8(0xFF00 + (X), (Y))
#define WRITE_ADDR16(X, Y) memory->write8((X), (Y))
#define READ16(X) memory->read16((X))
#define WRITE16(X, Y) memory->write16((X), (Y))
#define HL_READ memory->read8(REG16(HL))
#define HL_WRITE(X) memory->write8(REG16(HL), (X))
#define PREINC(REG) ++(REG)
#define PREDEC(REG) --(REG)
#define POSTINC(REG) (REG)++
#define POSTDEC(REG) (REG)++
#define N READ_ADDR16(POSTINC(REG16(PC)))
#define NN N + (N << 4)
#define SIGNED_IMM static_cast<int8_t>(N)
#define SET_C(X) registers->setFlag(FLAG_C, static_cast<bool>(X))
#define SET_N(X) registers->setFlag(FLAG_N, static_cast<bool>(X))
#define SET_H(X) registers->setFlag(FLAG_H, static_cast<bool>(X))
#define SET_Z(X) registers->setFlag(FLAG_Z, static_cast<bool>(X))
#define GET_C()  (registers->getFlag(FLAG_C) ? 1 : 0)
#define GET_N()  (registers->getFlag(FLAG_N) ? 1 : 0)
#define GET_H()  (registers->getFlag(FLAG_H) ? 1 : 0)
#define GET_Z()  (registers->getFlag(FLAG_Z) ? 1 : 0)
#define LOWER_NIBBLE(X) ((X) & 0x0F)
#define UPPER_NIBBLE(X) ((X) & 0xF0)
#define UPPER_NIBBLE_SHIFTED(X) (UPPER_NIBBLE(X) >> 4)
#define CALL(ADDR) \
	REG16(SP) -= 2; \
	if(!WRITE16(REG16(SP), REG16(PC))) { \
		return WRITE_FAIL; \
	} \
	REG16(PC) = ADDR
#define RET() \
	REG16(PC) = READ16(REG16(SP)); \
	REG16(SP) += 2

namespace gbemulator {

	InstructionSet::InstructionSet(CpuRegisters *registers, MemoryMap *memory)
       		: registers(registers), memory(memory) {
		instructions = std::vector<std::function<InstructionStatus()>>(0x100);
		cbInstructions = std::vector<std::function<InstructionStatus()>>(0x100);

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
		// 16-bit loads
		// LD XX,nn
		for(int i = 0x0; i <= 0x3; ++i) {
			instructions[(i << 4) + 0x1] = [&registers, &memory, i]() {
				REG16(i) = READ_ADDR16(NN);
				return OK;
			};
		}
		// LD (nn),SP
		instructions[0x08] = [&registers, &memory]() {
			if(WRITE_ADDR16(NN, REG16(SP))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		// POP BC
		instructions[0xC1] = [&registers, &memory]() {
			REG16(BC) = READ16(POSTINC(REG16(SP)));
			PREINC(REG16(SP));
			return OK;
		};
		// POP DE
		instructions[0xD1] = [&registers, &memory]() {
			REG16(DE) = READ16(POSTINC(REG16(SP)));
			PREINC(REG16(SP));
			return OK;
		};
		// POP HL
		instructions[0xE1] = [&registers, &memory]() {
			REG16(HL) = READ16(POSTINC(REG16(SP)));
			PREINC(REG16(SP));
			return OK;
		};
		// POP AF
		instructions[0xF1] = [&registers, &memory]() {
			REG16(AF) = READ16(POSTINC(REG16(SP)));
			PREINC(REG16(SP));
			return OK;
		};
		// PUSH BC
		instructions[0xC5] = [&registers, &memory]() {
			if(WRITE16(PREDEC(PREDEC(REG16(SP))), REG16(BC))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		// PUSH DE
		instructions[0xD5] = [&registers, &memory]() {
			if(WRITE16(PREDEC(PREDEC(REG16(SP))), REG16(DE))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		// PUSH HL
		instructions[0xE5] = [&registers, &memory]() {
			if(WRITE16(PREDEC(PREDEC(REG16(SP))), REG16(HL))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		// PUSH AF
		instructions[0xF5] = [&registers, &memory]() {
			if(WRITE16(PREDEC(PREDEC(REG16(SP))), REG16(AF))) {
				return OK;
			}
			return WRITE_FAIL;
		};
		// LD HL,SP+e
		instructions[0xF8] = [&registers, &memory]() {
			REG16(HL) = REG16(SP) + SIGNED_IMM;
			return OK;
		};
		// LD SP,HL
		instructions[0xF9] = [&registers, &memory]() {
			REG16(SP) = REG16(HL);
			return OK;
		};
		
		// 8-bit arithmetic
		// INC R
		for(int i = 0x0; i <= 0x2; i++) {
			instructions[(i << 4) + 0x4] = [&registers, &memory, i]() {
				PREINC(REG8(i*2));
				SET_Z(REG8(i*2) == 0);
				SET_N(false);
				SET_H(LOWER_NIBBLE(REG8(i*2)) == 0);
				return OK;
			};
			instructions[(i << 4) + 0xC] = [&registers, &memory, i]() {
				PREINC(REG8(i*2+1));
				SET_Z(REG8(i*2+1) == 0);
				SET_N(false);
				SET_H(LOWER_NIBBLE(REG8(i*2+1)) == 0);
				return OK;
			};
		}
		// INC (HL)
		instructions[0x34] = [&registers, &memory]() {
			uint8_t val = READ_ADDR16(REG16(HL)) + 1;
			if(WRITE_ADDR16(REG16(HL), val)) {
				SET_Z(val == 0);
				SET_N(false);
				SET_H(LOWER_NIBBLE(val) == 0);
				return OK;
			}
			return WRITE_FAIL;
		};
		// INC A
		instructions[0x3C] = [&registers, &memory]() {
			PREINC(REG8(A));
			SET_Z(REG8(A) == 0);
			SET_N(false);
			SET_H(LOWER_NIBBLE(REG8(A)) == 0);
			return OK;
		};
		// DEC X
		for(int i = 0x0; i <= 0x2; i++) {
			instructions[(i << 4) + 0x5] = [&registers, &memory, i]() {
				PREDEC(REG8(i*2));
				SET_Z(REG8(i*2) == 0);
				SET_N(true);
				SET_H(LOWER_NIBBLE(REG8(i*2)) == 0);
				return OK;
			};
			instructions[(i << 4) + 0xD] = [&registers, &memory, i]() {
				PREDEC(REG8(i*2+1));
				SET_Z(REG8(i*2+1) == 0);
				SET_N(true);
				SET_H(LOWER_NIBBLE(REG8(i*2+1)) == 0);
				return OK;
			};
		}
		// DEC (HL)
		instructions[0x35] = [&registers, &memory]() {
			uint8_t val = READ_ADDR16(REG16(HL)) - 1;
			if(WRITE_ADDR16(REG16(HL), val)) {
				SET_Z(val == 0);
				SET_N(true);
				SET_H(LOWER_NIBBLE(val) == 0);
				return OK;
			}
			return WRITE_FAIL;
		};
		// DEC A
		instructions[0x3D] = [&registers, &memory]() {
			PREDEC(REG8(A));
			SET_Z(REG8(A) == 0);
			SET_N(false);
			SET_H(LOWER_NIBBLE(REG8(A)) == 0);
			return OK;
		};
		// DAA
		instructions[0x27] = [&registers, &memory] () {
			SET_C(false);
			if(REG8(A) >= 0xA0) {
				REG8(A) -= 0xA0;
				SET_C(true);
			}
			if(REG8(A) & 0x0F >= 10) {
				REG8(A) += (0x10 - 10);
				if(REG8(A) >= 0xA0) {
					REG8(A) -= 0xA0;
					SET_C(true);
				}
			}
			SET_Z(REG8(A) == 0);
			
			return OK;
		};
		// SCF
		instructions[0x37] = [&registers, &memory] () {
			registers->setFlags("-001"); 
			return OK;
		};
		// CPL
		instructions[0x2F] = [&registers, &memory] () {
			REG8(A) = ~REG8(A);
			registers->setFlags("-11-");
			return OK;
		};
		// CCF
		instructions[0x3F] = [&registers, &memory] () {
			registers->setFlags("-00x");
			return OK;
		};
		// ADD R
		for(int i = 0x0; i <= 0x7; i++) {
			if(i == 0x6) continue;
			instructions[0x80 + i] = [&registers, &memory, i] () {
				SET_H(LOWER_NIBBLE(REG8(A)) + LOWER_NIBBLE(REG8(i)) > 0xF);
				SET_C(static_cast<int>(REG8(A)) + REG8(i) > 0xFF);
				REG8(A) += REG8(i);
				SET_Z(REG8(A) == 0);
				SET_N(false);
				return OK;
			};
		}
		// SUB R
		for(int i = 0x0; i <= 0x7; i++) {
			if(i == 0x6) continue;
			instructions[0x90 + i] = [&registers, &memory, i] () {
				SET_H(static_cast<int>(LOWER_NIBBLE(REG8(A))) - LOWER_NIBBLE(REG8(i)) < 0);
				SET_C(static_cast<int>(REG8(A)) - REG8(i) < 0);
				REG8(A) -= REG8(i);
				SET_Z(REG8(A) == 0);
				SET_N(true);
				return OK;
			};
		}
		// AND R
		for(int i = 0x0; i <= 0x7; i++) {
			if(i == 0x6) continue;
			instructions[0xA0 + i] = [&registers, &memory, i] () {
				REG8(A) &= REG8(i);
				SET_Z(REG8(A) == 0);
				registers->setFlags("-010");
				return OK;
			};
		}
		// OR R
		for(int i = 0x0; i <= 0x7; i++) {
			if(i == 0x6) continue;
			instructions[0xA0 + i] = [&registers, &memory, i] () {
				REG8(A) |= REG8(i);
				SET_Z(REG8(A) == 0);
				registers->setFlags("-000");
				return OK;
			};
		}
		// ADC R
		for(int i = 0x9; i <= 0xF; i++) {
			if(i == 0xE) continue;
			instructions[0x80 + i] = [&registers, &memory, i] () {
				SET_H(LOWER_NIBBLE(REG8(A)) + LOWER_NIBBLE(REG8(i-0x9)) + GET_C() > 0xF);
				bool c = static_cast<int>(REG8(A)) + REG8(i-0x9) + GET_C() > 0xFF;
				REG8(A) += REG8(i-0x9) + GET_C();
				SET_C(c);
				SET_Z(REG8(A) == 0);
				SET_N(false);
				return OK;
			};
		}
		// SBC R
		for(int i = 0x9; i <= 0xF; i++) {
			if(i == 0xE) continue;
			instructions[0x90 + i] = [&registers, &memory, i] () {
				SET_H(static_cast<int>(LOWER_NIBBLE(REG8(A))) - LOWER_NIBBLE(REG8(i-0x9)) - GET_C() < 0);
				bool c = static_cast<int>(REG8(A)) - REG8(i-0x9) - GET_C() < 0;
				REG8(A) -= REG8(i-0x9) + GET_C();
				SET_C(c);
				SET_Z(REG8(A) == 0);
				SET_N(true);
				return OK;
			};
		}
		// XOR R
		for(int i = 0x9; i <= 0xF; i++) {
			if(i == 0xE) continue;
			instructions[0xA0 + i] = [&registers, &memory, i] () {
				REG8(A) ^= REG8(i-0x9);
				SET_Z(REG8(A) == 0);
				registers->setFlags("-000");
				return OK;
			};
		}
		// CP R
		for(int i = 0x9; i <= 0xF; i++) {
			if(i == 0xE) continue;
			instructions[0xB0 + i] = [&registers, &memory, i] () {
				SET_Z(REG8(A) - REG8(i-0x9) == 0);
				SET_N(true);
				SET_H(static_cast<int>(LOWER_NIBBLE(REG8(A))) - LOWER_NIBBLE(REG8(i-0x9)) < 0);
				SET_C(static_cast<int>(REG8(A)) - REG8(i-0x9) < 0);
				return OK;
			};
		}
		// ADD (HL)
		instructions[0x86] = [&registers, &memory]() {
			uint8_t val = HL_READ;
			SET_H(LOWER_NIBBLE(REG8(A)) + LOWER_NIBBLE(val) > 0xF);
			SET_C(static_cast<int>(REG8(A)) + val > 0xFF);
			REG8(A) += val;
			SET_Z(REG8(A) == 0);
			SET_N(false);
			return OK;
		};
		// SUB (HL)
		instructions[0x96] = [&registers, &memory]() {
			uint8_t val = HL_READ;
			SET_H(static_cast<int>(LOWER_NIBBLE(REG8(A))) - LOWER_NIBBLE(val) < 0);
			SET_C(static_cast<int>(REG8(A)) - val < 0);
			REG8(A) -= val;
			SET_Z(REG8(A) == 0);
			SET_N(true);
			return OK;
		};
		// AND (HL)
		instructions[0xA6] = [&registers, &memory]() {
			REG8(A) &= HL_READ;
			SET_Z(REG8(A) == 0);
			registers->setFlags("-010");
			return OK;
		};
		// OR (HL)
		instructions[0xB6] = [&registers, &memory]() {
			REG8(A) |= HL_READ;
			SET_Z(REG8(A) == 0);
			registers->setFlags("-000");
			return OK;
		};
		// ADC (HL)
		instructions[0x8F] = [&registers, &memory]() {
			uint8_t val = HL_READ;
			SET_H(LOWER_NIBBLE(REG8(A)) + LOWER_NIBBLE(val) + GET_C() > 0xF);
			SET_C(static_cast<int>(REG8(A)) + val + GET_C() > 0xFF);
			REG8(A) += val + GET_C();
			SET_Z(REG8(A) == 0);
			SET_N(false);
			return OK;
		};
		// SBC (HL)
		instructions[0x9F] = [&registers, &memory]() {
			uint8_t val = HL_READ;
			SET_H(static_cast<int>(LOWER_NIBBLE(REG8(A))) - LOWER_NIBBLE(val) - GET_C() < 0);
			SET_C(static_cast<int>(REG8(A)) - val - GET_C() < 0);
			REG8(A) -= val + GET_C();
			SET_Z(REG8(A) == 0);
			SET_N(true);
			return OK;
		};
		// XOR (HL)
		instructions[0xAF] = [&registers, &memory]() {
			REG8(A) ^= HL_READ;
			SET_Z(REG8(A) == 0);
			registers->setFlags("-000");
			return OK;
		};
		// CP (HL)
		instructions[0xBF] = [&registers, &memory] () {
			uint8_t val = HL_READ;
			SET_Z(REG8(A) - REG8(val) == 0);
			SET_N(true);
			SET_H(static_cast<int>(LOWER_NIBBLE(REG8(A))) - LOWER_NIBBLE(val) < 0);
			SET_C(static_cast<int>(REG8(A)) - val < 0);
			return OK;
		};
		// ADD n
		instructions[0xC6] = [&registers, &memory] () {
			uint8_t val = N;
			SET_H(LOWER_NIBBLE(REG8(A)) + LOWER_NIBBLE(val) > 0xF);
			SET_C(static_cast<int>(REG8(A)) + val > 0xFF);
			REG8(A) += val;
			SET_Z(REG8(A) == 0);
			SET_N(false);
			return OK;
		};
		// SUB n
		instructions[0xD6] = [&registers, &memory]() {
			uint8_t val = N;
			SET_H(static_cast<int>(LOWER_NIBBLE(REG8(A))) - LOWER_NIBBLE(val) < 0);
			SET_C(static_cast<int>(REG8(A)) - val < 0);
			REG8(A) -= val;
			SET_Z(REG8(A) == 0);
			SET_N(true);
			return OK;
		};
		// AND n
		instructions[0xE6] = [&registers, &memory]() {
			REG8(A) &= N;
			SET_Z(REG8(A) == 0);
			registers->setFlags("-010");
			return OK;
		};
		// OR n
		instructions[0xF6] = [&registers, &memory]() {
			REG8(A) |= N;
			SET_Z(REG8(A) == 0);
			registers->setFlags("-000");
			return OK;
		};
		// ADC n
		instructions[0xCE] = [&registers, &memory]() {
			uint8_t val = N;
			SET_H(LOWER_NIBBLE(REG8(A)) + LOWER_NIBBLE(val) + GET_C() > 0xF);
			SET_C(static_cast<int>(REG8(A)) + val + GET_C() > 0xFF);
			REG8(A) += val + GET_C();
			SET_Z(REG8(A) == 0);
			SET_N(false);
			return OK;
		};
		// SBC n
		instructions[0xDE] = [&registers, &memory]() {
			uint8_t val = N;
			SET_H(static_cast<int>(LOWER_NIBBLE(REG8(A))) - LOWER_NIBBLE(val) - GET_C() < 0);
			SET_C(static_cast<int>(REG8(A)) - val - GET_C() < 0);
			REG8(A) -= val + GET_C();
			SET_Z(REG8(A) == 0);
			SET_N(true);
			return OK;
		};
		// XOR n
		instructions[0xEE] = [&registers, &memory]() {
			REG8(A) ^= N;
			SET_Z(REG8(A) == 0);
			registers->setFlags("-000");
			return OK;
		};
		// CP n
		instructions[0xEF] = [&registers, &memory] () {
			uint8_t val = N;
			SET_Z(REG8(A) - REG8(val) == 0);
			SET_N(true);
			SET_H(static_cast<int>(LOWER_NIBBLE(REG8(A))) - LOWER_NIBBLE(val) < 0);
			SET_C(static_cast<int>(REG8(A)) - val < 0);
			return OK;
		};

		// 16-bit arithmetic
		// INC RR
		for(int i = 0x0; i <= 0x3; ++i) {
			instructions[(i << 4) + 0x3] = [&registers, &memory, i]() {
				PREINC(REG16(i));
				return OK;
			};

		}
		// DEC RR
		for(int i = 0x0; i <= 0x3; ++i) {
			instructions[(i << 4) + 0xB] = [&registers, &memory, i]() {
				PREDEC(REG16(i));
				return OK;
			};

		}
		// ADD HL,RR
		for(int i = 0x0; i <= 0x3; ++i) {
			instructions[(i << 4) + 0x9] = [&registers, &memory, i]() {
				SET_N(false);
				SET_H(LOWER_NIBBLE(REG16(HL)) + LOWER_NIBBLE(REG16(i)) > 0xF);
				SET_C(REG16(HL) > 0xFFFF - REG16(i));
				REG16(HL) += REG16(i);
				return OK;
			};
		}
		// ADD SP,e
		instructions[0xE8] = [&registers, &memory]() {
			SET_Z(false);
			SET_N(false);
			int8_t val = SIGNED_IMM;
			if(val < 0 && LOWER_NIBBLE(val) > LOWER_NIBBLE(REG16(SP))) {
				SET_H(true);
			} else if(val > 0 && LOWER_NIBBLE(val) + LOWER_NIBBLE(REG16(SP))) {
				SET_H(true);
			} else {
				SET_H(false);
			}
			if(val < 0 && (-1 * val) > REG16(SP)) {
				SET_C(true);
			} else if(val > 0 && val > 0xFFFF - REG16(SP)) {
				SET_C(true);
			} else {
				SET_C(false);
			}
			REG16(SP) += val;
			return OK;
		};

		// Rotates, shifts, and bit operations
		// RLCA
		instructions[0x07] = [&registers, &memory]() {
			SET_C(REG8(A) & (1 << 7));
			REG8(A) <<= 1;
			REG8(A) |= GET_C();
			return OK;
		};
		// RLA
		instructions[0x17] = [&registers, &memory]() {
			int c = GET_C();
			SET_C(REG8(A) & (1 << 7));
			REG8(A) <<= 1;
			REG8(A) |= c;
			return OK;
		};
		// RRCA
		instructions[0x0F] = [&registers, &memory]() {
			SET_C(REG8(A) & 1);
			REG8(A) >>= 1;
			REG8(A) |= (GET_C() << 7);
			return OK;
		};
		// RRA
		instructions[0x1F] = [&registers, &memory]() {
			int c = GET_C();
			SET_C(REG8(A) & 1);
			REG8(A) >>= 1;
			REG8(A) |= (c << 7);
			return OK;
		};
		// CB
		instructions[0xCB] = [this, &registers, &memory]() {
			uint8_t cb_op = N;
			return this->cbInstructions[cb_op]();
		};
		
		// Control Flow
		// JP nn
		instructions[0xC3] = [&registers, &memory]() {
			REG16(PC) = NN;
			return OK;
		};
		// JP HL
		instructions[0xE9] = [&registers]() {
			REG16(PC) = REG16(HL);
			return OK;
		};
		// JP NZ
		instructions[0xC2] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(!GET_Z()) {
				REG16(PC) = nn;
			}
			return OK;
		};
		// JP NC
		instructions[0xD2] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(!GET_C()) {
				REG16(PC) = nn;
			}
			return OK;
		};
		// JP Z
		instructions[0xCA] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(GET_Z()) {
				REG16(PC) = nn;
			}
			return OK;
		};
		// JP C 
		instructions[0xDA] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(GET_C()) {
				REG16(PC) = nn;
			}
			return OK;
		};
		// JR e
		instructions[0x18] = [&registers, &memory]() {
			int8_t e = SIGNED_IMM;
			REG16(PC) += e;
			return OK;
		};
		// JR NZ,e
		instructions[0x20] = [&registers, &memory]() {
			uint8_t e = SIGNED_IMM;
			if(!GET_Z()) {
				REG16(PC) = e;
			}
			return OK;
		};
		// JR NC,e
		instructions[0x30] = [&registers, &memory]() {
			uint16_t e = SIGNED_IMM;
			if(!GET_C()) {
				REG16(PC) = e;
			}
			return OK;
		};
		// JR Z,e
		instructions[0x28] = [&registers, &memory]() {
			uint16_t e = SIGNED_IMM;
			if(GET_Z()) {
				REG16(PC) = e;
			}
			return OK;
		};
		// JR C,e
		instructions[0x38] = [&registers, &memory]() {
			uint16_t e = SIGNED_IMM;
			if(GET_C()) {
				REG16(PC) = e;
			}
			return OK;
		};
		// CALL nn
		instructions[0xCD] = [&registers, &memory]() {
			uint16_t nn = NN;
			CALL(nn);
			return OK;
		};
		// CALL NZ,nn
		instructions[0xC4] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(!GET_Z()) {
				CALL(nn);
			}
			return OK;
		};
		// CALL NC,nn
		instructions[0xD4] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(!GET_C()) {
				CALL(nn);
			}
			return OK;
		};
		// CALL Z,nn
		instructions[0xCC] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(GET_Z()) {
				CALL(nn);
			}
			return OK;
		};
		// CALL C,nn
		instructions[0xDC] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(GET_C()) {
				CALL(nn);
			}
			return OK;
		};
		// RET
		instructions[0xC9] = [&registers, &memory]() {
			RET();
			return OK;
		};
		// RET NZ
		instructions[0xC0] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(!GET_Z()) {
				RET();
			}
			return OK;
		};
		// RET NC
		instructions[0xD0] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(!GET_C()) {
				RET();
			}
			return OK;
		};
		// RET Z
		instructions[0xC8] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(GET_Z()) {
				RET();
			}
			return OK;
		};
		// RET C
		instructions[0xD8] = [&registers, &memory]() {
			uint16_t nn = NN;
			if(GET_C()) {
				RET();
			}
			return OK;
		};
		// RETI
		instructions[0xD9] = [&registers, &memory]() {
			RET();
			registers->registers16.IME = true;
			return OK;
		};
		// RST n
		for(int i = 0x0; i <= 0x3; i++) {
			instructions[0xC7 + (i << 4)] = [&registers, &memory, i]() {
				CALL(0x10 * i);
				return OK;
			};
			instructions[0xCF + (i << 4)] = [&registers, &memory, i]() {
				CALL(0x08 + 0x10 * i);
				return OK;
			};
		}
		// EI
		instructions[0xFB] = [&registers]() {
			registers->registers16.IME = true;
			return OK;
		}
		// DI
		instructions[0xF3] = [&registers]() {
			registers->registers16.IME = false;
			return OK;
		}
	}

	// Returns a status code.
	InstructionStatus InstructionSet::exec(uint8_t opcode) {
		return instructions[opcode]();
	}

}
