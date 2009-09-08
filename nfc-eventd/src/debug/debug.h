/*
 * PKCS #11 PAM Login Module
 * Copyright (C) 2003 Mario Strasser <mast@gmx.net>,
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * $Id: debug.h 246 2007-04-12 10:09:12Z ludovic.rousseau $
 */

/**
*@def DEBUG
* This module contains macros for generate debugging messages
* Will be compiled an linked only when -DDEBUG CFLAG is used
*/

#ifndef __DEBUG_H_
#define __DEBUG_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef DEBUG
  #warning "Debugging is completely disabled!"
  #define DBG(...) {}
  #define set_debug_level(l, ...) {}
  #define debug(l, ...) {}
#else
  #define DBG(x,...) debug_print(1, __FILE__, __LINE__, x, ## __VA_ARGS__ )
  #define ERR(x,...) debug_print(-1, __FILE__, __LINE__, x, ## __VA_ARGS__ )

  #ifndef __DEBUG_C_
    #define DEBUG_EXTERN extern
  #else 
    #define DEBUG_EXTERN
  #endif

/**
 * set_debug_level() Sets the current debug level.
 *@param level. New debug level
 */
DEBUG_EXTERN void set_debug_level(int level);

/**
 * get_debug_level() Returns the current debug level.
 *@return Current debug level
 */
DEBUG_EXTERN int get_debug_level(void);

/**
 * debug_print() prints the given message 

 * if the current debug-level 
 * is greater or equal to the defined level. The format string as well as all
 * further arguments are interpreted as by the printf() function. 
 *@param level Debug level of message
 *@param file Name of the file where message is generated
 *@param line Line number where message is generated
 *@param format Message format
 *@param .... Optional arguments
 */
DEBUG_EXTERN void debug_print(int level, const char *file, int line, const char *format, ...);

  #undef DEBUG_EXTERN

#endif /* DEBUG */

#endif /* __DEBUG_H_ */
