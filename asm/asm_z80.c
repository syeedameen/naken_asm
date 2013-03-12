/**
 *  naken_asm assembler.
 *  Author: Michael Kohn
 *   Email: mike@mikekohn.net
 *     Web: http://www.mikekohn.net/
 * License: GPL
 *
 * Copyright 2010-2013 by Michael Kohn
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "asm_common.h"
#include "asm_z80.h"
#include "assembler.h"
#include "table_z80.h"
#include "get_tokens.h"
#include "eval_expression.h"

enum
{
  OPERAND_NONE,
  OPERAND_NUMBER,
  OPERAND_REG8,
  OPERAND_REG_IHALF,
  OPERAND_REG16_XY,
  OPERAND_REG16,
};

enum
{
  REG_B=0,
  REG_C,
  REG_D,
  REG_E,
  REG_H,
  REG_L,
  REG_INDEX_HL,
  REG_A=7,
};

enum
{
  REG_IXH=0,
  REG_IXL,
  REG_IYH,
  REG_IYL,
  REG_IX,
  REG_IY,
};

enum
{
  REG_BC=0,
  REG_DE,
  REG_HL,
  REG_SP,
};

struct _operand
{
  int value;
  int type;
  int offset;
};

int parse_instruction_z80(struct _asm_context *asm_context, char *instr)
{
char token[TOKENLEN];
int token_type;
char instr_case[TOKENLEN];
struct _operand operands[3];
int operand_count=0;
int matched=0;
int num;
int n;

  lower_copy(instr_case, instr);

  memset(&operands, 0, sizeof(operands));
  while(1)
  {
    token_type=get_token(asm_context, token, TOKENLEN);
    if (token_type==TOKEN_EOL || token_type==TOKEN_EOF)
    {
      break;
    }

    if (operand_count>=3)
    {
      print_error_opcount(instr, asm_context);
      return -1;
    }

    if (IS_TOKEN(token,'a') || IS_TOKEN(token,'A'))
    {
      operands[operand_count].type=OPERAND_REG8;
      operands[operand_count].value=REG_A;
    }
      else
    if (IS_TOKEN(token,'b') || IS_TOKEN(token,'B'))
    {
      operands[operand_count].type=OPERAND_REG8;
      operands[operand_count].value=REG_B;
    }
      else
    if (IS_TOKEN(token,'c') || IS_TOKEN(token,'C'))
    {
      operands[operand_count].type=OPERAND_REG8;
      operands[operand_count].value=REG_C;
    }
      else
    if (IS_TOKEN(token,'d') || IS_TOKEN(token,'D'))
    {
      operands[operand_count].type=OPERAND_REG8;
      operands[operand_count].value=REG_D;
    }
      else
    if (IS_TOKEN(token,'e') || IS_TOKEN(token,'E'))
    {
      operands[operand_count].type=OPERAND_REG8;
      operands[operand_count].value=REG_E;
    }
      else
    if (IS_TOKEN(token,'h') || IS_TOKEN(token,'H'))
    {
      operands[operand_count].type=OPERAND_REG8;
      operands[operand_count].value=REG_H;
    }
      else
    if (IS_TOKEN(token,'l') || IS_TOKEN(token,'L'))
    {
      operands[operand_count].type=OPERAND_REG8;
      operands[operand_count].value=REG_L;
    }
      else
    if (strcasecmp(token,"ixh")==0)
    {
      operands[operand_count].type=OPERAND_REG_IHALF;
      operands[operand_count].value=REG_IXH;
    }
      else
    if (strcasecmp(token,"ixl")==0)
    {
      operands[operand_count].type=OPERAND_REG_IHALF;
      operands[operand_count].value=REG_IXL;
    }
      else
    if (strcasecmp(token,"iyh")==0)
    {
      operands[operand_count].type=OPERAND_REG_IHALF;
      operands[operand_count].value=REG_IYH;
    }
      else
    if (strcasecmp(token,"iyl")==0)
    {
      operands[operand_count].type=OPERAND_REG_IHALF;
      operands[operand_count].value=REG_IYL;
    }
      else
    if (strcasecmp(token,"bc")==0)
    {
      operands[operand_count].type=OPERAND_REG16;
      operands[operand_count].value=REG_BC;
    }
      else
    if (strcasecmp(token,"de")==0)
    {
      operands[operand_count].type=OPERAND_REG16;
      operands[operand_count].value=REG_DE;
    }
      else
    if (strcasecmp(token,"hl")==0)
    {
      operands[operand_count].type=OPERAND_REG16;
      operands[operand_count].value=REG_HL;
    }
      else
    if (strcasecmp(token,"sp")==0)
    {
      operands[operand_count].type=OPERAND_REG16;
      operands[operand_count].value=REG_SP;
    }
      else
    if (IS_TOKEN(token,'('))
    {
      token_type=get_token(asm_context, token, TOKENLEN);
      if (strcasecmp(token, "hl")==0)
      {
        operands[operand_count].type=OPERAND_REG8;
        operands[operand_count].value=REG_INDEX_HL;
      }
        else
      if (strcasecmp(token, "ix")==0)
      {
        operands[operand_count].type=OPERAND_REG16_XY;
        operands[operand_count].value=REG_IX;
        token_type=get_token(asm_context, token, TOKENLEN);
        pushback(asm_context, token, token_type);
        if (IS_NOT_TOKEN(token,')'))
        {
          if (eval_expression(asm_context, &num)!=0)
          {
            if (asm_context->pass==1)
            {
              eat_operand(asm_context);
              num=0;
            }
              else
            {
              print_error_illegal_expression(instr, asm_context);
              return -1;
            }
          }
          operands[operand_count].offset=num;
        }
      }
        else
      if (strcasecmp(token, "iy")==0)
      {
        operands[operand_count].type=OPERAND_REG16_XY;
        operands[operand_count].value=REG_IY;

        token_type=get_token(asm_context, token, TOKENLEN);
        pushback(asm_context, token, token_type);
        if (IS_NOT_TOKEN(token,')'))
        {
          if (eval_expression(asm_context, &num)!=0)
          {
            if (asm_context->pass==1)
            {
              eat_operand(asm_context);
              num=0;
            }
              else
            {
              print_error_illegal_expression(instr, asm_context);
              return -1;
            }
          }
          operands[operand_count].offset=num;
        }
      }
        else
      {
        print_error_unexp(token, asm_context);
        return -1;
      }
      if (expect_token_s(asm_context,")")!=0) { return -1; }
    }
      else
    {
      pushback(asm_context, token, token_type);

      if (eval_expression(asm_context, &num)!=0)
      {
        if (asm_context->pass==1)
        {
          eat_operand(asm_context);
        }
          else
        {
          print_error_illegal_expression(instr, asm_context);
          return -1;
        }
      }

      operands[operand_count].type=OPERAND_NUMBER;
      operands[operand_count].value=num;
    }

    operand_count++;
    token_type=get_token(asm_context, token, TOKENLEN);
    if (token_type==TOKEN_EOL) break;
    if (IS_NOT_TOKEN(token,',') || operand_count==3)
    {
      print_error_unexp(token, asm_context);
      return -1;
    }
  }

  // Instruction is parsed, now find matching opcode

  n=0;
  while(table_z80[n].instr!=NULL)
  {
    if (strcmp(table_z80[n].instr,instr_case)==0)
    {
       matched=1;
      switch(table_z80[n].type)
      {
        case OP_NONE:
          if (operand_count==0)
          {
            add_bin8(asm_context, table_z80[n].opcode, IS_OPCODE);
            return 1;
          }
        case OP_NONE16:
          if (operand_count==0)
          {
            add_bin8(asm_context, table_z80[n].opcode>>8, IS_OPCODE);
            add_bin8(asm_context, table_z80[n].opcode&0xff, IS_OPCODE);
            return 2;
          }
        case OP_A_REG8:
          if (operand_count==2 &&
              operands[0].type==OPERAND_REG8 &&
              operands[0].value==REG_A &&
              operands[1].type==OPERAND_REG8)
          {
            add_bin8(asm_context, table_z80[n].opcode|operands[1].value, IS_OPCODE);
            return 1;
          }
          break;
        case OP_REG8:
          if (operand_count==1 &&
              operands[0].type==OPERAND_REG8)
          {
            add_bin8(asm_context, table_z80[n].opcode|operands[0].value, IS_OPCODE);
            return 1;
          }
          break;
        case OP_A_REG_IHALF:
          if (operand_count==2 &&
              operands[0].type==OPERAND_REG8 &&
              operands[0].value==REG_A &&
              operands[1].type==OPERAND_REG_IHALF)
          {
            unsigned char y=(operands[1].value>>1);
            unsigned char l=(operands[1].value&0x1);
            add_bin8(asm_context, (table_z80[n].opcode>>8)|(y<<5), IS_OPCODE);
            add_bin8(asm_context, (table_z80[n].opcode&0xff)|l, IS_OPCODE);
            return 2;
          }
          break;
        case OP_A_INDEX:
          if (operand_count==2 &&
              operands[0].type==OPERAND_REG8 &&
              operands[0].value==REG_A &&
              operands[1].type==OPERAND_REG16_XY)
          {
            add_bin8(asm_context, (table_z80[n].opcode>>8)|((operands[1].value&0x1)<<5), IS_OPCODE);
            add_bin8(asm_context, table_z80[n].opcode&0xff, IS_OPCODE);
            add_bin8(asm_context, (unsigned char)operands[1].offset, IS_OPCODE);
            return 3;
          }
          break;
        case OP_A_NUMBER8:
          if (operand_count==2 &&
              operands[0].type==OPERAND_REG8 &&
              operands[0].value==REG_A &&
              operands[1].type==OPERAND_NUMBER)
          {
            if (operands[1].value<-128 || operands[1].value>127)
            {
              print_error_range("Constant", -128, 127, asm_context);
              return -1;
            }
            add_bin8(asm_context, table_z80[n].opcode, IS_OPCODE);
            add_bin8(asm_context, (unsigned char)operands[1].value, IS_OPCODE);
            return 2;
          }
          break;
        case OP_HL_REG16_1:
          if (operand_count==2 &&
              operands[0].type==OPERAND_REG16 &&
              operands[0].value==REG_HL &&
              operands[1].type==OPERAND_REG16)
          {
            add_bin8(asm_context, table_z80[n].opcode|(operands[1].value<<4), IS_OPCODE);
            return 1;
          }
        case OP_HL_REG16_2:
          if (operand_count==2 &&
              operands[0].type==OPERAND_REG16 &&
              operands[0].value==REG_HL &&
              operands[1].type==OPERAND_REG16)
          {
            add_bin8(asm_context, table_z80[n].opcode>>8, IS_OPCODE);
            add_bin8(asm_context, (table_z80[n].opcode&0xff)|(operands[1].value<<4), IS_OPCODE);
            return 2;
          }
      }
    }
    n++;
  }

  if (matched==1)
  {
    printf("Error: Unknown operands combo for '%s' at %s:%d.\n", instr, asm_context->filename, asm_context->line);
  }
    else
  {
    printf("Error: Unknown instruction '%s' at %s:%d.\n", instr, asm_context->filename, asm_context->line);
  }

  return -1;
}



