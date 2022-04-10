#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <cpu-registers.h>
#include <instruction-set.h>
#include <memory-map.h>

using namespace gbemulator;

TEST_CASE("NOP", "[InstructionSet]") {
	CpuRegisters registers;
	MemoryMap *memory = new MemoryMap();
	InstructionSet instructions(&registers, memory);
	CpuRegisters registerCpy = registers;
	instructions.exec(0x00);
	REQUIRE(registers.registers16.PC == registerCpy.registers16.PC);
}
