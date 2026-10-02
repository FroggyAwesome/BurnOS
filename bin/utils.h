#ifndef UTILS_H
#define UTILS_H

int strprefix(const char *pre, const char *str);
void cmd_help(void);
void cmd_time(void);
void cmd_date(void);
void cmd_day(void);
void cmd_sleep(const char *args);
void cmd_uptime(void);
void cmd_colors(void);
void cmd_set(const char *args);
void cmd_dir(void);
void cmd_cat(const char *args);
void cmd_write(const char *args);
void cmd_rm(const char *args);
void cmd_echo(const char *args);
void cmd_whoami(void);
void cmd_hostname(void);
void cmd_arch(void);
void cmd_version(void);
void cmd_calc(const char *args);

#endif
