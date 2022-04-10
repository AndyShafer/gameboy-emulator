#pragma once

#include "cpu-registers.h"
#include "register-map.h"
#include "memory-map.h"
#include "instruction-set.h"

namespace gbemulator {

class Cpu {
public:
	Cpu();
	void run();
	void pause() {};
	void resume() {};
private:
	CpuRegisters registers;
	MemoryMap *memory;
	InstructionSet *instructions;
};

}
