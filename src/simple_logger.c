#include "simple_logger.h"
#include "simple_logger_define.h"
#include "simple_logger_info.h"
#include "simple_logger_output.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int status;
#if LOG_COLOR
    _Bool enable_color;
#endif
    int enable_level;
    // default stdout
    FILE* default_std_fp;
    _Bool disable_std_output;
#if LOG_FILE
    char log_path[LOG_FILENAME_MAX_LENGTH];
    _Bool enable_file_output;
#endif
    uint8_t formats[LOG_LEVEL_TOTAL_NUM + 1];
} slog_t;

static slog_t __slog_instance = { .status = 0,
#if LOG_COLOR
    .enable_color = 1,
#endif
    .enable_level = LOG_LEVEL,
    .default_std_fp = NULL,
    .disable_std_output = 0,
#if LOG_FILE
    .log_path = LOG_DEFAULT_PATH,
    .enable_file_output = 1,
#endif
    .formats = {
        [LOG_LEVEL_VERBOSE] = LOG_CONTENT_ALL,
        [LOG_LEVEL_DEBUG] = LOG_CONTENT_ALL,
        [LOG_LEVEL_INFO] = LOG_CONTENT_LEVEL|LOG_CONTENT_TAG|LOG_CONTENT_TIME,
        [LOG_LEVEL_WARNING] = LOG_CONTENT_ALL,
        [LOG_LEVEL_ERROR] = LOG_CONTENT_ALL
    }
};

static const char* content_format_table[] = {
    [0x00]
        = "",
    [LOG_CONTENT_LEVEL] 
        = "[%s] ",
    [LOG_CONTENT_TIME] 
        = "%s ",
    [LOG_CONTENT_CODE_INFO] 
        = "(%s:%ld@%s) ",
    [LOG_CONTENT_THREAD_INFO]
        = "(PID:%ld,TID:0x%lx) ",
    [LOG_CONTENT_CODE_INFO | LOG_CONTENT_THREAD_INFO] 
        = "(PID:%ld,TID:0x%lx %s:%ld@%s) ",
    [LOG_CONTENT_TAG] 
        = "[%s] ",
    [LOG_CONTENT_LEVEL | LOG_CONTENT_TAG]
        = "[%s/%s] "
};

#if LOG_COLOR
static const char* color_for_each_level[] = {
    [LOG_LEVEL_VERBOSE] = LOG_VERBOSE_COLOR,
    [LOG_LEVEL_DEBUG] = LOG_DEBUG_COLOR,
    [LOG_LEVEL_INFO] = LOG_INFO_COLOR,
    [LOG_LEVEL_WARNING] = LOG_WARNING_COLOR,
    [LOG_LEVEL_ERROR] = LOG_ERROR_COLOR
};

static const char* csi_end_str = CSI_END;
static int color_str_len[LOG_LEVEL_TOTAL_NUM];
static int csi_end_length;

#endif

static const char* log_level_str[] = { 
        [LOG_LEVEL_VERBOSE] = "V", 
        [LOG_LEVEL_DEBUG] = "D", 
        [LOG_LEVEL_INFO] = "I", 
        [LOG_LEVEL_WARNING] = "W",
        [LOG_LEVEL_ERROR] = "E" 
};

/* start logger */
int slog_start()
{
    if(__slog_instance.status==1){
        return LOG_ERROR;
    }
    if (__slog_instance.default_std_fp == NULL) {
        __slog_instance.default_std_fp = stdout;
    }
    if (__slog_instance.disable_std_output == 1) {
        set_std_fp(NULL);
    } else {
        set_std_fp(__slog_instance.default_std_fp);
    }
#if LOG_COLOR
    int i;
    for (i = 0; i < LOG_LEVEL_TOTAL_NUM;i++){
        color_str_len[i] = strlen(color_for_each_level[i]) + 1;
    }
    csi_end_length = strlen(csi_end_str) + 1;
#endif
    // calculate the time zone and set it
    set_time_zone();
#if LOG_FILE
    if(__slog_instance.enable_file_output){
        prepare_output(__slog_instance.log_path);
    }else{
        prepare_output(NULL);
    }
#else
    prepare_output(NULL);
#endif
    __slog_instance.status = 1;
    return LOG_STATUS_OK;
}

