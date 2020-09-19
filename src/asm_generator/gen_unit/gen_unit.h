#pragma once

#include <string>
#include "../asm.h"

using std::string;

const int BSS_SEC_ID = 1;
const int DATA_SEC_ID = BSS_SEC_ID + 1;

// operands
extern const string stack_pointer;
extern const string link_register;
extern const string program_counter;

string gpr(int id);
// the id-th general purpose register (0 <= id < 13)
string constant(unsigned int value);
// value is of one of the forms:
// 1. produced by shifting an 2-byte value left
// 2. 0x00XY00XY
// 3. 0xXY00XY00
// 4. 0xXYXYXYXY
string reg_with_shift(string Rm, string shift_opr, string amount);
// shift_opr: asr, lsl, lsr, ror, rrx
// amount: a constant or a register

// labels
string label(int id);
string new_label();
string array_name(int id);
LinePtr gen_label(const string &label);

LinePtr gen_b(const string &cond, const string &label);
//
// data-processing instructions
LinePtr gen_lsl(bool S, const string &cond, const string &Rd, const string &Rm,
               const string &rs);
LinePtr gen_lsl(bool S, const string &cond, const string &Rd, const string &Rm,
               int sh);
LinePtr gen_lsr(bool S, const string &cond, const string &Rd, const string &Rm,
               const string &rs);
LinePtr gen_lsr(bool S, const string &cond, const string &Rd, const string &Rm,
               int sh);
LinePtr gen_asr(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Rm);
LinePtr gen_asl(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Rm);
LinePtr gen_ror();
LinePtr gen_rrx();
LinePtr gen_adc(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Operand2);
LinePtr gen_add(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Operand2);
LinePtr gen_adr();
LinePtr gen_and();
LinePtr gen_bic(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Operand2);
LinePtr gen_cmp(const string &cond, const string &Rn, const string &Operand2);
LinePtr gen_eor();
LinePtr gen_mov(bool S, const string &cond, const string &Rd, const string &Operand2);
LinePtr gen_mov(const string &cond, const string &Rd, int imm16);
LinePtr gen_movw(const string &cond, const string &Rd, int imm16);
LinePtr gen_movt(const string &cond, const string &Rd, int imm16);
void gen_mov32(Lines &prog, const string &Rd, int imm32);
void gen_mov32(Lines &prog, const string &cond, const string &Rd, int imm32);
LinePtr gen_mul(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Rm);
LinePtr gen_mla(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Rm, const string &Ra);
LinePtr gen_mls(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Rm, const string &Ra);
LinePtr gen_neg(const string &cond, const string &Rd, const string &Rm);
LinePtr gen_smmul(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Rm);
LinePtr gen_orn();
LinePtr gen_orr();
LinePtr gen_rsb(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Operand2);
LinePtr gen_rsc();
LinePtr gen_sbc();
LinePtr gen_sub(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Operand2);
LinePtr gen_rsb(bool S, const string &cond, const string &Rd, const string &Rn,
               const string &Operand2);
LinePtr gen_teq();
LinePtr gen_tst();

LinePtr gen_bl(const string &function_name);
LinePtr gen_push(const string &push_registers);
LinePtr gen_pop(const string &pop_registers);
LinePtr gen_bx_lr();
// ...

// status register access instructions

// load/store registers
LinePtr gen_ldr(const string &cond, const string &Rt, const string &label);
LinePtr gen_ldr(const string &cond, const string &Rt, const string &Rn, int offset);
LinePtr gen_ldr(const string &cond, const string &Rt, const string &Rn, const string &Rm);
LinePtr gen_str(const string &cond, const string &Rt, const string &Rn, int offset);
LinePtr gen_str(const string &cond, const string &Rt, const string &Rn, const string &Rm);
LinePtr gen_ldr(const string &cond, const string &Rt, const string &Rn, const string &Rm, const string &shift);

// ...
LinePtr gen_info(const string &info);
LinePtr gen_word(const int &word);
