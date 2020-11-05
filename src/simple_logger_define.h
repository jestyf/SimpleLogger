#ifndef _simple_logger_def_h
#define _simple_logger_def_h

//log level
#define LOG_LEVEL_VERBOSE     (0)
#define LOG_LEVEL_DEBUG       (1)
#define LOG_LEVEL_INFO        (2)
#define LOG_LEVEL_WARNING     (3)
#define LOG_LEVEL_ERROR       (4)

// total number of log level
#define LOG_LEVEL_TOTAL_NUM   (5)

//log content
#define LOG_CONTENT_LEVEL       (0x1 << 0)
#define LOG_CONTENT_TIME        (0x1 << 1)
#define LOG_CONTENT_CODE_INFO   (0x1 << 2)
#define LOG_CONTENT_THREAD_INFO (0x1 << 3)
#define LOG_CONTENT_TAG         (0x1 << 4)
#define LOG_CONTENT_ALL         (LOG_CONTENT_LEVEL | LOG_CONTENT_TIME | \
                                LOG_CONTENT_CODE_INFO | LOG_CONTENT_THREAD_INFO | \
                                LOG_CONTENT_TAG ) 

#define LF                      "\n"
#define CR                      "\r"
#define CRLF                    (CR LF)
#define LOG_STATUS_OK           (0)
#define LOG_ERROR               (-1)

/**
 * CSI(Control Sequence Introducer/Initiator) sign
 * more information on https://en.wikipedia.org/wiki/ANSI_escape_code
 */
#define CSI_START          "\033["
#define CSI_END            "\033[0m"
/* output log front color */
#define FONT_BLACK         "30;"
#define FONT_RED           "31;"
#define FONT_GREEN         "32;"
#define FONT_YELLOW        "33;"
#define FONT_BLUE          "34;"
#define FONT_MAGENTA       "35;"
#define FONT_CYAN          "36;"
#define FONT_WHITE         "37;"
#define FONT_DEFAULT
/* output log background color */
#define BACKGROUND_DEFAULT
#define BACKGROUND_BLACK   "40;"
#define BACKGROUND_RED     "41;"
#define BACKGROUND_GREEN   "42;"
#define BACKGROUND_YELLOW  "43;"
#define BACKGROUND_BLUE    "44;"
#define BACKGROUND_MAGENTA "45;"
#define BACKGROUND_CYAN    "46;"
#define BACKGROUND_WHITE   "47;"
/* output log fonts style */
#define STYLE_BOLD         "1m"
#define STYLE_UNDERLINE    "4m"
#define STYLE_BLINK        "5m"
#define STYLE_NORMAL       "22m"


#define LOG_COLOR               1
#define LOG_FILE                1 
// end of line
#define LOG_NEWLINE_SIGN        LF
// buffer size of each log line
#define LOG_LINE_BUF_SIZE       (512)
#define LOG_TIME_STR_LENGTH     (32)
// maxium length of the the log file path
#define LOG_FILENAME_MAX_LENGTH (256)
// default file of the log
#define LOG_DEFAULT_PATH        "/tmp/slog.log"
// the maxium number of lines of a log file
#define LOG_FILE_MAX_LINE       (10000)
// use a fifo to print the log, fifo size must be 2^n
// modify this number may cause some fault
#define LOG_FIFO_SIZE           (4096)    

#if LOG_COLOR
// set font color
#   define LOG_VERBOSE_COLOR (CSI_START FONT_BLUE   BACKGROUND_DEFAULT STYLE_NORMAL)
#   define LOG_DEBUG_COLOR   (CSI_START FONT_CYAN   BACKGROUND_DEFAULT STYLE_NORMAL)
#   define LOG_INFO_COLOR    (CSI_START FONT_GREEN  BACKGROUND_DEFAULT STYLE_NORMAL)
#   define LOG_WARNING_COLOR (CSI_START FONT_YELLOW BACKGROUND_DEFAULT STYLE_NORMAL)
#   define LOG_ERROR_COLOR   (CSI_START FONT_RED    BACKGROUND_DEFAULT STYLE_BOLD) 
#endif

#endif