/* stop logger */
int slog_stop()
{
    finalize_output();
    __slog_instance.status = 0;
    return LOG_STATUS_OK;
}

/** set the path of log file */
void slog_set_file(char* path)
{
#if LOG_FILE
    if(path == NULL){
        __slog_instance.enable_file_output = 0;
        __slog_instance.log_path[0] = '\0';
    } else {
        strcpy(__slog_instance.log_path, path);
    }
#else
#   pragma message("Log File is Disabled! Set LOG_FILE to 1(in simple_logger_define.h) To Enable It!")
#endif
}

// set whether output to file
void slog_set_output_to_file(int enable)
{
#if LOG_FILE
    if (__slog_instance.enable_file_output == 0 \
        && __slog_instance.log_path[0] == '\0') {
        // disable file output by set path to NULL
        return;
    }
    __slog_instance.enable_file_output = (_Bool)enable;
#endif
}

/* set log format for each level */
void slog_set_content_for_each_level(uint8_t level, uint8_t content)
{
    if (level >= LOG_LEVEL_TOTAL_NUM) {
        return;
    }
    if (content > LOG_CONTENT_ALL) {
        return;
    }
    __slog_instance.formats[level] = content;
}


/* set color output enable/disable  */
void slog_set_color_output(int enable)
{
#if LOG_COLOR
    __slog_instance.enable_color = enable;
#else
#   pragma message("Color Output is Disabled! Set LOG_COLOR to 1(in simple_logger_define.h) To Enable It!")
#endif
}


/* set level filter */
void slog_set_level(uint8_t level)
{
    if (level >= LOG_LEVEL_TOTAL_NUM) {
        return;
    }
    __slog_instance.enable_level = level;
}

void slog_set_stdout_redirection(FILE* fp)
{
    if (fp == NULL) {
        __slog_instance.disable_std_output = 1;
        return;
    }
    __slog_instance.default_std_fp = fp;
}

