/**
 * @ingroup  mksc68_prg
 * @file     mksc68/mksc68_gdb.h
 * @author   Benjamin Gerard
 * @date     2013-07-09
 * @brief    remote gdb.
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef MKSC68_GDB_H
#define MKSC68_GDB_H

#include "mksc68_def.h"

/**
 * remote gdb object opaque structure type.
 */
typedef struct gdb_s gdb_t;

/**
 * Enumerates running status values (gdb::run) returned by the
 * gdb_event() function.
 */
enum gdbrun_e {
  RUN_IDLE,                       /**< waiting for gdb              */
  RUN_STOP,                       /**< just stopped (breakpoint...) */
  RUN_CONT,                       /**< running (stepping or normal) */
  RUN_EXIT,                       /**< exit requested               */
  RUN_SKIP,                       /**< detached / ignore            */
};

/**
 * Enumerates return code values (gdb_t::code) returned by the
 * gdb_error() function.
 */
enum gdbcode_e {
  CODE_IDLE,                            /**< stub is idle */
  CODE_ERROR,                           /**< stub has failed */
  CODE_KILL,                            /**< TODO */
  CODE_DETACH,                          /**< TODO */
  CODE_TRACE,                           /**< TODO */
  CODE_STOP,                            /**< TODO */
  CODE_BREAK,                           /**< TODO */
  CODE_HALT,                            /**< TODO */
  CODE_INT,                             /**< TODO */
  CODE_PRIVATE,                         /**< TODO */
};

/* #define DEFSIG(NAME,VAL,X,Y) SIGVAL_#NAME = VAL */

/**
 * Enumerates gdb signals.
 */
enum {
  SIGVAL_0       = 0,  /**< Signal 0.                    */
  SIGVAL_HUP     = 1,  /**< Hangup.                      */
  SIGVAL_INT     = 2,  /**< Interrupt.                   */
  SIGVAL_QUIT    = 3,  /**< Quit.                        */
  SIGVAL_ILL     = 4,  /**< Illegal instruction.         */
  SIGVAL_TRAP    = 5,  /**< Trace/breakpoint trap.       */
  SIGVAL_ABRT    = 6,  /**< Aborted.                     */
  SIGVAL_EMT     = 7,  /**< Emulation trap.              */
  SIGVAL_FPE     = 8,  /**< Arithmetic exception.        */
  SIGVAL_KILL    = 9,  /**< Killed.                      */
  SIGVAL_BUS     = 10, /**< Bus error.                   */
  SIGVAL_SEGV    = 11, /**< Segmentation fault.          */
  SIGVAL_SYS     = 12, /**< Bad system call.             */
  SIGVAL_PIPE    = 13, /**< Broken pipe.                 */
  SIGVAL_ALRM    = 14, /**< Alarm clock.                 */
  SIGVAL_TERM    = 15, /**< Terminated.                  */
  SIGVAL_URG     = 16, /**< Urgent I/O condition.        */
  SIGVAL_STOP    = 17, /**< Stopped (signal).            */
  SIGVAL_TSTP    = 18, /**< Stopped (user).              */
  SIGVAL_CONT    = 19, /**< Continued.                   */
  SIGVAL_CHLD    = 20, /**< Child status changed.        */
  SIGVAL_TTIN    = 21, /**< Stopped (tty input).         */
  SIGVAL_TTOU    = 22, /**< Stopped (tty output).        */
  SIGVAL_IO      = 23, /**< I/O possible.                */
  SIGVAL_XCPU    = 24, /**< CPU time limit exceeded.     */
  SIGVAL_XFSZ    = 25, /**< File size limit exceeded.    */
  SIGVAL_VTALRM  = 26, /**< Virtual timer expired.       */
  SIGVAL_PROF    = 27, /**< Profiling timer expired.     */
  SIGVAL_WINCH   = 28, /**< Window size changed.         */
  SIGVAL_LOST    = 29, /**< Resource lost.               */
  SIGVAL_USR1    = 30, /**< User defined signal 1.       */
  SIGVAL_USR2    = 31, /**< User defined signal 2.       */
  SIGVAL_PWR     = 32, /**< Power fail/restart.          */
  SIGVAL_POLL    = 33, /**< Pollable event occurred.     */
  SIGVAL_WIND    = 34, /**< WIND.                        */
  SIGVAL_PHONE   = 35, /**< PHONE.                       */
  SIGVAL_WAITING = 36, /**< Process's LWPs are blocked.  */
  SIGVAL_LWP     = 37, /**< Signal LWP.                  */
  SIGVAL_DANGER  = 38, /**< Swap space dangerously low.  */
  SIGVAL_GRANT   = 39, /**< Monitor mode granted.        */
  SIGVAL_RETRACT = 40, /**< ???.                         */
  SIGVAL_MSG     = 41, /**< Monitor mode data available. */
  SIGVAL_SOUND   = 42, /**< Sound completed.             */
  SIGVAL_SAK     = 43, /**< Secure attention.            */
  SIGVAL_PRIO    = 44, /**< PRIO.                        */
};

EXTERN68
/**
 * Create a gdb stub.
 *
 *    the gdb_create() function creates a new gdb stub and wait for
 *    one gdb incomming connection on the channel defined by the
 *    gdb_conf() function.
 *
 *  @return  gdb object
 *  @retval  0 on error
 */
gdb_t * gdb_create(void);

EXTERN68
/**
 * Destroy a gdb stub.
 *
 *    The gdb_destroy() function closes the communication socket with
 *    gdb and free all resources allocated.
 *
 *  @param  gdb  gdb object (null is safe)
 */
void gdb_destroy(gdb_t * gdb);

EXTERN68
/**
 * Process gdb stub event.
 *
 * @param  gdb     gdb object
 * @param  vector  exception vector (from emu68 simulator)
 * @param  emu68   the emu68 simulator instance
 *
 * @return a gdbrun_e value
 */
int gdb_event(gdb_t * gdb, int vector, void * emu68);

EXTERN68
/**
 * Get the lastest gdb stub code status.
 *
 *   The gdb_error() function gets the stub code status and the last
 *   message associate. The message returned into errp is volatile. It
 *   should be use or copied before resuming any operation, including
 *   running the 68k simulator.
 *
 * @param  gdb   gdb object
 * @param  errp  receive the last message. Null is safe.
 *
 * @return a gdbcode_e enum value
 * @retval -1  not a valid gdb strub
 */
int gdb_error(gdb_t * gdb, const char ** errp);

EXTERN68
/**
 * Set the gdb stub configuration.
 *
 *   The gdb_conf() function configures the next gdb stubs that
 *   created by the gdb_create() function. At the moment this URI is
 *   in fact an IPV4 host:port address only.
 *
 * @param  uri  At the moment HOST:PORT string
 *
 * @retval  0 on success
 * @retval -1 on failure
 */
int gdb_conf(char * uri);

EXTERN68
/**
 * Get the gdb stub configuration.
 *
 *   The gdb_get_conf() function returns the current gdb configuration
 *   as set by the gdb_conf() function.
 *
 * @param addr if non nil, a 4 bytes buffer to receive the ipv4 address
 * @param port if non nil, receive the ipv4 port
 */
void gdb_get_conf(unsigned char * addr, int * port);

#endif
