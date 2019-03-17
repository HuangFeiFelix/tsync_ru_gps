#include "log.h"
#include <stdarg.h>


LOG_DEBUG debug_level = LEVEL_STORE;
const int MY_LOGGER = 0;
static unsigned int log_index = 1;
static unsigned int log_line = 0;
struct clog *_clog_loggers[CLOG_MAX_LOGGERS] = { 0 };

const char *const CLOG_LEVEL_NAMES[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
};

void _clog_err(const char *fmt, ...)
{
#ifdef CLOG_SILENT
    (void) fmt;
#else
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
#endif
}


int clog_init_path(int id, const char *const path)
{
    int fd = open(path, O_CREAT | O_WRONLY | O_APPEND, 0666);
    if (fd == -1) {
        _clog_err("Unable to open %s: %s\n", path, strerror(errno));
        return 1;
    }
    if (clog_init_fd(id, fd)) {
        close(fd);
        return 1;
    }
    _clog_loggers[id]->opened = 1;
    return 0;
}

int clog_init_fd(int id, int fd)
{
    struct clog *logger;

    if (_clog_loggers[id] != NULL) {
        _clog_err("Logger %d already initialized.\n", id);
        return 1;
    }

    logger = (struct clog *) malloc(sizeof(struct clog));
    if (logger == NULL) {
        _clog_err("Failed to allocate logger: %s\n", strerror(errno));
        return 1;
    }

    logger->level = CLOG_DEBUG;
    logger->fd = fd;
    logger->opened = 0;
    strcpy(logger->fmt, CLOG_DEFAULT_FORMAT);
    strcpy(logger->date_fmt, CLOG_DEFAULT_DATE_FORMAT);
    strcpy(logger->time_fmt, CLOG_DEFAULT_TIME_FORMAT);

    _clog_loggers[id] = logger;
    return 0;
}

void clog_free(int id)
{
    if (_clog_loggers[id]) {
        if (_clog_loggers[id]->opened) {
            close(_clog_loggers[id]->fd);
        }
        free(_clog_loggers[id]);
    }
    _clog_loggers[0] = NULL;
}

int clog_set_level(int id, enum clog_level level)
{
    if (_clog_loggers[id] == NULL) {
        return 1;
    }
    if ((unsigned) level > CLOG_ERROR) {
        return 1;
    }
    _clog_loggers[id]->level = level;
    return 0;
}

int clog_set_time_fmt(int id, const char *fmt)
{
    struct clog *logger = _clog_loggers[id];
    if (logger == NULL) {
        _clog_err("clog_set_time_fmt: No such logger: %d\n", id);
        return 1;
    }
    if (strlen(fmt) >= CLOG_FORMAT_LENGTH) {
        _clog_err("clog_set_time_fmt: Format specifier too long.\n");
        return 1;
    }
    strcpy(logger->time_fmt, fmt);
    return 0;
}

int clog_set_date_fmt(int id, const char *fmt)
{
    struct clog *logger = _clog_loggers[id];
    if (logger == NULL) {
        _clog_err("clog_set_date_fmt: No such logger: %d\n", id);
        return 1;
    }
    if (strlen(fmt) >= CLOG_FORMAT_LENGTH) {
        _clog_err("clog_set_date_fmt: Format specifier too long.\n");
        return 1;
    }
    strcpy(logger->date_fmt, fmt);
    return 0;
}

int clog_set_fmt(int id, const char *fmt)
{
    struct clog *logger = _clog_loggers[id];
    if (logger == NULL) {
        _clog_err("clog_set_fmt: No such logger: %d\n", id);
        return 1;
    }
    if (strlen(fmt) >= CLOG_FORMAT_LENGTH) {
        _clog_err("clog_set_fmt: Format specifier too long.\n");
        return 1;
    }
    strcpy(logger->fmt, fmt);
    return 0;
}

/* Internal functions */

int _clog_append_str(char **dst, char *orig_buf, const char *src, size_t cur_size)
{
    int new_size = cur_size;

    while (strlen(*dst) + strlen(src) >= new_size) {
        new_size *= 2;
    }
    if (new_size != cur_size) {
        if (*dst == orig_buf) {
            *dst = (char *) malloc(new_size);
            strcpy(*dst, orig_buf);
        } else {
            *dst = (char *) realloc(*dst, new_size);
        }
    }

    strcat(*dst, src);
    return new_size;
}

int _clog_append_int(char **dst, char *orig_buf, long int d, size_t cur_size)
{
    char buf[40]; /* Enough for 128-bit decimal */
    if (snprintf(buf, 40, "%ld", d) >= 40) {
        return cur_size;
    }
    return _clog_append_str(dst, orig_buf, buf, cur_size);
}

int _clog_append_time(char **dst, char *orig_buf, struct tm *lt,
                  const char *fmt, size_t cur_size)
{
    char buf[CLOG_DATETIME_LENGTH];
    int result = strftime(buf, CLOG_DATETIME_LENGTH, fmt, lt);

    if (result > 0) {
        return _clog_append_str(dst, orig_buf, buf, cur_size);
    }

    return cur_size;
}

