/*
 * Geoclustering micro service
 * (c) Prince Cuberdon 2018
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "log.h"

#include <time.h>
#include <stdarg.h>

struct Logger
{
    int initialized;
    MessageType level;
    FILE *output;
    const char *format;
};

static const char *MessageStr[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR",
    "CRITICAL",
};

#define MAX_DATE_SIZE 50

static struct Logger _logger;

void log_init(FILE *file, MessageType level)
{
    _logger.output = file;
    _logger.format = "[%s] - %-8s - %s\n";
    _logger.initialized = 1;
    _logger.level = level;
}

static void now(char *date)
{
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(date, MAX_DATE_SIZE, "%x %X", timeinfo);
}

static void _log(MessageType type, const char *message, va_list args)
{
    if (!_logger.initialized)
    {
        _logger.output = stderr;
    }

    if (type >= _logger.level)
    {
        char buffer[2048];
        char date[MAX_DATE_SIZE];
        now(date);
        vsprintf(buffer, message, args);
        fprintf(_logger.output, _logger.format, date, MessageStr[type], buffer);
    }
}

void log_debug(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    _log(LOG_DEBUG, message, args);
    va_end(args);

}

void log_info(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    _log(LOG_INFO, message, args);
    va_end(args);

}

void log_warning(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    _log(LOG_WARNING, message, args);
    va_end(args);

}

void log_error(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    _log(LOG_ERROR, message, args);
    va_end(args);

}

void log_critical(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    _log(LOG_CRITICAL, message, args);
    va_end(args);

}

