/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: http://www.mikekohn.net/
 * License: GPLv3
 *
 * Copyright 2010-2018 by Michael Kohn
 *
 */

#ifndef _ASM_COPPER_H
#define _ASM_COPPER_H

#include "common/assembler.h"

int parse_instruction_copper(struct _asm_context *asm_context, char *instr);

#endif