const char * _clog_basename(const char *path)
{
    const char *slash = strrchr(path, '/');
    if (slash) {
        path = slash + 1;
    }

    return path;
}

char * _clog_format(const struct clog *logger, char buf[], size_t buf_size,
             const char *sfile, int sline, const char *level,
             const char *message)
{
    int cur_size = buf_size;
    char *result = buf;
    enum { NORMAL, SUBST } state = NORMAL;
    int fmtlen = strlen(logger->fmt);
    int i;
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);

    sfile = _clog_basename(sfile);
    result[0] = 0;
    for (i = 0; i < fmtlen; ++i) {
        if (state == NORMAL) {
            if (logger->fmt[i] == '%') {
                state = SUBST;
            } else {
                char str[2] = { 0 };
                str[0] = logger->fmt[i];
                cur_size = _clog_append_str(&result, buf, str, cur_size);
            }
        } else {
            switch (logger->fmt[i]) {
                case '%':
                    cur_size = _clog_append_str(&result, buf, "%", cur_size);
                    break;
                case 't':
                    cur_size = _clog_append_time(&result, buf, lt,
                                                 logger->time_fmt, cur_size);
                    break;
                case 'd':
                    cur_size = _clog_append_time(&result, buf, lt,
                                                 logger->date_fmt, cur_size);
                    break;
                case 'l':
                    cur_size = _clog_append_str(&result, buf, level, cur_size);
                    break;
                case 'n':
                    cur_size = _clog_append_int(&result, buf, sline, cur_size);
                    break;
                case 'f':
                    cur_size = _clog_append_str(&result, buf, sfile, cur_size);
                    break;
                case 'm':
                    cur_size = _clog_append_str(&result, buf, message,
                                                cur_size);
                    break;
            }
            state = NORMAL;
        }
    }

    return result;
}

void _clog_log(const char *sfile, int sline, enum clog_level level,
          int id, const char *fmt, va_list ap)
{
    /* For speed: Use a stack buffer until message exceeds 4096, then switch
     * to dynamically allocated.  This should greatly reduce the number of
     * memory allocations (and subsequent fragmentation). */
    char buf[4096];
    size_t buf_size = 4096;
    char *dynbuf = buf;
    char *message;
    va_list ap_copy;
    int result;
    struct clog *logger = _clog_loggers[id];

    if (!logger) {
        _clog_err("No such logger: %d\n", id);
        return;
    }

    if (level < logger->level) {
        return;
    }
    
    log_line++;

    /* Format the message text with the argument list. */
    va_copy(ap_copy, ap);
    result = vsnprintf(dynbuf, buf_size, fmt, ap);
    if ((size_t) result >= buf_size) {
        buf_size = result + 1;
        dynbuf = (char *) malloc(buf_size);
        result = vsnprintf(dynbuf, buf_size, fmt, ap_copy);
        if ((size_t) result >= buf_size) {
            /* Formatting failed -- too large */
            _clog_err("Formatting failed (1).\n");
            va_end(ap_copy);
            free(dynbuf);
            return;
        }
    }
    va_end(ap_copy);

    /* Format according to log format and write to log */
    {
        char message_buf[4096];
        message = _clog_format(logger, message_buf, 4096, sfile, sline,
                               CLOG_LEVEL_NAMES[level], dynbuf);
        if (!message) {
            _clog_err("Formatting failed (2).\n");
            if (dynbuf != buf) {
                free(dynbuf);
            }
            return;
        }
        result = write(logger->fd, message, strlen(message));
        if (result == -1) {
            _clog_err("Unable to write to log file: %s\n", strerror(errno));
        }
        if (message != message_buf) {
            free(message);
        }
        if (dynbuf != buf) {
            free(dynbuf);
        }
    }
}



int syslog_init()
{
    int ret = -1;
    char fname[100];
    time_t t = time(NULL);
    struct tm *it = localtime(&t);
    memset(fname,0,sizeof(fname));
    
    sprintf(fname,"/home/TimeSync_log_%d-%d-%d_%04d.log",it->tm_year+1900,it->tm_mon+1,it->tm_mday,log_index);
    
    ret = clog_init_path(MY_LOGGER, fname);
    
    return ret;
}

void syslog(enum clog_level level,const char *fmt,...)
{
    va_list ap;
    va_start(ap, fmt);

    if(log_line >= CLOG_LINE_COUNT)
    {
        clog_free(MY_LOGGER);
        log_line = 0;
        log_index += 1;
        syslog_init();
    }

    switch(level)
    {
        case CLOG_DEBUG:
            _clog_log(__FILE__, __LINE__, CLOG_DEBUG, MY_LOGGER, fmt, ap);
            break;
        case CLOG_INFO:
            
            _clog_log(__FILE__, __LINE__, CLOG_INFO, MY_LOGGER, fmt, ap);
            break;
        case CLOG_WARN:
            
            _clog_log(__FILE__, __LINE__, CLOG_WARN, MY_LOGGER, fmt, ap);
            break;
        case CLOG_ERROR:
            _clog_log(__FILE__, __LINE__, CLOG_ERROR, MY_LOGGER, fmt, ap);
            break;

        default:
            break;
    }
    va_end(ap);

}

