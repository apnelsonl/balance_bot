#include "delay.h"

void delay_usec(int t) {
	int i;
	for(i = 0; i < t; ++i)
		__delay_cycles(us);
}
