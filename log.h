#ifndef _LOGER_H
#define _LOGER_H
 
#include "common.h"


/* Number of loggers that can be defined. */
#define CLOG_MAX_LOGGERS 16

/* Format strings cannot be longer than this. */
#define CLOG_FORMAT_LENGTH 256

/* Formatted times and dates should be less than this length. If they are not,
 * they will not appear in the log. */
#define CLOG_DATETIME_LENGTH 256

#define CLOG_LINE_COUNT 2000


/* Default format strings. */
#define CLOG_DEFAULT_FORMAT "%d %t %f(%n): %l: %m\n"
#define CLOG_DEFAULT_DATE_FORMAT "%Y-%m-%d"
#define CLOG_DEFAULT_TIME_FORMAT "%H:%M:%S"

#define SYSLOG_FILE_FORMAT "TimeSync_log_%Y-%m-%d_%02d"

#define CLOG(id) __FILE__, __LINE__, id
extern LOG_DEBUG debug_level;


enum clog_level {
    CLOG_DEBUG,
    CLOG_INFO,
    CLOG_WARN,
    CLOG_ERROR
};


struct clog {

    /* The current level of this logger. Messages below it will be dropped. */
    enum clog_level level;

    /* The file being written. */
    int fd;

    /* The format specifier. */
    char fmt[CLOG_FORMAT_LENGTH];

    /* Date format */
    char date_fmt[CLOG_FORMAT_LENGTH];

    /* Time format */
    char time_fmt[CLOG_FORMAT_LENGTH];

    /* Tracks whether the fd needs to be closed eventually. */
    int opened;
};


int clog_init_path(int id, const char *const path);


int clog_init_fd(int id, int fd);


void clog_free(int id);

int clog_set_level(int id, enum clog_level level);

int clog_set_time_fmt(int id, const char *fmt);


int clog_set_date_fmt(int id, const char *fmt);


int clog_set_fmt(int id, const char *fmt);

int syslog_init();

void syslog(enum clog_level level,const char *fmt,...);

#endif
