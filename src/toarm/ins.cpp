#include "ins.h"
void ins_pop(const char* arg1, std::ostream &os){
    os << "pop" << " {" << arg1<<"} " << std::endl;
}
void ins_push(const char* arg1, std::ostream &os){
    os << "push" << " {" << arg1 <<"} "<< std::endl;
}
void ins_mov(const char* arg1, const char* arg2, std::ostream &os){
    os << "mov" << " " << arg1 << ", " << arg2 << std::endl;
}
void ins_mov(const char* arg1, int arg2, std::ostream &os){//assert: arg2 is a legal immediate 
    os << "mov" << " " << arg1 << ", " << "#" << arg2 << std::endl;
}
void ins_mov(int arg1, int arg2, std::ostream &os){
    os << "mov" <<" "<<"r"<<arg1 <<", "<<"r"<<arg2<<std::endl;
}
void ins_mov_lsl(int arg1, int arg2, int bits, std::ostream &os){
    os << "mov" << " " <<"r"<<arg1<<", "<<"r"<<arg2<<", "<<"LSL"<<" "<<"#"<<bits<<std::endl;
}
void ins_mov_imme(int arg1, int imme, std::ostream &os){
    os << "mov" <<" "<<"r"<<arg1<<", "<<"#"<<imme<<std::endl;
}
void ins_mvn_imme(int arg1, int imme, std::ostream &os){
    os << "mvn" <<" "<<"r"<<arg1<<", "<<"#"<<imme<<std::endl;
}
void ins_movt_imme(int arg1, int imme, std::ostream &os){
    os << "movt" <<" "<<"r"<<arg1<<", "<<"#"<<imme<<std::endl;
}
void ins_ldr(int target, const char* base, int offset, std::ostream &os){
    os << "ldr" << " " << "r" << target << ", " <<"["<< base <<", "<<"#"<< offset <<"]" << std::endl;
}
void ins_str(int target, const char* base, int offset, std::ostream &os){
    os << "str" << " " << "r" << target <<", " << "[" << base <<", "<<"#"<<offset<<"]" << std::endl;
}
void ins_label(const char* arg1, std::ostream &os){
    os << arg1 <<":" << std::endl;
}
void ins_label(const std::string arg1, std::ostream &os){
    os << arg1 <<":" << std::endl;
}
void ins_globl(const char* arg1, std::ostream &os){
    os << ".globl" << " " << arg1 << std::endl;
}
void ins_bl(const char * arg1, std::ostream &os){
    os << "bl" << " " << arg1 << std::endl;
}


//arithmetic
void ins_neg(int arg1, int arg2, std::ostream &os){
    os << "neg" << " " << "r"<<arg1 <<", "<< "r"<<arg2 << std::endl;
}
void ins_add_imme(const char* arg1, int arg2, std::ostream &os){//assert the arg2 can be expressed in Opd2
    os << "add" << " " << arg1 <<", "<< "#" <<arg2 << std::endl;
}
void ins_add(int arg1, int arg2, int arg3, std::ostream &os){
    os << "add" << " " <<"r"<<arg1<<", "<<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
}
void ins_sub_imme(int arg1, int arg2, std::ostream &os){
    os << "sub" << " " << "r"<<arg1 <<", "<< "#" <<arg2 << std::endl;
}
void ins_sub_imme(const char* arg1, int arg2, std::ostream &os){
    os << "sub" << " " << arg1 <<", "<< "#" <<arg2 << std::endl;
}
void ins_sub(int arg1, int arg2, int arg3, std::ostream &os){
    os << "sub" << " " <<"r"<<arg1<<", "<<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
}
void ins_mul(int arg1, int arg2, int arg3, std::ostream &os){
    os << "mul" << " " <<"r"<<arg1<<", "<<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
}
void ins_div(int arg1, int arg2, int arg3, std::ostream &os){
    os << "sdiv" << " " <<"r"<<arg1<<", "<<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
}
void ins_mod(int arg1, int arg2, int arg3, std::ostream &os){
    os << "sdiv" << " " <<"r"<<arg1<<", "<<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
    os << "mul" << " " <<"r"<<arg1<<", "<<"r"<<arg1<<", "<<"r"<<arg3<<std::endl;
    os << "sub" << " " <<"r"<<arg1<<", "<<"r"<<arg2<<", "<<"r"<<arg1<<std::endl;

}
void ins_lt(int arg1, int arg2, int arg3, std::ostream &os){
    os << "cmp" << " " <<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
    os << "mov" <<" "<<"r"<< arg1 <<", "<< "#0" << std::endl;
    os << "movlt" <<" "<<"r" << arg1 <<", "<<"#1" << std::endl; 
}
void ins_gt(int arg1, int arg2, int arg3, std::ostream &os){
    os << "cmp" << " " <<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
    os << "mov" <<" "<<"r"<< arg1 <<", "<< "#0" << std::endl;
    os << "movgt" <<" "<<"r" << arg1 <<", "<<"#1" << std::endl; 
}
void ins_ge(int arg1, int arg2, int arg3, std::ostream &os){
    os << "cmp" << " " <<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
    os << "mov" <<" "<<"r"<< arg1 <<", "<< "#0" << std::endl;
    os << "movge" <<" "<<"r" << arg1 <<", "<<"#1" << std::endl; 
}
void ins_le(int arg1, int arg2, int arg3, std::ostream &os){
    os << "cmp" << " " <<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
    os << "mov" <<" "<<"r"<< arg1 <<", "<< "#0" << std::endl;
    os << "movls" <<" "<<"r" << arg1 <<", "<<"#1" << std::endl; // lower or same
}
void ins_eq(int arg1, int arg2, int arg3, std::ostream &os){
    os << "cmp" << " " <<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
    os << "mov" <<" "<<"r"<< arg1 <<", "<< "#0" << std::endl;
    os << "moveq" <<" "<<"r" << arg1 <<", "<<"#1" << std::endl; 
}
void ins_ne(int arg1, int arg2, int arg3, std::ostream &os){
    os << "cmp" << " " <<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
    os << "mov" <<" "<<"r"<< arg1 <<", "<< "#0" << std::endl;
    os << "movne" <<" "<<"r" << arg1 <<", "<<"#1" << std::endl; 
}
void ins_and(int arg1, int arg2, int arg3, std::ostream &os){
    os << "and" << " " <<"r"<<arg1<<", "<<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
}
void ins_orr(int arg1, int arg2, int arg3, std::ostream &os){
    os << "orr" << " " <<"r"<<arg1<<", "<<"r"<<arg2<<", "<<"r"<<arg3<<std::endl;
}


void ins_ldr(int arg1, int arg2, int offset, std::ostream &os){
    os << "ldr" << " " <<"r"<<arg1<<", "<<"[" <<"r"<<arg2<<", "<<"#"<<offset <<"]"<<std::endl;
}
void ins_str(int arg1, int arg2, int offset, std::ostream &os){
    os << "str" << " " <<"r"<<arg1<<", "<<"[" <<"r"<<arg2<<", "<<"#"<<offset <<"]"<<std::endl;
}
