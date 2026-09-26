#include "timer.h"
#include "io.h"

static volatile unsigned int timer_ticks = 0;

void
init_pit(unsigned int frequency)
{
	unsigned int divisor = 1193180 / frequency;
	outb(0x43, 0x36);
	outb(0x40, (unsigned char)(divisor & 0xFF));
	outb(0x40, (unsigned char)((divisor >> 8) & 0xFF));
}

void
timer_handler(void)
{
	timer_ticks++;
}

unsigned int
get_ticks(void)
{
	return timer_ticks;
}

void
sleep(unsigned int seconds)
{
	sleep_ms(seconds * 1000);
}

void
sleep_ms(unsigned int milliseconds)
{
	unsigned int target_ticks = timer_ticks + milliseconds;
	while (timer_ticks < target_ticks) {
		__asm__ volatile("sti; hlt");
	}
}
