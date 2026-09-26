#include "rtc.h"
#include "io.h"

static unsigned char
get_rtc_register(unsigned char reg)
{
	outb(0x70, reg);
	return inb(0x71);
}

static int
is_updating(void)
{
	outb(0x70, 0x0A);
	return (inb(0x71) & 0x80);
}

void
read_rtc_time(int *hours, int *minutes, int *seconds)
{
	while (is_updating()) {
		__asm__ volatile("pause");
	}

	unsigned char raw_s = get_rtc_register(0x00);
	unsigned char raw_m = get_rtc_register(0x02);
	unsigned char raw_h = get_rtc_register(0x04);
	unsigned char reg_b = get_rtc_register(0x0B);

	if (!(reg_b & 0x04)) {
		*seconds = (raw_s & 0x0F) + ((raw_s >> 4) * 10);
		*minutes = (raw_m & 0x0F) + ((raw_m >> 4) * 10);
		*hours = (raw_h & 0x0F) + (((raw_h & 0x70) >> 4) * 10);
	} else {
		*seconds = raw_s;
		*minutes = raw_m;
		*hours = raw_h;
	}
}

void
read_rtc_date(int *day, int *month, int *year)
{
	while (is_updating()) {
		__asm__ volatile("pause");
	}

	unsigned char raw_d = get_rtc_register(0x07);
	unsigned char raw_mo = get_rtc_register(0x08);
	unsigned char raw_y = get_rtc_register(0x09);
	unsigned char reg_b = get_rtc_register(0x0B);

	if (!(reg_b & 0x04)) {
		*day = (raw_d & 0x0F) + ((raw_d >> 4) * 10);
		*month = (raw_mo & 0x0F) + ((raw_mo >> 4) * 10);
		*year = 2000 + (raw_y & 0x0F) + ((raw_y >> 4) * 10);
	} else {
		*day = raw_d;
		*month = raw_mo;
		*year = 2000 + raw_y;
	}
}
