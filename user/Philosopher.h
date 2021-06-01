/* Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
 *
 * Use of this source code is restricted per the CC BY-NC-ND license, a copy of
 * which can be found via http://creativecommons.org (and should be included as
 * LICENSE.txt within the associated archive or repository).
 */

 #ifndef __PHILOSOPHER_H
 #define __PHILOSOPHER_H

 #include <stdbool.h>
 #include <stddef.h>
 #include <stdint.h>
 #include <stdio.h>

 #include <string.h>

 #include "PL011.h"

 #include "libc.h"

 #define MAX_CMD_CHARS ( 1024 )
 #define MAX_CMD_ARGS  (    2 )

 typedef enum {
   INVALID_STATUS,

   HUNGRY_STATUS,
   THINKING_STATUS,
   HOLDING_LEFT_FORK_STATUS,
   HOLDING_RIGHT_FORK_STATUS,
   EATING_STATUS,
 } status_p;

 #endif