void slog_output(uint8_t level, const char* tag,
    const char* file, const char* func, uint32_t line,
    const char* format, ...)
{
    int buf_remain = LOG_LINE_BUF_SIZE;
    int buf_use = 0;
#if DEBUG
    printf("current level %d tag %s file %s func %s \n", level, tag, file, func);
#endif
    if (!__slog_instance.status) {
        return;
    }
    // level filter
    if (level < __slog_instance.enable_level) {
        return;
    }

    // output_info_t* output;
    // output = (output_info_t*)malloc(sizeof(output_info_t));
    output_info_t output;
    output.log_control_start_offset = -1;
    output.log_info_offset = -1;
    output.log_control_end_offset = -1;
    output.buffer_size_to_copy = 4 * sizeof(int);

    char* log_buffer = output.buffer;
#if LOG_COLOR
    if (__slog_instance.enable_color) {
        buf_remain = buf_remain - csi_end_length;
        sprintf(log_buffer, "%s", color_for_each_level[level]);
        output.log_control_start_offset = 0;
        log_buffer += color_str_len[level];
        buf_remain = buf_remain - color_str_len[level];
    }
#endif
    output.log_info_offset = (int)(log_buffer - output.buffer);
    buf_use = 0;
    uint8_t part_tag_level = __slog_instance.formats[level] & (LOG_CONTENT_LEVEL | LOG_CONTENT_TAG);
    if (part_tag_level == ((LOG_CONTENT_LEVEL | LOG_CONTENT_TAG))){
        if (tag == NULL) {
            buf_use = snprintf(log_buffer, buf_remain, 
                               content_format_table[LOG_CONTENT_LEVEL], log_level_str[level]);
        } else {
            buf_use = snprintf(log_buffer, buf_remain,
                               content_format_table[LOG_CONTENT_LEVEL | LOG_CONTENT_TAG],
                               log_level_str[level], tag);
        }
    } else {
        if (part_tag_level == LOG_CONTENT_TAG && tag != NULL) {
            buf_use = snprintf(log_buffer, buf_remain,
                content_format_table[LOG_CONTENT_LEVEL], tag);
            
        }
        if(part_tag_level==LOG_CONTENT_LEVEL){
            buf_use = snprintf(log_buffer, buf_remain,
                content_format_table[LOG_CONTENT_LEVEL], log_level_str[level]);
        }
    }
    buf_remain = buf_remain - buf_use;
    log_buffer = log_buffer + buf_use;
    if (__slog_instance.formats[level] & LOG_CONTENT_TIME) {
        char current_time[LOG_TIME_STR_LENGTH];
        get_current_time_str(current_time);
        buf_use = snprintf(log_buffer, buf_remain,
            content_format_table[LOG_CONTENT_TIME], current_time);
        buf_remain = buf_remain - buf_use;
        log_buffer = log_buffer + buf_use;
    }
    uint8_t part_code_info = __slog_instance.formats[level] & \
                            (LOG_CONTENT_THREAD_INFO | LOG_CONTENT_CODE_INFO);
    buf_use = 0;
    switch (part_code_info) {
    case LOG_CONTENT_CODE_INFO:
        buf_use = snprintf(log_buffer, buf_remain,
            content_format_table[LOG_CONTENT_CODE_INFO],
            file, line, func);
        break;
    case LOG_CONTENT_THREAD_INFO:
        buf_use = snprintf(log_buffer, buf_remain,
            content_format_table[LOG_CONTENT_THREAD_INFO],
            get_pid(), get_tid());
        break;
    case LOG_CONTENT_THREAD_INFO | LOG_CONTENT_CODE_INFO:
        buf_use = snprintf(log_buffer, buf_remain,
            content_format_table[LOG_CONTENT_CODE_INFO | LOG_CONTENT_THREAD_INFO],
            get_pid(), get_tid(), file, line, func);
        break;
    default:
        break;
    }
    buf_remain = buf_remain - buf_use;
    log_buffer = log_buffer + buf_use;
    va_list args;
    va_start(args, format);
    buf_use = vsnprintf(log_buffer, buf_remain, format, args);
    buf_use += 1;
    va_end(args);
    // to the end of buffer
    buf_use = buf_use > buf_remain ? buf_remain : buf_use;
    buf_remain = buf_remain - buf_use;
    log_buffer = log_buffer + buf_use;
    *(log_buffer - 1) = '\0';
    // printf("buf_use:%d,remain %d,csi %d %d\n", buf_use, buf_remain, 
    //         color_str_len[level], csi_end_length);
#if LOG_COLOR
    if (__slog_instance.enable_color) {
        sprintf(log_buffer, "%s", csi_end_str);
        output.log_control_end_offset = (int)(log_buffer - output.buffer);
        // log_buffer += csi_end_length;
        // buf_remain = buf_remain - csi_end_length;
    }
#endif
    // align to 4, use bit operation to make it faster
    output.buffer_size_to_copy += (size_t)(3 + LOG_LINE_BUF_SIZE - buf_remain);
    // printf("copy bytes:%d\n", output.buffer_size_to_copy);
    output.buffer_size_to_copy = output.buffer_size_to_copy - (output.buffer_size_to_copy & 3);
    // printf("copy bytes:%d\n", output.buffer_size_to_copy);
    // fprintf(__slog_instance.default_std_fp, "%s%s%s, copy size %d\n",
    //     output.log_control_start, output.log_info,
    //     output.log_control_end, output.buffer_size_to_copy);
    output_log(&output);
    // free(output);
}
