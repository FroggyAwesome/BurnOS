#ifndef SYSTEM_H
#define SYSTEM_H

void poweroff(void) __attribute__((noreturn));
void reboot(void) __attribute__((noreturn));
void halt(void) __attribute__((noreturn));
void panic(const char *message) __attribute__((noreturn));

#endif
