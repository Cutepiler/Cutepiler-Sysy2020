#ifndef _toarm_ins
#define _toarm_ins
#include <iostream>
#include <string>
void ins_pop(const char* arg1, std::ostream &os);
void ins_push(const char* arg1, std::ostream &os);
void ins_mov(const char* arg1, const char* arg2, std::ostream &os);
void ins_mov(const char* arg1, int arg2, std::ostream &os);//assert: arg2 is a legal immediate
void ins_mov(int arg1, int arg2, std::ostream &os);
void ins_mov_lsl(int arg1, int arg2, int bits, std::ostream &os);
void ins_mov_imme(int arg1, int imme, std::ostream &os);
void ins_mvn_imme(int arg1, int imme, std::ostream &os);
void ins_movt_imme(int arg1, int imme, std::ostream &os);
void ins_ldr(int target, const char* base, int offset, std::ostream &os);
void ins_str(int target, const char* base, int offset, std::ostream &os);
void ins_label(const char* arg1, std::ostream &os);
void ins_label(const std::string arg1, std::ostream &os);
void ins_globl(const char* arg1, std::ostream &os);
void ins_bl(const char * arg1, std::ostream &os);
void ins_neg(int arg1, int arg2, std::ostream &os);
void ins_add_imme(const char* arg1, int arg2, std::ostream &os);
void ins_add(int arg1, int arg2, int arg3, std::ostream &os);
void ins_sub_imme(const char* arg1, int arg2, std::ostream &os);
void ins_sub_imme(int arg1, int arg2, std::ostream &os);
void ins_sub(int arg1, int arg2, int arg3, std::ostream &os);
void ins_mul(int arg1, int arg2, int arg3, std::ostream &os);
void ins_div(int arg1, int arg2, int arg3, std::ostream &os);
void ins_mod(int arg1, int arg2, int arg3, std::ostream &os);
void ins_lt(int arg1, int arg2, int arg3, std::ostream &os);
void ins_gt(int arg1, int arg2, int arg3, std::ostream &os);
void ins_le(int arg1, int arg2, int arg3, std::ostream &os);
void ins_ge(int arg1, int arg2, int arg3, std::ostream &os);
void ins_eq(int arg1, int arg2, int arg3, std::ostream &os);
void ins_ne(int arg1, int arg2, int arg3, std::ostream &os);
void ins_and(int arg1, int arg2, int arg3, std::ostream &os);
void ins_orr(int arg1, int arg2, int arg3, std::ostream &os);
void ins_ldr(int arg1, int arg2, int offset, std::ostream &os);
void ins_str(int arg1, int arg2, int offset, std::ostream &os);
#endif
