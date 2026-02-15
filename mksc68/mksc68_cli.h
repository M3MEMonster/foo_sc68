/**
 * @ingroup  mksc68_prg
 * @file     mksc68_cli.h
 * @author   Benjamin Gerard
 * @date     2011-11-01
 * @brief    mksc68 command line header.
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef MKSC68_CLI_H
#define MKSC68_CLI_H

#include "mksc68_def.h"

EXTERN68
/** Read and parse command line.
 *
 *  The mksc68_cli_read() function display prompt and read command line.
 *  The previous command line is freed by mksc68_cli_free().
 *  Optionnaly the function can use readline library. In that case it handles
 *  an history and any readline basic functionnalities..
 *
 * @param  prompt  Displayed prompt if not 0.
 * @return number of arguments in command line.
 * @retval -1  error
 * @retval 0   no arguments (empty command line)
 *
 * @warning Be sure to clear the comlime struct before the first call to
 *          prevents mksc68_cli_free() to try to free invalid buffer.
 */
int cli_read(char * argv[], int max);

EXTERN68
/** Set command line prompt.
 */
char * cli_prompt(const char * fmt, ...);

EXTERN68
/** Release prompt et command line buffers.
 */
void cli_release(void);

#endif
