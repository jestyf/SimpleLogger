#include "simple_logger_output.h"
#include "simple_logger_define.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static FILE* stdio_fp = NULL;
#if LOG_FILE
static FILE* file_fp = NULL;
static _Bool enable_file_output = 1;
#endif
static int current_written_lines = 0;
static int current_file_number = 0;
static char log_filename[LOG_FILENAME_MAX_LENGTH];

static int find_max_file_no(const char* filename);
static void rotate_filename(const char* filename, int num);
static void prepare_file(const char* filename);
static void finalize_file();
static inline void output_to_stdio(const output_info_t* output);
static inline void output_to_file(const output_info_t* output);

typedef enum {
    STATE_UNINITIALIZED = 0,
    STATE_READY,
    STATE_STOP
} state_t;

// pthread_mutex_t __fifo_push_mutex;
pthread_spinlock_t __fifo_push_spinlock;
pthread_t __output_thread;
state_t __current_state;
static void* output_thread_func(void* args);

typedef struct{
    output_info_t* items;
    int head;
    int tail;
} ring_fifo_t;

ring_fifo_t __fifo = {
    .items = NULL,
    .head = 0,
    .tail = 0
};

static void create_fifo();
static void destroy_fifo();
static inline int fifo_get_elem_num();
static inline _Bool fifo_is_full();
static inline _Bool fifo_is_empty();
static inline void fifo_push(const output_info_t* data);
// pop a element from fifo,
// first call try_pop(), if return value is not NULL,
// then call pop_finalize()
static inline output_info_t* fifo_try_pop();
static inline void fifo_pop_finalize();

static void create_fifo()
{
    // __fifo.contain = 0;
    __fifo.head = 0;
    __fifo.tail = 0;
    __fifo.items = (output_info_t*)malloc(LOG_FIFO_SIZE * sizeof(output_info_t));
}

static void destroy_fifo()
{
    __fifo.head = 0;
    __fifo.tail = 0;
    free(__fifo.items);
    __fifo.items = NULL;
}

static int fifo_get_elem_num()
{
    return (__fifo.tail + LOG_FIFO_SIZE - __fifo.head) & (LOG_FIFO_SIZE - 1);
}

static _Bool fifo_is_full()
{
    return (__fifo.tail + 1) & (LOG_FIFO_SIZE - 1) == __fifo.head;
}

static _Bool fifo_is_empty()
{
    return __fifo.head == __fifo.tail;
}

static void fifo_push(const output_info_t* data)
{
    if (fifo_is_full()) {
        // drop it when fifo is full
        return;
    }
    // memcpy(&__fifo.items[__fifo.tail], data, data->buffer_size_to_copy);
    memcpy(&__fifo.items[__fifo.tail], data, data->buffer_size_to_copy);
    // move queue tail to next position
    __fifo.tail = (__fifo.tail + 1) & (LOG_FIFO_SIZE - 1);
    
}

static output_info_t* fifo_try_pop()
{
    if(fifo_is_empty()){
        // nothing in the queue
        return NULL;
    }
    // the address of the element in the front
    return &(__fifo.items[__fifo.head]);
}

static void fifo_pop_finalize()
{
    __fifo.head = (__fifo.head + 1) & (LOG_FIFO_SIZE - 1);
}

static void rotate_filename(const char* filename, int num)
{
    int i;
    int len;
    char old_filename[256];
    char new_filename[256];
    char* old_filename_ptr;
    char* new_filename_ptr;
    len = snprintf(old_filename, 256, "%s", filename);
    len = snprintf(new_filename, 256, "%s", filename);
    old_filename_ptr = old_filename + len;
    new_filename_ptr = new_filename + len;
    for (i = num; i >= 0; i--) {
        // construct the old filename
        if (i != 0) {
            sprintf(old_filename_ptr, ".%d", i);
        } else {
            *old_filename_ptr = '\0';
        }
        sprintf(new_filename_ptr, ".%d", i + 1);
        rename(old_filename, new_filename);
    }
}

static int find_max_file_no(const char* filename)
{
    FILE* fp;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        // there is no log file at present
        return -1;
    }
    fclose(fp);

    char filename_[LOG_FILENAME_MAX_LENGTH];
    int i = 0;
    while (1) {
        i++;
        sprintf(filename_, "%s.%d", filename, i);
        fp = fopen(filename_, "r");
        if(fp==NULL){
            // file do not exist
            break;
        }
        fclose(fp);
    }
    return i - 1;
}

