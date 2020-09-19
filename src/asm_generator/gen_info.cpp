#include "asm.h"
#include "gen_unit/gen_unit.h"

void genProgramStart(Lines &prog){
	// .syntax unified
	// .globl main
	prog.push_back(gen_info(".syntax unified"));
	prog.push_back(gen_info(".globl main"));
	return;
}
void genBss(Lines &prog, int bss_length){
	// 用到bsssec, datasec的地方都搞个标签池?
	//.bss
	//bsssec:
	//.align 2
	//.zero len * 4
	prog.push_back(gen_info(".bss"));
	prog.push_back(gen_info(".align 2"));
	prog.push_back(gen_info("bsssec:"));
	prog.push_back(gen_info(".zero "+std::to_string(bss_length * 4)));
}
void genData(Lines &prog,const std::vector<int>& Data){
	//.data
	//.align 2
	//datasec:
	//.word 1,2,3,....,99999
	prog.push_back(gen_info(".data"));
	prog.push_back(gen_info(".align 2"));
	prog.push_back(gen_info("datasec:"));
	for(auto x:Data){
	    prog.push_back(gen_word(x));
	}

}

