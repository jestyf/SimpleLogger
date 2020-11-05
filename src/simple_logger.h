#ifndef _s_logger_h
#define _s_logger_h

#ifndef __linux
#error platform error
#pragma message("This Library Only Support Linux System!")
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "simple_logger_define.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int slog_start();
int slog_stop();
void slog_set_file(char* path);
void slog_set_output_to_file(int enable);
void slog_set_content_for_each_level(uint8_t level, uint8_t content);
void slog_set_color_output(int enable);
void slog_set_level(uint8_t level);
void slog_set_stdout_redirection(FILE* fp);

void slog_output(uint8_t level, const char* tag,
    const char* file, const char* func, uint32_t line,
    const char* format, ...);

//tag 
#ifndef LOG_TAG
#   define LOG_TAG (NULL)
#endif

// default log level
#ifndef LOG_LEVEL
#   define LOG_LEVEL LOG_LEVEL_INFO
#endif

#if LOG_LEVEL <= LOG_LEVEL_VERBOSE
#   define slog_v(...)                                         \
            slog_output(LOG_LEVEL_VERBOSE, LOG_TAG, __FILE__,  \
                        __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#   define slog_v ((void)0);
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#   define slog_d(...)                                         \
            slog_output(LOG_LEVEL_DEBUG, LOG_TAG, __FILE__,    \
                        __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#   define slog_d ((void)0);
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#   define slog_i(...)                                         \
            slog_output(LOG_LEVEL_INFO, LOG_TAG, __FILE__,     \
                        __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#   define slog_i ((void)0);
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARNING
#   define slog_w(...)                                         \
            slog_output(LOG_LEVEL_WARNING, LOG_TAG, __FILE__,  \
                        __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#   define slog_w ((void)0);
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#   define slog_e(...)                                         \
            slog_output(LOG_LEVEL_ERROR, LOG_TAG, __FILE__,    \
                        __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#   define slog_e ((void)0);
#endif

#ifdef __cplusplus
}
#endif

#endif
