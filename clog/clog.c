// Copyright 2023 Vahid Mardani
/*
 * This file is part of clog.
 *  clog is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  clog is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with clog. If not, see <https://www.gnu.org/licenses/>.
 *
 *  Author: Vahid Mardani <vahid.mardani@gmail.com>
 */
#include "clog.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>


enum clog_verbosity clog_verbosity = CLOG_DEBUG;


const char * clog_verbosities[] = {
    [CLOG_SILENT]  = "never",  // 0
    [CLOG_FATAL]   = "fatal",  // 1
    [CLOG_ERROR]   = "error",  // 2
    [CLOG_WARNING] = "warn",   // 3
    [CLOG_INFO]    = "info",   // 4
    [CLOG_DEBUG]   = "debug",  // 5
};


void
clog_log(
        enum clog_verbosity level,
        const char *filename,
        int lineno,
        const char *function,
        bool newline,
        const char *format,
        ...) {
    va_list args;

    if (format) {
        va_start(args, format);
    }

    clog_vlog(level, filename, lineno, function, newline, format, args);

    if (format) {
        va_end(args);
    }
}


void
clog_hless(enum clog_verbosity level, bool newline,
        const char *format, ...) {
    va_list args;
    int fd = (level <= CLOG_ERROR)? STDERR_FILENO: STDOUT_FILENO;

    if (level > clog_verbosity) {
        return;
    }

    if (format) {
        va_start(args, format);
    }

    vdprintf(fd, format, args);

    if (newline) {
        dprintf(fd, NEWLINE);
    }

    if (format) {
        va_end(args);
    }

    if (level == CLOG_FATAL) {
        exit(EXIT_FAILURE);
    }
}


void
clog_vlog(
        enum clog_verbosity level,
        const char *filename,
        int lineno,
        const char *function,
        bool newline,
        const char *format,
        va_list args) {
    int fd = (level <= CLOG_ERROR)? STDERR_FILENO: STDOUT_FILENO;

    if (level > clog_verbosity) {
        return;
    }

    dprintf(fd, "[%-5s]", clog_verbosities[level]);
    if (clog_verbosity == CLOG_DEBUG) {
        dprintf(fd, " [%s:%d %s]", filename, lineno, function);
    }

    if (format) {
        dprintf(fd, " ");
        vdprintf(fd, format, args);
    }

    if (newline) {
        dprintf(fd, NEWLINE);
    }

    if (level == CLOG_FATAL) {
        exit(EXIT_FAILURE);
    }
}
