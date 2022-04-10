#pragma once

#include <stdexcept>

namespace gbemulator {

enum Register8Id {
	B = 0,
	C = 1,
	D = 2,
	E = 3,
	H = 4,
	L = 5,
	A = 7
};

enum Register16Id {
	BC = 0,
	DE = 1,
	HL = 2,
	SP = 3
};

struct Registers8BitView {
	uint8_t B;
	uint8_t C;
	uint8_t D;
	uint8_t E;
	uint8_t H;
	uint8_t L;
	uint8_t A;
};

struct Registers16BitView {
	uint16_t BC;
	uint16_t DE;
	uint16_t HL;
	uint8_t   A;
	uint16_t PC;
	uint16_t SP;
};

union CpuRegisters {
	Registers8BitView registers8;
	Registers16BitView registers16;
	uint8_t& get8BitReg(Register8Id id) {
		switch(id) {
			case B: return registers8.B;
			case C: return registers8.C;
			case D: return registers8.D;
			case E: return registers8.E;
			case H: return registers8.H;
			case L: return registers8.L;
			case A: return registers8.A;
		}
		throw new std::invalid_argument("Could not map ID to register");
	}
	uint16_t& get16BitReg(Register16Id id) {
		switch(id) {
			case BC: return registers16.BC;
			case DE: return registers16.DE;
			case HL: return registers16.HL;
			case SP: return registers16.SP;
		}
		throw new std::invalid_argument("Could not map ID to register");
	}
};

}
