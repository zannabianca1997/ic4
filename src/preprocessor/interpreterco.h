/**
 * @file interpreterco.h
 * @author zannabianca1997 (zannabianca199712@gmail.com)
 * @brief Interface to open the interpreter
 * @version 0.1
 * @date 2022-03-14
 *
 * @copyright Copyright (c) 2022
 */

#ifndef _INTERPRETERCO_H
#define _INTERPRETERCO_H

#include "interpreter.h"
#include "filemanager.h"

/**
 * @brief Create a new interpreter, reading a translation unit
 *
 * @param context the context creating the interpreter
 * @param file the starting file of the unit
 * @return pp_interpreter_t* the interpreter opened
 */
pp_interpreter_t *pp_interpreter_open(context_t *context, pp_file_t *tsunit);

#endif // _INTERPRETERCO_H