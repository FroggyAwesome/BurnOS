#ifndef TIMER_H
#define TIMER_H

void init_pit(unsigned int frequency);
void timer_handler(void);
unsigned int get_ticks(void);
void sleep(unsigned int seconds);
void sleep_ms(unsigned int milliseconds);

#endif
