/**
 * @ingroup  mksc68_eva
 * @file     mksc68/mksc68_eva.h
 * @author   Benjamin Gerard
 * @date     2009-01-01
 * @brief    expression eval
 *
 */

/* Copyright (c) 1998-2015 Benjamin Gerard */

#ifndef MKSC68_EVA_H
#define MKSC68_EVA_H

#include "mksc68_def.h"

EXTERN68
/** Evaluate a string 0 or space terminated.
 *
 * @param   expr     expression string to evaluate.
 * @param   status   pointer to status.
 * @return           result of expression.
 */
int eval(char * expr, int * status);

#endif
