#include "asm.h"
#include "gen_unit/gen_unit.h"
#include <cstdio>
#include <set>

using std::set;

void genParam(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg, int param_count) {
	// 函数caller准备工作
	// (传参)
	// 统统压栈, 统统压栈
	assert(reg.find(tac.opd1)!=reg.end());
	int param_reg = reg.at(tac.opd1);
	prog.push_back(gen_str("", gpr(param_reg), "sp", -4*(6 + 1 + param_count)));
}

void genMod(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
	//  		
	// r0 到 r3, r9, r12
	//prog.push_back(gen_push("r0"));
	//prog.push_back(gen_push("r1-r3,r9,r12"));
	int target = reg.at(tac.opd1);
	//把参数放到r0, r1
	
	std::set<TacOpd> livein = tac.getLiveIn(); 
	std::vector<int> AllLive;
	for(auto opd:livein){
		if(reg.find(opd)!=reg.end()){
			int r = reg.at(opd);
			if( r == 0 || r == 1 || r == 2 || r == 3 || r == 9 || r == 12){
				/*int offset = -4 * (r + 1);
				if(r==9) offset = -4 * 5;
				if(r==12) offset = -4*6;*/
				AllLive.push_back(r);
				prog.push_back(gen_push(gpr(r)));
				//prog.push_back(gen_str("", gpr(r), "sp", offset));	
			}
		}
	}
	
	if(tac.opd2.getType()==OpdType::Imme && tac.opd3.getType()==OpdType::Imme){
		gen_mov32(prog, "r0", tac.opd2.getVal());
		gen_mov32(prog, "r1", tac.opd3.getVal());
	}else if(tac.opd2.getType()==OpdType::Imme && tac.opd3.getType()==OpdType::Reg){
		prog.push_back(gen_mov(false, "", "r1", gpr(reg.at(tac.opd3))));//注意这里顺序不可颠倒, 有一种情况: opd3一开始存在r0里面
		gen_mov32(prog, "r0", tac.opd2.getVal());
	}else if(tac.opd2.getType()==OpdType::Reg && tac.opd3.getType()==OpdType::Imme){
		prog.push_back(gen_mov(false, "", "r0", gpr(reg.at(tac.opd2))));//注意这里顺序不可颠倒, 有一种情况: opd2一开始存在r1里面
		gen_mov32(prog, "r1", tac.opd3.getVal());
	}else{//都是Reg
		// 需要考虑两个参数原先在r0 / r1的情况, 需要分情况 可以使用r0,r1,r2,r3,r9,r12这些寄存器
		int reg_a = reg.at(tac.opd2),reg_b = reg.at(tac.opd3);
		if(reg_b != 0){
			prog.push_back(gen_mov(false, "", "r0", gpr(reg_a)));
			prog.push_back(gen_mov(false, "", "r1", gpr(reg_b)));
		}else if(reg_a != 1){
			prog.push_back(gen_mov(false, "", "r1", gpr(reg_b)));
			prog.push_back(gen_mov(false, "", "r0", gpr(reg_a)));
		}else{
			prog.push_back(gen_mov(false, "", "r2", gpr(reg_b)));
			prog.push_back(gen_mov(false, "", "r0", gpr(reg_a)));
			prog.push_back(gen_mov(false, "", "r1", "r2"));
		}
	}
	//把r0, r1放进去
	prog.push_back(gen_bl("__modsi3"));
	//恢复r1,r2,r3,r9,r12
	for(int i=AllLive.size()-1;i>=0;--i){
		if(AllLive[i]==0){
			if(target == 0){
				prog.push_back(gen_add(false, "", "sp", "sp", "#4"));	
			}else{
				prog.push_back(gen_mov(false, "", gpr(target) ,"r0"));
				prog.push_back(gen_pop("r0"));
			}
		}else{
			prog.push_back(gen_pop(gpr(AllLive[i])));
		}
	}
 	//执行完前后, 除了保存除法结果的寄存器, 其他寄存器应当保持不变
	//把r0放到它该在的结果寄存器
	//恢复其他寄存器
}
void genDiv(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg) {
	//  		
	// r0 到 r3, r9, r12
	//prog.push_back(gen_push("r0"));
	//prog.push_back(gen_push("r1-r3,r9,r12"));
	int target = reg.at(tac.opd1);
	//把参数放到r0, r1
	
	std::set<TacOpd> livein = tac.getLiveIn(); 
	std::vector<int> AllLive;
	for(auto opd:livein){
		if(reg.find(opd)!=reg.end()){
			int r = reg.at(opd);
			if( r == 0 || r == 1 || r == 2 || r == 3 || r == 9 || r == 12){
				/*int offset = -4 * (r + 1);
				if(r==9) offset = -4 * 5;
				if(r==12) offset = -4*6;*/
				AllLive.push_back(r);
				prog.push_back(gen_push(gpr(r)));
				//prog.push_back(gen_str("", gpr(r), "sp", offset));	
			}
		}
	}
	if(tac.opd2.getType()==OpdType::Imme && tac.opd3.getType()==OpdType::Imme){
		gen_mov32(prog, "r0", tac.opd2.getVal());
		gen_mov32(prog, "r1", tac.opd3.getVal());
	}else if(tac.opd2.getType()==OpdType::Imme && tac.opd3.getType()==OpdType::Reg){
		prog.push_back(gen_mov(false, "", "r1", gpr(reg.at(tac.opd3))));//注意这里顺序不可颠倒, 有一种情况: opd3一开始存在r0里面
		gen_mov32(prog, "r0", tac.opd2.getVal());
	}else if(tac.opd2.getType()==OpdType::Reg && tac.opd3.getType()==OpdType::Imme){
		prog.push_back(gen_mov(false, "", "r0", gpr(reg.at(tac.opd2))));//注意这里顺序不可颠倒, 有一种情况: opd2一开始存在r1里面
		gen_mov32(prog, "r1", tac.opd3.getVal());
	}else{//都是Reg
		// 需要考虑两个参数原先在r0 / r1的情况, 需要分情况 可以使用r0,r1,r2,r3,r9,r12这些寄存器
		int reg_a = reg.at(tac.opd2),reg_b = reg.at(tac.opd3);
		if(reg_b != 0){
			prog.push_back(gen_mov(false, "", "r0", gpr(reg_a)));
			prog.push_back(gen_mov(false, "", "r1", gpr(reg_b)));
		}else if(reg_a != 1){
			prog.push_back(gen_mov(false, "", "r1", gpr(reg_b)));
			prog.push_back(gen_mov(false, "", "r0", gpr(reg_a)));
		}else{
			prog.push_back(gen_mov(false, "", "r2", gpr(reg_b)));
			prog.push_back(gen_mov(false, "", "r0", gpr(reg_a)));
			prog.push_back(gen_mov(false, "", "r1", "r2"));
		}
	}

	prog.push_back(gen_bl("__divsi3"));
	for(int i=AllLive.size()-1;i>=0;--i){
		if(AllLive[i]==0){
			if(target == 0){
				prog.push_back(gen_add(false, "", "sp", "sp", "#4"));	
			}else{
				prog.push_back(gen_mov(false, "", gpr(target) ,"r0"));
				prog.push_back(gen_pop("r0"));
			}
		}else{
			prog.push_back(gen_pop(gpr(AllLive[i])));
		}
	}
 	//执行完前后, 除了保存除法结果的寄存器, 其他寄存器应当保持不变
	//把r0放到它该在的结果寄存器
	//恢复其他寄存器
}

