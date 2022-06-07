#include "mathUtil.h"

unsigned int divCeil(unsigned int a, unsigned int b) {
	return (a + b - 1) / b;
}