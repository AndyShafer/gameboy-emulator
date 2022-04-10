#pragma once

#include <cstdint>

namespace gbemulator {

class RegisterMap {
public:
	RegisterMap(uint8_t *addr) : addr(addr) {}

private:
	uint8_t *addr;
};

}