void genCall(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg, std::string func_name, int param_count, bool lineno) {
	//依然采取"Param"不生成代码, "Call"生成代码的策略
	//不妨假设参数已经按照规矩完成压栈		
	std::set<TacOpd> livein = tac.getLiveIn(); 
	for(auto opd:livein){
		if(reg.find(opd)!=reg.end()){
			int r = reg.at(opd);
			if( r == 0 || r == 1 || r == 2 || r == 3 || r == 9 || r == 12){
				int offset = -4 * (r + 1);
				if(r==9) offset = -4 * 5;
				if(r==12) offset = -4*6;
				prog.push_back(gen_str("", gpr(r), "sp", offset));	
			}
		}
	}
	prog.push_back(gen_sub(false, "", "sp", "sp", constant(4*6 + 4*param_count)));
	if(param_count > 0)prog.push_back(gen_pop("r0"));
	if(param_count > 1)prog.push_back(gen_pop("r1"));
	if(param_count > 2)prog.push_back(gen_pop("r2"));
	if(param_count > 3)prog.push_back(gen_pop("r3"));
	if(lineno) prog.push_back(gen_mov("", "r0", 233));	
	
	int fucntion_id = tac.opd1.getVal();
	// 函数caller准备工作
	// 在传参(第0个参数)之前,将 caller-saved寄存器压栈
	// 将前4个参数放到r0, r1, r2, r3
	// 接下来的参数倒序压栈,
	prog.push_back(gen_bl(func_name));
	// 函数caller恢复工作

	if(param_count > 4)prog.push_back(gen_add(false, "", "sp", "sp", constant(4*6 + 4*(param_count-4))));
	else prog.push_back(gen_add(false, "", "sp", "sp", constant(4*6)));
 
 	//需要在第一次param之前查一下live-out的寄存器, 只保存live-out的寄存器	
	//TODO: 把r0 mov 到对应参数的就
	if(reg.find(tac.opd2)!=reg.end()){
		int opd2_reg = reg.at(tac.opd2);
		prog.push_back(gen_mov(false, "", gpr(opd2_reg), "r0"));
	}
	for(auto opd:livein){
		if(reg.find(opd)!=reg.end()){
			int r =reg.at(opd);
			if( r == 0 || r == 1 || r == 2 || r == 3 || r == 9 || r == 12){
				int offset = -4 * (r + 1);
				if(r==9) offset = -4 * 5;
				if(r==12) offset = -4*6;
				prog.push_back(gen_ldr("", gpr(r), "sp", offset));	
			}
		}
	}
}
void genFuncStart(Lines &prog, std::string func_name, 
				  int stack_length_in_words, const set<int> &used_registers){//stack_length: spill / local variable/array
	// 函数callee准备工作
	// 除了global函数名(gen_info来做), main函数的其他准备工作和普通函数是一样的.
	// .type func_name, %function
	// 1. 保存callee-saved寄存器 r4-r11
	// 2. spill所需要的空间数目 + 局部数组的大小, 在一开始就应当知道, 对sp进行修改, 还有局部数组也应当分配. 之后load, store的时候, 都应当根据这个offset来进行相对sp的寻址
	prog.push_back(gen_info(".type "+func_name+", %function"));	
 	prog.push_back(gen_info(func_name + ":"));
	//TODO 生成一大堆store, 保存 r4-r11 8个寄存器
	for(int i = 4; i <= 11; ++i){
		if (used_registers.count(i))
		    prog.push_back(gen_str("", gpr(i), "sp", (-4) * (i-3)));
	}
	prog.push_back(gen_str("", "lr", "sp", (-4) * 9));
	gen_mov32(prog, "r4", 4*(stack_length_in_words+9));	
	////prog.push_back(gen_mov32("", "r4", constant(4*stack_length_in_words)));
	prog.push_back(gen_sub(false, "", "sp", "sp", "r4"));
}
void genRet(Lines &prog, const Tac &tac, const std::map<TacOpd, int> &reg, 
			int stack_length_in_words, const set<int> &used_registers) {
	// 函数callee恢复工作
	// 需要Ret把返回值mov到r0里?
	// main函数和其他函数的恢复工作....有所不同. 其他函数最后一句是bx lr, main 函数最后一句是 bl exit... 除了这里就没别的了?
	// 恢复callee-saved寄存器
	// 如果有返回值, 把返回值放到r0
	// TODO 恢复寄存器
	// 应当释放栈上的局部数组空间/spill需要的空间
	
	if(tac.opd1.getType()==OpdType::Reg){
		assert(reg.find(tac.opd1)!=reg.end());
		prog.push_back(gen_mov(false, "", "r0", gpr(reg.at(tac.opd1))));
	}else if(tac.opd1.getType()==OpdType::Imme){
		gen_mov32(prog, "r0", tac.opd1.getVal());
		//prog.push_back(gen_mov32("", "r0", constant(tac.opd1.getVal())));
	}else{
	}
	gen_mov32(prog, "r4", 4*(stack_length_in_words+9));
	prog.push_back(gen_add(false, "", "sp", "sp", "r4"));
	prog.push_back(gen_ldr("", "lr", "sp", (-4) * 9));
	for(int i = 4; i <= 11; ++i){
		if (used_registers.count(i))
	    	prog.push_back(gen_ldr("", gpr(i), "sp", (-4) * (i-3)));
	}
	prog.push_back(gen_bx_lr());
}
