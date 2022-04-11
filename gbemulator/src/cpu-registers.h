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
	SP = 3,
	AF = 4,
	PC = 5
};

enum Flag {
	FLAG_Z = 7, // Zero
	FLAG_N = 6, // Subtraction (BCD)
	FLAG_H = 5, // Half Carry (BCD)
	FLAG_C = 4  // Carry
};

struct Registers8BitView {
	uint8_t B;
	uint8_t C;
	uint8_t D;
	uint8_t E;
	uint8_t H;
	uint8_t L;
	uint8_t A;
	uint8_t F;
};

struct Registers16BitView {
	uint16_t BC;
	uint16_t DE;
	uint16_t HL;
	uint16_t AF;
	uint16_t PC;
	uint16_t SP;
	bool     IME;
};

union CpuRegisters {
	Registers8BitView registers8;
	Registers16BitView registers16;
	uint8_t& get8BitReg(int id) {
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
	uint16_t& get16BitReg(int id) {
		switch(id) {
			case BC: return registers16.BC;
			case DE: return registers16.DE;
			case HL: return registers16.HL;
			case SP: return registers16.SP;
			case AF: return registers16.AF;
			case PC: return registers16.PC;
		}
		throw new std::invalid_argument("Could not map ID to register");
	}
	bool getFlag(Flag f) {
		return registers8.F & (1 << f);
	}
	void setFlag(Flag f, bool value) {
		if(value) {
			registers8.F |= (1 << f);
		} else {
			registers8.F &= ~(1 << f);
		}
	}
	void setFlag(Flag f, char op) {
		switch(op) {
			case '0':
				this->setFlag(f, false);
				return;
			case '1':
				this->setFlag(f, true);
				return;
			case 'x':
				registers8.F ^= (1 << f);
				return;
			case '-':
				return;
		}
	}
	void setFlags(char z, char n, char h, char c) {
		this->setFlag(FLAG_Z, z);
		this->setFlag(FLAG_N, n);
		this->setFlag(FLAG_H, h);
		this->setFlag(FLAG_C, c);
	}
	void setFlags(std::string flags) {
		this->setFlag(FLAG_Z, flags[0]);
		this->setFlag(FLAG_N, flags[1]);
		this->setFlag(FLAG_H, flags[2]);
		this->setFlag(FLAG_C, flags[3]);
	}
};

}
