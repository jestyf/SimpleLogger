#ifndef _s_logger_output_h
#define _s_logger_output_h

#include <stdio.h>
#include <stdlib.h>
#include "simple_logger_define.h"

/**
    memory layout of a output buffer
    buffer:
    |-----CSI start-----|----log----|--CSI END--|
     ^                   ^           ^
     log_control_offset  log_offset  log_control_offset
    each string ends with '\0'

    memory layout of this struct (64 bit system)
    |copy size|start offset|info offset|end offset |     log buffer          |
    |-4 bytes-|-- 4 bytes--|--4 bytes--|--4 bytes--|-LOG_LINE_BUF_SIZE bytes-|
    |<---------------buffer_size_to_copy----------------->|
    when copy this struct to another memory block,
    only copy buffer_size_to_copy bytes, which is align to 4
*/
typedef struct{
    int buffer_size_to_copy;         // occupy 4 bytes on a 64 bits linux system
    int log_control_start_offset;    // occupy 4 bytes on a 64 bits linux system
    int log_info_offset;             // occupy 4 bytes on a 64 bits linux system
    int log_control_end_offset;      // occupy 4 bytes on a 64 bits linux system
    char buffer[LOG_LINE_BUF_SIZE];
} output_info_t;

void set_std_fp(FILE* fp);
void prepare_output(const char* filename);
void finalize_output();
void output_log(const output_info_t* output);

#endif