static void prepare_file(const char* filename)
{
#if LOG_FILE
    strcpy(log_filename, filename);
    int file_number;
    file_number = find_max_file_no(filename);
    if (file_number != -1) {
        // there are some log files
        rotate_filename(filename, file_number);
        current_file_number = file_number + 1;
    } else {
        current_file_number = 0;
    }
    current_written_lines = 0;
    file_fp = fopen(filename, "w");
#endif
}

static void finalize_file()
{
#if LOG_FILE
    if (file_fp != NULL) {
        fclose(file_fp);
    }
#endif
}

static void output_to_stdio(const output_info_t* output)
{
    if (stdio_fp == NULL) {
        return;
        // stdio_fp = stdout;
    }
    if (output->log_control_start_offset != -1 && output->log_control_end_offset != -1) {
        fprintf(stdio_fp, "%s", output->buffer + output->log_control_start_offset);
        fprintf(stdio_fp, "%s", output->buffer + output->log_info_offset);
        fprintf(stdio_fp, ("%s" LOG_NEWLINE_SIGN), output->buffer + output->log_control_end_offset);
    } else {
        fprintf(stdio_fp, ("%s" LOG_NEWLINE_SIGN), output->buffer + output->log_info_offset);
    }
}

static void output_to_file(const output_info_t* output)
{
#if LOG_FILE
    if(enable_file_output){
        if (current_written_lines >= LOG_FILE_MAX_LINE) {
            // rotate file, reopen the log file
            fclose(file_fp);
            rotate_filename(log_filename, current_file_number);
            file_fp = fopen(log_filename, "w");
            current_file_number++;
            current_written_lines = 0;
        }
        fprintf(file_fp, ("%s" LOG_NEWLINE_SIGN), output->buffer + output->log_info_offset);
        current_written_lines++;
    }
#endif
}

void set_std_fp(FILE* fp)
{
    stdio_fp = fp;
}

void prepare_output(const char* filename)
{
    if (filename == NULL) {
        enable_file_output = 0;
    }
    __current_state = STATE_UNINITIALIZED;
    // create fifo
    create_fifo();
#if LOG_FILE
    // prepare file
    if (enable_file_output) {
        prepare_file(filename);
    }
#endif
    // create mutex
    // pthread_mutex_init(&__fifo_push_mutex, NULL);
    pthread_spin_init(&__fifo_push_spinlock, 0);
    // start output thread
    pthread_create(&__output_thread, NULL, output_thread_func, NULL);
    // finish init
    __current_state = STATE_READY;
    return;
}

void finalize_output()
{
    // set current state to stop
    __current_state = STATE_STOP;
    // wait output thread stop
    pthread_join(__output_thread, NULL);
    // destroy mutex
    // pthread_mutex_destroy(&__fifo_push_mutex);
    pthread_spin_destroy(&__fifo_push_spinlock);
    // destroy fifo
    destroy_fifo();
#if LOG_FILE
    // close file fp
    finalize_file();
#endif
    return;
}

void output_log(const output_info_t* output)
{
    // lock mutex before push
    // pthread_mutex_lock(&__fifo_push_mutex);
    pthread_spin_lock(&__fifo_push_spinlock);
    // push output_data to fifo
    fifo_push(output);
    // unlock it when finish
    // pthread_mutex_unlock(&__fifo_push_mutex);
    pthread_spin_unlock(&__fifo_push_spinlock);
}

void* output_thread_func(void* args)
{
    // wait for ready
    while(__current_state==STATE_UNINITIALIZED){
        usleep(5);
    }
    // process those logs
    while (1) {
        if (__current_state == STATE_STOP) {
            break;
        }
        output_info_t* output;
        output = fifo_try_pop();
        if (output == NULL) {
            usleep(5);
        } else {
            output_to_stdio(output);
#if LOG_FILE
            output_to_file(output);
#endif
            fifo_pop_finalize();
        }
    }
    // process those remaing logs
    while(__current_state==STATE_STOP){
        output_info_t* output;
        output = fifo_try_pop();
        if (output == NULL) {
            break;
        } else {
            output_to_stdio(output);
#if LOG_FILE
            output_to_file(output);
#endif
            fifo_pop_finalize();
        }
    }
    return NULL;
}
