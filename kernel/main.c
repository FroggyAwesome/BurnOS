#include "init.h"

int
main(void)
{
	init();
	while (1) {
		__asm__ volatile("hlt");
	}
}
