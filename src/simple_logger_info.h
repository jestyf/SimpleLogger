#ifndef _s_logger_info_h
#define _s_logger_info_h
#include "simple_logger_define.h"
// get info
void set_time_zone();
void get_current_time_str(char* time_str);
int get_pid();
unsigned long get_tid();

#endif