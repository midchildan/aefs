/* logging.h -- Logging support code.
   Copyright (C) 2000, 2001 Eelco Dolstra (eelco@cs.uu.nl).

   $Id: logging.c,v 1.1 2001/12/24 19:29:08 eelco Exp $

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <stdio.h>
#include <stdarg.h>

#include "logging.h"

bool fDebug = false;

/* Write a message to syslog. */
void logMsg(int level, char * pszMsg, ...)
{
    va_list args;
    if ((level == LOG_DEBUG) && !fDebug) return;
    va_start(args, pszMsg);
    if (fDebug) {
        vfprintf(stderr, pszMsg, args);
        fprintf(stderr, "\n");
        fflush(stderr);
    } else {
        vsyslog(level, pszMsg, args);
    }
    va_end(args);
}
