#include "toarm.h"
#include "ins.h"
#include <cassert>
#include <cstdio>
int my_ffs(unsigned int x){
   if(x == 0) return 0;
   if (x&1) return 0;
   int ans = 0;
   if(!(x&0xffff)){
       ans += 16;
       x >>= 16;
   } 
   if(!(x&0xff)){
       ans += 8;
       x >>= 8;
   }
   if(!(x&0xf)){
       ans += 4;
       x >>= 4;
   }
   if(!(x&3)){
       ans += 2;
       x >>= 2;
   }
   if(!(x&1)){
       ans += 1;
       x >>= 1;
   }
   return ans;
}
bool const_ok_for_arm(int i, bool mov_mvn){// whether MOV #x create a legel 32-bit immediate, if return false, we need MOVW & MOVT
    // 0: mov
    // 1: mvn
    // 2: movw + movt
    int lowbit;
    if(mov_mvn){
        if ((i & ~(unsigned int) 0xffff) == 0) return true; // mov 16-bit immediate 
    }else{
    	if ((i & ~(unsigned int) 0xff) == 0) return true; // mvn 8-bit immediate 
    }
    /* Get the number of trailing zeros.  */
    lowbit = my_ffs((unsigned int) i) - 1;
    lowbit &= ~1; // use even number

    /* Allow rotated constants in ARM mode.  */
    if ((i & ~(((unsigned int) 0xff) << lowbit)) == 0)return true;
    // 0xc000003f: 1100 0000 0000 0000 0000 0000 0011 1111
    // 0xf000000f: 1111 0000 0000 0000 0000 0000 0000 1111
    // 0xfc000003: 1111 1100 0000 0000 0000 0000 0000 0011
    if (lowbit <= 4 && ((i & ~0xc000003f) == 0 || (i & ~0xf000000f) == 0 || (i & ~0xfc000003) == 0))
	return true;
    return false;
}
int const_type_for_arm(int i){
    if(const_ok_for_arm(i, true))return 0;//single mov ok
    if(const_ok_for_arm(~i, false))return 1;//single mvn ok, mvn ~i
    return 2;//movw & movt
}
void exit0(std::ostream &os){
    ins_mov("r0", 0, os);
    os << "bl exit" << std::endl;
}
void return_main(std::ostream &os){
    ins_pop("r4-r12, lr", os); //TODO: 这个main的返回值输出之前是否需要换行?
    os <<"bl exit"<<std::endl;
  //   ins_bl("putint", os);
  // ins_mov("r0", R"(#'\n')", os);
  //  ins_bl("putch",os);
  //  exit0(os);
}
void move_imme(int opd1_reg, int op2, std::ostream& os){
	int imme_t = const_type_for_arm(op2); 
	if(imme_t==0){//mov is ok
		ins_mov_imme(opd1_reg,op2,os);		
	}else if(imme_t==1){//mvn is ok
		ins_mvn_imme(opd1_reg,~op2,os);
	}else{//mov + movt
		ins_mov_imme(opd1_reg,op2&0xffff,os);
		ins_movt_imme(opd1_reg,(unsigned (op2))>>16,os);
	}
}
class AsmGenerate{
private:
    int reg_table[16];// reg_table[i]: the virtual register it corresponds to 
    //these registers are unavailable: pc, sp, lr , r0\r1\r2\r3, fp(r11)
    std::map<int, int> reg_map;// map between virtual register and memory address; if it don't corresponds to memory, map to register or -100
    // each virtual register corresponds to some position in the memory... also maintain the map????
    int param_count;// how many parameters have it pass? determine where to put next parameter
    int next_victim;
    int vreg_offset_count;
    int r11_offset;
    bool hot[16]; // currently in use, shouldn't be swapped out
    std::map<int, std::string> func_name;
    std::vector<TacOpd> params;
public:
    // global variable/array: .data section
    // local variable: pushed on to stack
    // global constant: safer if put in .text section, but .data section is ok?
    AsmGenerate(){
    	func_name[0] = "main";
	func_name[1] = "getint";
	func_name[2] = "getch";
	func_name[3] = "getarray";
	func_name[4] = "putint";
	func_name[5] = "putch";
	func_name[6] = "putarray";
	func_name[7] = "printf";
	func_name[8] = "_sysy_starttime";
	func_name[9] = "_sysy_stoptime";
	func_name[10] = "memset";
    }
    void ldr_r11(int reg, int offset, std::ostream &os){//TODO: consider offset out of range(+-4096), how to load them without change r11 at last? r11 could be changed in the middle
        int delta_r11 = 0;	
	while(offset - r11_offset > 4090){
	    r11_offset += 4096;
	    delta_r11 += 4096;
	}
	while(offset - r11_offset < -4090){
	    r11_offset -=4096;
	    delta_r11 -= 4096;
	}
	if(delta_r11 != 0){
	    ins_add_imme("r11", delta_r11, os);
	}
	int real_offset = offset - r11_offset;
	ins_ldr(reg, 11, real_offset, os);
    }
    void str_r11(int reg, int offset, std::ostream &os){//TODO:
	// 提问: 这个r11 offset, 在每一轮循环过后, 还好吗???????
        int delta_r11 = 0;	
	while(offset - r11_offset > 4090){
	    r11_offset += 4096;
	    delta_r11 += 4096;
	}
	while(offset - r11_offset < -4090){
	    r11_offset -=4096;
	    delta_r11 -= 4096;
	}
	if(delta_r11 != 0){
	    ins_add_imme("r11", delta_r11, os);
	}
	int real_offset = offset - r11_offset;
	ins_str(reg, 11, real_offset, os);
    }
    void reset_r11(std::ostream &os){ //TODO: reset the change of r11
   	if(r11_offset > 0)ins_sub_imme("r11", r11_offset, os);
	else if(r11_offset < 0)ins_add_imme("r11", -r11_offset, os);
	r11_offset = 0;	
    }
    int choose_victim(std::ostream &os){//choose a register to use. The original content should be saved
        if(next_victim < param_count && next_victim < 4) next_victim = param_count < 4?param_count:4;
	do{
        	++ next_victim;	    
		if(next_victim == 11)next_victim = 12;
		if(next_victim == 13)next_victim = (param_count>=4)?4:param_count;
	}while(hot[next_victim]);
	hot[next_victim] = true;
	if(reg_table[next_victim]!=-1){
	   //ins_str(next_victim, "r11", reg_map[reg_table[next_victim]],os);
	   str_r11(next_victim, reg_map[reg_table[next_victim]], os);
	}
	reg_table[next_victim] = -1;
	return next_victim;
    }
    int choose_victim_not0(std::ostream &os){//choose a register to use. The original content should be saved
        if(next_victim < param_count && next_victim < 4) next_victim = param_count < 4?param_count:4;
	do{
        	++ next_victim;	    
		if(next_victim == 11)next_victim = 12;
		if(next_victim == 13){
			next_victim = (param_count>=4)?4:param_count;
			if(next_victim == 0)next_victim ++;
		}
	}while(hot[next_victim]);
	hot[next_victim] = true;
	if(reg_table[next_victim]!=-1){
	   //ins_str(next_victim, "r11", reg_map[reg_table[next_victim]],os);
	   str_r11(next_victim, reg_map[reg_table[next_victim]], os);
	}
	reg_table[next_victim] = -1;
	return next_victim;
    }
    int prepare_reg_for(int v_reg, std::ostream &os, int flag=-1, int from_load_reg_into = 0){
    	//ensure there's an empty register to put v_reg into it
	//如果v_reg之前从未出现, 还需要在栈上分配一个空间, 给sp +=4, 然后分配这一段空间, 偏差值是相对frame pointer r11的, 所以应当是负数
	int movein = -1;
	for(int i=0;i<16;++i){
	    if(reg_table[i] == v_reg){
	      	 movein = 16 + i;break;
	    }
    	}	
	if(movein == -1){
            if(flag == -1)movein = choose_victim(os);
	    else{
	        movein = flag;
	    }
	}
	assert(reg_map.find(v_reg)!=reg_map.end());
	reg_table[movein&0xf] = v_reg;
	hot[movein&0xf] = true;
	if(from_load_reg_into) return movein;
	else return movein&0xf;
    }
    int load_reg_into(int v_reg, std::ostream &os,int flag=-1){
	int movein;
	assert(reg_map.find(v_reg)!=reg_map.end());
        movein = prepare_reg_for(v_reg,os, flag, 1);//choose_victim(os);//choose_victim should save the victim properly
	if(movein < 16)ldr_r11(movein, reg_map[v_reg], os);//ins_ldr(movein,"r11",reg_map[v_reg],os);//r11: frame pointer TODO: offset > 4096
	else movein -=16;
	//get the position
	return movein;
    }
    void mov_reg_out(int reg, std::ostream &os){
        if(reg_table[reg]!=-1){
	    str_r11(reg, reg_map[reg_table[reg]], os);//ins_str(reg, "r11", reg_map[reg_table[reg]], os); // TODO: offset > 4096
	    reg_table[reg] = -1;
	}
    }
    void mov_all_out(std::ostream &os){
        for(int i=0;i<13;++i){//largest general register: r12
	    mov_reg_out(i, os); //允许r11片里frame pointer, 仅当把r11赋值回sp的时候, 才进行处理? 越界的时候以4096为单位进行加减?
	}
    }
    void move_vreg_into(int v_reg, int reg, std::ostream &os){
        if(reg_table[reg] == v_reg)return;
	int c = -1;
	for(int i=0;i<16;++i){
	    if(reg_table[i] == v_reg){
		    c = i; break;
	    }
	}
	mov_reg_out(reg, os);
	if(c!=-1){
	    ins_mov(reg, c, os);
	}else{
	    ldr_r11(reg, reg_map[v_reg], os);//ins_ldr(reg, "r11",reg_map[v_reg], os); //TODO: offset > 4096
	}
    }
    void move_reg_into(int reg, int v_reg, std::ostream &os){
        if(reg_table[reg] == v_reg)return;
	int c = -1;
	for(int i=0;i<16;++i){
	    if(reg_table[i] == v_reg){
		    c = i; break;
	    }
	}
	assert(reg_table[reg] == -1);
	if(c!=-1){
		reg_table[c] = -1;
	}
	reg_table[reg] = v_reg;
    }
    void try_add(const TacOpd &opd, std::ostream &os){
	    if(opd.getType() == OpdType::Reg){
	         int v_reg = opd.getId();
		 if(reg_map.find(v_reg) == reg_map.end()){
		     vreg_offset_count -= 4; 
		     reg_map[v_reg] = vreg_offset_count;
		 }
	    }
    }
    void alloc_vreg(const TacFunc &f, std::ostream &os){
        //allocate space for virtual registers on stack (reg_map)
	for(auto ins: f.insts){
	   try_add(ins.opd1, os);
	   try_add(ins.opd2, os);
	   try_add(ins.opd3, os);
	}		
    }
    void sub_imme(const char* arg1, int arg2, std::ostream &os){
	    if(-256<arg2 && arg2 < 256){
	         ins_sub_imme(arg1, arg2, os);
	         return;	
	    }

	    int tmp = choose_victim(os);
	    move_imme(tmp, arg2, os);
	    os << "sub" <<" "<<arg1 << ", "<<arg1<<", "<<"r"<<tmp<<std::endl;
    }
    void add_imme(const char* arg1, int arg2, std::ostream &os){
	    if(-256<arg2 && arg2 < 256) {
		    ins_add_imme(arg1, arg2, os);
	     	    return;
	    }
	    int tmp = choose_victim_not0(os);
	    move_imme(tmp, arg2, os);
	    os << "add" <<" "<<arg1 << ", "<<arg1<<", "<<"r"<<tmp<<std::endl;
    }
    void func_begin(const TacFunc &f, std::ostream &os){
	// r0 - r3: at most 4 arguments
	// on the stack: more arguments
	// use frame pointer as base
	params.clear();
	reg_map.clear();
	// 在一开始的时候直接把reg_map填满
	for(int i=0;i<15;++i)reg_table[i] = -1;
        if (f.name == "main") {
	    ins_globl("main", os);
        }
        os << ".type" << " " << f.name <<"," << "%function" << std::endl;
        ins_label(f.name, os);//name: 使用字符串函数名作为标签
        // push registers onto stack?
        // r0-r3: arguments, don't change
	// stack : arg7, arg6....arg5 , arg4 | arg3, arg2, arg1, arg0 | r4-r12, lr |(frame pointer) local array | local variable
        //if (f.name != "main"){
	ins_push("r0-r3",os);
	ins_push("r4-r12, lr", os);//这些东西等到函数执行结束的时候都要还回来
	ins_mov("r11", "sp", os);//r12: frame pointer, 注意在frame pointer前面并不紧邻栈上的函数参数,而是有callee saved 的一些寄存器
	r11_offset = 0; //r11_offset = r11(current) - r11(func_begin), multiple of 4096
	//construct the mapping between arguments and memory in reg_map/reg_table
	//循环所有参数, 依次分配内存里的对应位置在reg_map里
	int param_len  =  f.paramId.size();
	for(int i=0;i<param_len;++i){
		reg_map[f.paramId[i]] = (10+i)*4;//relative to r11(frame pointer) 	
	}
	//对至多4个参数分配r0-r3在reg_table里
	for(int i=0;i<4&&i<param_len;++i){
		reg_table[i]=f.paramId[i];
	}
	next_victim = 4;param_count = 0;vreg_offset_count = -4 * f.length; 
	//两个重要符号: datasec, bsssec, 在函数一开始的时候准备好, 放到距离r11不远的地方? 至少比每次用到的时候都调用函数好...调用函数范围32MB, 不容易超过吧....
	//要在符号附近定义函数, 返回符号地址
        alloc_vreg(f, os);	
	move_imme(4, vreg_offset_count, os);  
	os << "add" <<" " <<"sp, sp, "<<"r4"<<std::endl;
	// Call在执行bl之前，把参数压栈(第5个之后)、压入寄存器(r0-r3)
	//a: r0 b: r1, c: r2, d: r3, 出于高效寄存器分配的考虑，应当允许把这几个参数放到栈上. 我们把这几个参数放到寄存器上，但是也要在栈上给它预留空间。
	// 现在需要在reg_table和reg_map为函数实参建立虚拟寄存器和函数关系的映射。
	// e, f, g: 依次压栈， 对应到内存上的地址
	// 以及函数退出的时候，需要把栈上函数参数所占据的空间退栈！
	// 变量声明：数组的空间分配由length确定，然后变量的声明不会表示出来。。。所以只能通过“出现新的虚拟寄存器”来确定，出现一个之前没用过的虚拟寄存器编号就在内存里分配一下，建立映射。
	// 需要在栈上把函数内声明的数组的空间分配出来（length), 以及退出的时候要退栈
	//  需要编写函数: 从tac使用的虚拟寄存器、虚拟base + offset寻址转换到真实内存地址、真实base + offset寻址（同时把用到的东西加载到通用寄存器，如果它本来不在通用寄存器里面
	//  重复一遍: 现在不在乎内存使用的效率和寄存器分配的效率，能跑就行，那么只要暴力分配一下模拟出tac的行为即可, 然后参照这个暴力版本，可以找到哪里是瓶颈, 相应进行改进和优化
	//  目前tac的指令分为：普通的算数和逻辑运算（只要完成了虚拟寄存器到真实内存的分配函数，这些操作都是trivial的），
	//  	执行一条算数、逻辑运算，需要： 1. 保证操作数已经在通用寄存器里 2. 进行操作 3. 现在结果保存在寄存器中，将结果保存回内存
	//      需要处理的情况：这里可能使用一些不能直接使用的立即数，需要MOV到寄存器里面
	//      如果是立即数和立即数的操作，应当容易地优化掉。
	//      简单起见，现在我们把不能直接使用的立即数直接扔到寄存器里，两个立即数直接把其中一个扔到寄存器里
	//  	函数 load_into(virtual_reg): 把虚拟寄存器存到真实寄存器里面，返回真实寄存器的编号(r0-r12), sava_into(virtual_reg),从真实寄存器里把数值保存到虚拟寄存器对应的内存位置里
	//  LOAD/STORE/ADDR： 主要是数组的使用，包括数组的传参。LOAD、STORE的实现，需要大力算算地址？ADDR...需要能用的绝对地址....算一算也行？
    }

    void generateText(const TacProg &prog, std::ostream &os){
	os << ".syntax unified" << std::endl;
  	for(auto &f:prog.funcs){
	    func_name[f->id] = f->name;
	}	
	for(auto &f:prog.funcs){//generate a function
	    bool is_main = (f->id == 0);
	    func_begin(*f, os);
	    int param_len = f->paramId.size();
	    int count = 0;
	    int buffer_param_count;//used by param/call
	    for(auto ins:f->insts){
		count ++;
		for(int i=0;i<16;++i)hot[i] = false;
		switch(ins.opr){
		    case TacOpr::Labl:
			mov_all_out(os);
			reset_r11(os);
	 		os << "label" << ins.opd1.getId() << ":" << std::endl;
			break;
		    case TacOpr::Branch: //unconditional branch
			mov_all_out(os);
			reset_r11(os);
			os << "b" <<" " <<"label" <<ins.opd1.getId() << std::endl;
			break;
		    case TacOpr::Beqz:
			if(ins.opd1.getType() == OpdType::Imme){
			    if(ins.opd1.getVal() == 0){
				mov_all_out(os);
			        reset_r11(os);
			        os <<"b" <<" " <<"label"<<ins.opd2.getId()<<std::endl;
			    }
			}else{
			    int victim = load_reg_into(ins.opd1.getId(), os); 
			    os << "cmp" <<" " << "r" << victim <<", " <<"#0"<<std::endl;
			    mov_all_out(os);
			    reset_r11(os);
			    os << "beq" <<" " << "label" << ins.opd2.getId() << std::endl;
			}	
			break;
		    case TacOpr::Bnez:
			if(ins.opd1.getType() == OpdType::Imme){
			    if(ins.opd1.getVal() != 0){
			        mov_all_out(os);
				reset_r11(os);
			        os <<"b" <<" " <<"label" << ins.opd2.getId()<<std::endl;
			    }
			}else{
			    int victim = load_reg_into(ins.opd1.getId(), os); 
			    os << "cmp" <<" " << "r" << victim <<", " <<"#0"<<std::endl;
			    mov_all_out(os);
			    reset_r11(os);
			    os << "bne" <<" " << "label" << ins.opd2.getId() << std::endl;
			}
			break;
		    case TacOpr::Mov:// if opd1 is a new register?????
			//opd1: a virtual register
			//opd2: a virtual register or immediate
			assert(ins.opd1.getType()==OpdType::Reg);
			if (ins.opd2.getType() == OpdType::Reg){
				//first check if opd1 stays in register?
				// if opd2 stays in register??? 
				int opd2_reg=load_reg_into(ins.opd2.getId(), os);
				int opd1_reg=prepare_reg_for(ins.opd1.getId(), os);
				ins_mov(opd1_reg,opd2_reg,os);
			}else if (ins.opd2.getType() == OpdType::Imme){
				int opd1_reg = prepare_reg_for(ins.opd1.getId(),os); 
				int op2 = ins.opd2.getVal();
				move_imme(opd1_reg, op2, os);
			}else{
				printf("move from Null or Label");assert(0);
			};
			break;
		    case TacOpr::Neg:// if opd1 is a new register???
			assert(ins.opd1.getType()==OpdType::Reg);
			if(ins.opd2.getType() == OpdType::Reg){
			    int opd2_reg = load_reg_into(ins.opd2.getId(),os);
			    int opd1_reg = prepare_reg_for(ins.opd1.getId(),os);
			    ins_neg(opd1_reg, opd2_reg, os);
			}else if(ins.opd2.getType() == OpdType::Imme){
			    int opd1_reg = prepare_reg_for(ins.opd1.getId(),os); 
		            int op2 = ins.opd2.getVal();
			    move_imme(opd1_reg, -op2, os);
			}
			break;	
		    case TacOpr::Ret:
			if(is_main){
			    //main function don't need to restore
		            if(ins.opd1.getType() == OpdType::Reg){
				int result =  load_reg_into(ins.opd1.getId(),os);
				if(result != 0){
				    ins_mov(0, result, os);
				}
			    }else if(ins.opd1.getType()==OpdType::Imme){
			        int opd1_reg = 0;
			        int op2 = ins.opd1.getVal();
			        move_imme(opd1_reg, op2, os);
			    }else{
			        assert(0);
				printf("main function return type error!");
			    }
			    return_main(os);
			}else{
		            if(ins.opd1.getType() == OpdType::Reg){
				int result = load_reg_into(ins.opd1.getId(),os);
				if(result!=0){
				    ins_mov(0, result, os);
				}
			    }else if(ins.opd1.getType()==OpdType::Imme){
			        int opd1_reg = 0;
			        int op2 = ins.opd1.getVal();
			        move_imme(opd1_reg, op2, os);
			    }else{
				assert(ins.opd1.getType()==OpdType::Null);
			    }
			    reset_r11(os);
			    ins_mov("sp", "r11", os); //这里释放了全部局部变量和数组,把sp恢复为分配局部变量之前的数值)
			    ins_pop("r4-r12, lr", os);
			    if(param_len>=4){
				add_imme("sp", param_len * 4, os);
				//ins_add_imme("sp", param_len * 4, os);
			    }else ins_add_imme("sp", 4*4, os);
			    os<<"bx lr"<<std::endl;	
			}
			break;		
		    case TacOpr::Param:
			//现在我们颠倒传参的顺序, 在收集所有参数后, 调转顺序做我们之前做过的事情
			/*if(param_count < 4){
				mov_reg_out(param_count, os);
				if(ins.opd1.getType() == OpdType::Imme){
					move_imme(param_count,ins.opd1.getVal(),os);
				}else{
					move_vreg_into(ins.opd1.getId(), param_count, os);
				}
			}*/
			params.push_back(ins.opd1);
			++param_count;
			break;
		    case TacOpr::Call:
			//generate Param and Call together
			//确保第5个参数（如果存在）位于栈顶i
			//现在传递前4个参数,如果有
			buffer_param_count = param_count;
			param_count = 0;
			for(int i=0;i<4 && i < buffer_param_count; ++i){
			    mov_reg_out(i, os);
			    if(params[buffer_param_count - i - 1].getType() == OpdType::Imme){
			    	move_imme(i, params[buffer_param_count - i - 1].getVal(), os);
			    }else{
				    move_vreg_into(params[buffer_param_count - i - 1].getId(), i, os);
			    }
			    ++ param_count;
			}
			os << "push {r0-r3}" << std::endl;
			os << "push {r9-r12}" << std::endl;
			param_count = buffer_param_count;
			//for(int i=param_count - 1; i>=4; --i){
			for(int i = 0;i < param_count - 4; ++i){ // 新i  + 旧 i = param_count - 1
			    //把params[i]放到栈顶
				if(params[i].getType() == OpdType::Imme){
					//need a temporary register
					int victim = choose_victim(os);
					move_imme(victim, params[i].getVal(), os); 
					os <<"str"<<" "<<"r"<<victim<<", "<< "[sp, "<<"#"<<-(i + 1)*4 << "]"<<std::endl;
					hot[victim] = false;
				}else{
					int tmp = load_reg_into(params[i].getId(),os);
					os <<"str"<<" "<<"r"<<tmp<<", "<< "[sp, "<<"#"<<-(i + 1)*4 << "]"<<std::endl;
					hot[tmp] = false;
					//maybe we need many many registers here....
				}
			}
			//函数调用开始时，从栈顶开始，依次是第5,6,7,8,9,个参数，栈顶之上为第4，3，2，1个参数预留空间？？？
			//generate Call
			assert(ins.opd1.getType()==OpdType::Imme);
		        if(ins.opd2.getType()==OpdType::Null){
			    if(param_count>4){
				    sub_imme("sp", (param_count-4)*4, os);    
				    //ins_sub_imme("sp", (param_count-4)*4, os);    
			    }
			    os <<"bl" <<" "<< func_name[ins.opd1.getVal()]<<std::endl;
			    params.clear();
			    os << "pop {r9-r12}" << std::endl;
			    os << "pop {r0-r3}" << std::endl;
			    param_count = 0;
			}else{
			    assert(ins.opd2.getType() == OpdType::Reg);
			    if(param_count>4){
				    sub_imme("sp", (param_count-4)*4, os);    
				    //ins_sub_imme("sp", (param_count-4)*4, os);    
			    }
			    os <<"bl" <<" "<< func_name[ins.opd1.getVal()]<<std::endl; //bl swap
			    mov_reg_out(8, os); // 现在使用的r11值不正确???
			    for(int i=0;i<16;++i){
			        if(reg_table[i] == ins.opd2.getId())reg_table[i] = -1;
			    }
			    reg_table[8] = ins.opd2.getId();
			    ins_mov(8, 0, os);
			    int v_reg = ins.opd2.getId(); 

               		    params.clear();
			    os << "pop {r9-r12}" << std::endl;
			    os << "pop {r0-r3}" << std::endl;
			    param_count = 0;
	        	    assert(reg_map.find(v_reg)!=reg_map.end());
			}			
			break;
		    case TacOpr::Add:
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				// 这里出了bug: 当Op1, Op2是同一个寄存器, 先prepare_reg_for打上标记 会导致load_reg_into的时候不能把正确数值load进来
				// 如果简单把顺序交换过来, 还会出现Bug: 先load_reg_into, 然后prepare_reg_for的时候给覆盖了.... 所以现在需要加一些机制.... 而且所有算术运算都有影响...
				int arg1 = load_reg_into(ins.opd2.getId(),os),arg2=load_reg_into(ins.opd3.getId(),os);
				int victim = prepare_reg_for(ins.opd1.getId(),os);
				ins_add(victim, arg1, arg2, os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
		 		int arg2 = choose_victim(os);
				int arg1 = load_reg_into(ins.opd2.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg2, ins.opd3.getVal(), os);
				ins_add(ans, arg1, arg2, os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
		 		int arg1 = choose_victim(os);
				int arg2 = load_reg_into(ins.opd3.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg1, ins.opd2.getVal(), os);
				ins_add(ans, arg1, arg2, os);
			}
			break;
		    case TacOpr::Sub:
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				int arg1 = load_reg_into(ins.opd2.getId(),os),arg2=load_reg_into(ins.opd3.getId(),os);
				int victim = prepare_reg_for(ins.opd1.getId(),os);
				ins_sub(victim, arg1, arg2, os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
		 		int arg2 = choose_victim(os);
				int arg1 = load_reg_into(ins.opd2.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg2, ins.opd3.getVal(), os);
				ins_sub(ans, arg1, arg2, os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
		 		int arg1 = choose_victim(os);
				int arg2 = load_reg_into(ins.opd3.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg1, ins.opd2.getVal(), os);
				ins_sub(ans, arg1, arg2, os);
			}
			break;
		    case TacOpr::Mul:
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				int arg1 = load_reg_into(ins.opd2.getId(),os),arg2=load_reg_into(ins.opd3.getId(),os);
				int victim = prepare_reg_for(ins.opd1.getId(),os);
				ins_mul(victim, arg1, arg2, os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
		 		int arg2 = choose_victim(os);
				int arg1 = load_reg_into(ins.opd2.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg2, ins.opd3.getVal(), os);
				ins_mul(ans, arg1, arg2, os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
		 		int arg1 = choose_victim(os);
				int arg2 = load_reg_into(ins.opd3.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg1, ins.opd2.getVal(), os);
				ins_mul(ans, arg1, arg2, os);
			}
			break;
		    case TacOpr::Div: //TODO: 除以常数的时候, 乘法+移位 优化?
			// 调用gcc除法函数 __divsi3 
			/*if(true){
				int arg1 = prepare_reg_for(ins.opd1.getId(), os);
				int arg2 = load_reg_into(ins.opd2.getId(), os);			
				os << "ASR" <<" " <<"r"<<arg1<<", "<<"r"<<arg2<<", "<<"#1"<<std::endl;
			}*/
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				move_vreg_into(ins.opd2.getId(), 0, os);
				move_vreg_into(ins.opd3.getId(), 1, os);
				os << "push {r1-r3, r9, r12}" << std::endl;
				os <<"bl __divsi3"<<std::endl;
				os << "pop {r1-r3, r9, r12}" << std::endl;
				reg_table[0] = reg_table[1] = -1;
				move_reg_into(0, ins.opd1.getId(), os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
				move_vreg_into(ins.opd2.getId(), 0, os);
				move_imme(1, ins.opd3.getVal(), os);
				os << "push {r1-r3, r9, r12}" << std::endl;
				os <<"bl __divsi3"<<std::endl;
				os << "pop {r1-r3, r9, r12}" << std::endl;
				reg_table[0] = reg_table[1] = -1;
				move_reg_into(0, ins.opd1.getId(), os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
				move_imme(0, ins.opd2.getVal(), os);
				move_vreg_into(ins.opd3.getId(), 1, os);
				os << "push {r1-r3, r9, r12}" << std::endl;
				os <<"bl __divsi3"<<std::endl;
				os << "pop {r1-r3, r9, r12}" << std::endl;
				reg_table[0] = reg_table[1] = -1;
				move_reg_into(0, ins.opd1.getId(), os);
			}
			break;
		    case TacOpr::Mod: //TODO: mod 特殊常数的优化?
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				move_vreg_into(ins.opd2.getId(), 0, os);
				move_vreg_into(ins.opd3.getId(), 1, os);
				os << "push {r1-r3, r9, r12}" << std::endl;
				os <<"bl __modsi3"<<std::endl;
				os << "pop {r1-r3, r9, r12}" << std::endl;
				reg_table[0] = reg_table[1] = -1;
				move_reg_into(0, ins.opd1.getId(), os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
				move_vreg_into(ins.opd2.getId(), 0, os);
				move_imme(1, ins.opd3.getVal(), os);
				os << "push {r1-r3, r9, r12}" << std::endl;
				os <<"bl __modsi3"<<std::endl;
				os << "pop {r1-r3, r9, r12}" << std::endl;
				reg_table[0] = reg_table[1] = -1;
				move_reg_into(0, ins.opd1.getId(), os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
				move_imme(0, ins.opd2.getVal(), os);
				move_vreg_into(ins.opd3.getId(), 1, os);
				os << "push {r1-r3, r9, r12}" << std::endl;
				os <<"bl __modsi3"<<std::endl;
				os << "pop {r1-r3, r9, r12}" << std::endl;
				reg_table[0] = reg_table[1] = -1;
				move_reg_into(0, ins.opd1.getId(), os);
			}
			break;
		    case TacOpr::Lt:
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				int arg1 = load_reg_into(ins.opd2.getId(),os),arg2=load_reg_into(ins.opd3.getId(),os);
				int victim = prepare_reg_for(ins.opd1.getId(),os);
				ins_lt(victim, arg1, arg2, os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
		 		int arg2 = choose_victim(os);
				int arg1 = load_reg_into(ins.opd2.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg2, ins.opd3.getVal(), os);
				ins_lt(ans, arg1, arg2, os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
		 		int arg1 = choose_victim(os);
				int arg2 = load_reg_into(ins.opd3.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg1, ins.opd2.getVal(), os);
				ins_lt(ans, arg1, arg2, os);
			}
			break;
		    case TacOpr::Gt:
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				int arg1 = load_reg_into(ins.opd2.getId(),os),arg2=load_reg_into(ins.opd3.getId(),os);
				int victim = prepare_reg_for(ins.opd1.getId(),os);
				ins_gt(victim, arg1, arg2, os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
		 		int arg2 = choose_victim(os);
				int arg1 = load_reg_into(ins.opd2.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg2, ins.opd3.getVal(), os);
				ins_gt(ans, arg1, arg2, os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
		 		int arg1 = choose_victim(os);
				int arg2 = load_reg_into(ins.opd3.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg1, ins.opd2.getVal(), os);
				ins_gt(ans, arg1, arg2, os);
			}
			break;
		    case TacOpr::Le:
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				int arg1 = load_reg_into(ins.opd2.getId(),os),arg2=load_reg_into(ins.opd3.getId(),os);
				int victim = prepare_reg_for(ins.opd1.getId(),os);
				ins_le(victim, arg1, arg2, os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
		 		int arg2 = choose_victim(os);
				int arg1 = load_reg_into(ins.opd2.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg2, ins.opd3.getVal(), os);
				ins_le(ans, arg1, arg2, os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
		 		int arg1 = choose_victim(os);
				int arg2 = load_reg_into(ins.opd3.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg1, ins.opd2.getVal(), os);
				ins_le(ans, arg1, arg2, os);
			}
			break;
		    case TacOpr::Ge:
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				int arg1 = load_reg_into(ins.opd2.getId(),os),arg2=load_reg_into(ins.opd3.getId(),os);
				int victim = prepare_reg_for(ins.opd1.getId(),os);
				ins_ge(victim, arg1, arg2, os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
		 		int arg2 = choose_victim(os);
				int arg1 = load_reg_into(ins.opd2.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg2, ins.opd3.getVal(), os);
				ins_ge(ans, arg1, arg2, os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
		 		int arg1 = choose_victim(os);
				int arg2 = load_reg_into(ins.opd3.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg1, ins.opd2.getVal(), os);
				ins_ge(ans, arg1, arg2, os);
			}
			break;
		    case TacOpr::Eq:
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				int arg1 = load_reg_into(ins.opd2.getId(),os),arg2=load_reg_into(ins.opd3.getId(),os);
				int victim = prepare_reg_for(ins.opd1.getId(),os);
				ins_eq(victim, arg1, arg2, os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
		 		int arg2 = choose_victim(os);
				int arg1 = load_reg_into(ins.opd2.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg2, ins.opd3.getVal(), os);
				ins_eq(ans, arg1, arg2, os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
		 		int arg1 = choose_victim(os);
				int arg2 = load_reg_into(ins.opd3.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg1, ins.opd2.getVal(), os);
				ins_eq(ans, arg1, arg2, os);
			}
			break;
		    case TacOpr::Ne:
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				int arg1 = load_reg_into(ins.opd2.getId(),os),arg2=load_reg_into(ins.opd3.getId(),os);
				int victim = prepare_reg_for(ins.opd1.getId(),os);
				ins_ne(victim, arg1, arg2, os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
		 		int arg2 = choose_victim(os);
				int arg1 = load_reg_into(ins.opd2.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg2, ins.opd3.getVal(), os);
				ins_ne(ans, arg1, arg2, os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
		 		int arg1 = choose_victim(os);
				int arg2 = load_reg_into(ins.opd3.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg1, ins.opd2.getVal(), os);
				ins_ne(ans, arg1, arg2, os);
			}
			break;
		    case TacOpr::Not:
			if(ins.opd2.getType() == OpdType::Imme){
			    int victim = prepare_reg_for(ins.opd1.getId(), os);
			    move_imme(victim, (ins.opd2.getVal()==0), os); 
			}else{
			    int origin = load_reg_into(ins.opd2.getId(), os);
		            int victim = prepare_reg_for(ins.opd1.getId(), os);
			    os << "EOR" <<" "<<"r"<<victim <<", "<<"r"<<origin<<", "<<"#1"<<std::endl;  
			}	
			break;
		    case TacOpr::And: // 参数有可能是10和20这样的....要搞类型转换....麻烦啊.... 目前先不考虑那种情况????
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				int arg1 = load_reg_into(ins.opd2.getId(),os),arg2=load_reg_into(ins.opd3.getId(),os);
				int victim = prepare_reg_for(ins.opd1.getId(),os);
				ins_and(victim, arg1, arg2, os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
		 		int arg2 = choose_victim(os);
				int arg1 = load_reg_into(ins.opd2.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg2, ins.opd3.getVal(), os);
				ins_and(ans, arg1, arg2, os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
		 		int arg1 = choose_victim(os);
				int arg2 = load_reg_into(ins.opd3.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg1, ins.opd2.getVal(), os);
				ins_and(ans, arg1, arg2, os);
			}
			break;
		    case TacOpr::Or:
			if(ins.opd2.getType() == OpdType::Reg && ins.opd3.getType() == OpdType::Reg){
				int arg1 = load_reg_into(ins.opd2.getId(),os),arg2=load_reg_into(ins.opd3.getId(),os);
				int victim = prepare_reg_for(ins.opd1.getId(),os);
				ins_orr(victim, arg1, arg2, os);
			}else if(ins.opd2.getType() == OpdType::Reg&&ins.opd3.getType()==OpdType::Imme){
		 		int arg2 = choose_victim(os);
				int arg1 = load_reg_into(ins.opd2.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg2, ins.opd3.getVal(), os);
				ins_orr(ans, arg1, arg2, os);
			}else{
				assert(ins.opd2.getType() == OpdType::Imme&&ins.opd3.getType()==OpdType::Reg);
		 		int arg1 = choose_victim(os);
				int arg2 = load_reg_into(ins.opd3.getId(), os);
				int ans = prepare_reg_for(ins.opd1.getId(),os);
				move_imme(arg1, ins.opd2.getVal(), os);
				ins_orr(ans, arg1, arg2, os);
			}
			break;
		    case TacOpr::Load:
			switch(prog.fbase(ins.opd3.getVal())){
				case SpaceType::Stack:// relative to frame pointer(r11)? 分配的长为length的空间, 地址比fp低, 也就是通过 [r12, #-xxx]的方式来访问
					if(ins.opd2.getType()==OpdType::Imme){
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    move_imme(victim, -r11_offset-4*f->length+ins.opd2.getVal() ,os);
					    ins_add(victim, victim, 11, os);
					    ins_ldr(victim, victim, 0, os);
					}else{
					    int arg2 = load_reg_into(ins.opd2.getId(), os);
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    move_imme(victim, -r11_offset-4 * f-> length, os);
					    ins_add(victim, victim, arg2, os);
					    ins_add(victim, victim, 11,os);
					    ins_ldr(victim, victim, 0, os);
					}
					break;
				case SpaceType::Data://relative to .data, datasec? 在data区加入额外的标签方便load/st
					if(ins.opd2.getType()==OpdType::Imme){
					    
					    mov_reg_out(0, os);
                  			    int datasec = 0; hot[0] = true;
					    os <<"bl"<<" "<<"__get_data_section"<<std::endl;
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    move_imme(victim, ins.opd2.getVal() ,os);
					    ins_add(victim, victim, datasec, os);
					    ins_ldr(victim, victim, 0, os);
					}else{
					    mov_reg_out(0, os);
                  			    int datasec = 0; hot[0] = true;
					    os <<"bl"<<" "<<"__get_data_section"<<std::endl;
					    int arg2 = load_reg_into(ins.opd2.getId(), os);
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    ins_mov(victim, arg2, os);
					    ins_add(victim, victim, datasec, os);
					    ins_ldr(victim, victim, 0, os);
					}
					break;
				case SpaceType::BSS://relative to .bss, bsssec? 在bss区加入额外的标签方便load/store?
					if(ins.opd2.getType()==OpdType::Imme){
					    mov_reg_out(0, os);
                  			    int bsssec = 0; hot[0] = true;
					    os << "bl" <<" "<<"__get_bss_section"<<std::endl;
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    move_imme(victim, ins.opd2.getVal(),os);
					    ins_add(victim, victim, bsssec, os);
					    ins_ldr(victim, victim, 0, os);
					}else{
					    mov_reg_out(0, os);
                  			    int bsssec = 0; hot[0] = true;
					    os << "bl" <<" "<<"__get_bss_section"<<std::endl;
					    int arg2 = load_reg_into(ins.opd2.getId(), os);
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    ins_mov(victim, arg2, os);
					    ins_add(victim, victim, bsssec, os);
					    ins_ldr(victim, victim, 0, os);
					}
					break;
				case SpaceType::Abs:
					//mov Opd2 into Opd1
					if(ins.opd2.getType()==OpdType::Imme){
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    move_imme(victim, ins.opd2.getVal(),os);
					    ins_ldr(victim, victim, 0, os);
					}else{
					    int arg2 = load_reg_into(ins.opd2.getId(), os);
					    int victim = prepare_reg_for(ins.opd1.getId(), os);	//如何确保这个时候不会占用arg2的寄存器?????	
					    ins_ldr(victim, arg2, 0, os);
					}
					break;
			}
			break;
		    case TacOpr::Store:
			switch(prog.fbase(ins.opd3.getVal())){ //ins1 could be immediate
				case SpaceType::Stack:// relative to frame pointer(r12)? 分配的长为length的空间, 地址比fp低, 也就是通过 [r12, #-xxx]的方式来访问
					//这个...不会超过4096的限制吧....不会吧.....在一个函数里开大小超过4096的数组???
					if(ins.opd1.getType()==OpdType::Reg){
					    if(ins.opd2.getType()==OpdType::Imme){
					        int value = load_reg_into(ins.opd1.getId(), os);
					 	int victim = choose_victim(os);		
					        move_imme(victim, -r11_offset-4*f->length + ins.opd2.getVal() ,os);
					        ins_add(victim, victim, 11, os);
					        ins_str(value, victim, 0, os);
					    }else{
					        int arg2 = load_reg_into(ins.opd2.getId(), os);
					        int value = load_reg_into(ins.opd1.getId(), os);
						int victim = choose_victim(os);
						move_imme(victim,-r11_offset-4 * f->length, os);
						ins_add(victim, victim, arg2, os);
						ins_add(victim, victim, 11, os);
					        ins_str(value, victim, 0, os);
					    }
					}else{
					    assert(ins.opd1.getType() == OpdType::Imme);
					    if(ins.opd2.getType()==OpdType::Imme){
						int value = choose_victim(os); 
					        int victim = choose_victim(os);		
						move_imme(value, ins.opd1.getVal(), os);
					        move_imme(victim, -r11_offset-4*f->length + ins.opd2.getVal() ,os);
					        ins_add(victim, victim, 11, os);
					        ins_str(value, victim, 0, os);
					    }else{
					        int arg2 = load_reg_into(ins.opd2.getId(), os);
						int value = choose_victim(os);
						int victim = choose_victim(os);
						move_imme(victim, -r11_offset-4* f->length, os);
						move_imme(value, ins.opd1.getVal(), os);
						ins_add(victim, victim, arg2, os);
						ins_add(victim, victim, 11, os);
					        ins_str(value, victim, 0, os);
					    }
					}
					break;
				case SpaceType::Data://relative to .data, datasec? 在data区加入额外的标签方便load/st
				    if(ins.opd1.getType()==OpdType::Reg){
					if(ins.opd2.getType()==OpdType::Imme){
					    mov_reg_out(0, os);
                  			    int datasec = 0; hot[0] = true;
					    os <<"bl"<<" "<<"__get_data_section"<<std::endl;
					    int victim = choose_victim(os);
					    move_imme(victim, ins.opd2.getVal(),os);
					    ins_add(victim, victim, datasec, os);
					    int value = load_reg_into(ins.opd1.getId(), os, datasec);
					    ins_str(value, victim, 0, os);
					}else{
					    mov_reg_out(0, os);
                  			    int address = 0; hot[0] = true;
					    os <<"bl"<<" "<<"__get_data_section"<<std::endl;
					    int victim = load_reg_into(ins.opd2.getId(), os);
					    ins_add(address, address, victim, os);
					    int value = load_reg_into(ins.opd1.getId(), os);		
					    ins_str(value, address, 0, os);
					}
				    }else{
					assert(ins.opd1.getType() == OpdType::Imme);
					if(ins.opd2.getType()==OpdType::Imme){
					    mov_reg_out(0, os);
                  			    int datasec = 0; hot[0] = true;
					    os <<"bl"<<" "<<"__get_data_section"<<std::endl;
					    int victim = choose_victim(os);
					    move_imme(victim, ins.opd2.getVal(),os);
					    ins_add(victim, victim, datasec, os);
					    int value = datasec;
					    move_imme(value, ins.opd1.getVal(), os);
					    ins_str(value, victim, 0, os);
					}else{
					    mov_reg_out(0, os);
                  			    int address = 0; hot[0] = true;
					    os <<"bl"<<" "<<"__get_data_section"<<std::endl;
					    int victim = load_reg_into(ins.opd2.getId(), os);
					    ins_add(address, address, victim, os);
					    int value = choose_victim(os); 
					    move_imme(value, ins.opd1.getVal(), os);
					    ins_str(value, address, 0, os);
					}
				    }
				    break;
				case SpaceType::BSS://relative to .bss, bsssec? 在bss区加入额外的标签方便load/store?
				    if(ins.opd1.getType()==OpdType::Reg){
					if(ins.opd2.getType()==OpdType::Imme){
					    mov_reg_out(0, os);
                  			    int bsssec = 0; hot[0] = true;
					    os << "bl" <<" "<<"__get_bss_section"<<std::endl;
					    int victim = choose_victim(os);
					    move_imme(victim, ins.opd2.getVal(),os);
					    ins_add(victim, victim, bsssec, os);
					    int value = load_reg_into(ins.opd1.getId(), os, bsssec);
					    ins_str(value, victim, 0, os);
					}else{
					    mov_reg_out(0, os);
                  			    int address = 0; hot[0] = true;
					    os << "bl" <<" "<<"__get_bss_section"<<std::endl;
					    int victim = load_reg_into(ins.opd2.getId(), os);//5
					    ins_add(address, address, victim, os); 
					    int value = load_reg_into(ins.opd1.getId(), os); //10		
					    ins_str(value, address, 0, os);
					}
				    }else{
					assert(ins.opd1.getType() == OpdType::Imme);
					if(ins.opd2.getType()==OpdType::Imme){
					    mov_reg_out(0, os);
                  			    int bsssec = 0; hot[0] = true;
					    os << "bl" <<" "<<"__get_bss_section"<<std::endl;
					    int victim = choose_victim(os);
					    move_imme(victim, ins.opd2.getVal(),os);
					    ins_add(victim, victim, bsssec, os);
					    int value = bsssec;
					    move_imme(value, ins.opd1.getVal(), os); 
					    ins_str(value, victim, 0, os);
					}else{
					    mov_reg_out(0, os);
                  			    int address = 0; hot[0] = true;
					    os << "bl" <<" "<<"__get_bss_section"<<std::endl;
					    int victim = load_reg_into(ins.opd2.getId(), os);//5
					    ins_add(address, address, victim, os); 
					    int value = load_reg_into(ins.opd1.getId(), os);
					    move_imme(value, ins.opd1.getVal(), os); 
					    ins_str(value, address, 0, os);
					}
				    }
				    break;
				case SpaceType::Abs:
					//mov Opd2 into Opd1
				    if(ins.opd1.getType()==OpdType::Reg){
					if(ins.opd2.getType()==OpdType::Imme){
					    int victim = choose_victim(os);		
					    int value = load_reg_into(ins.opd1.getId(), os);
					    move_imme(victim, ins.opd2.getVal(), os);
					    ins_str(value, victim, 0, os);
					}else{
					    int value = load_reg_into(ins.opd1.getId(), os);
					    int arg2 = load_reg_into(ins.opd2.getId(), os);
					    ins_str(value, arg2, 0, os);
					}
				    }else{
					assert(ins.opd1.getType() == OpdType::Imme);
					if(ins.opd2.getType()==OpdType::Imme){
					    int victim = choose_victim(os);		
					    int value = choose_victim(os);
					    move_imme(value, ins.opd1.getVal(), os); 
					    move_imme(victim, ins.opd2.getVal(), os);
					    ins_str(value, victim, 0, os);
					}else{
					    int value = choose_victim(os); 
					    move_imme(value, ins.opd1.getVal(), os); 
					    int arg2 = load_reg_into(ins.opd2.getId(), os);
					    ins_str(value, arg2, 0, os);
					}
				    }
				    break;
			}
			break;
		    case TacOpr::Addr:// put absolute address Base(Opd3)[Opd2] into register Opd1
			//需要搞清楚: 在什么时候会出现新的寄存器编号? 实际上都在prepare_reg_for里进行处理就行了?
			switch(prog.fbase(ins.opd3.getVal())){
				case SpaceType::Stack:// relative to frame pointer(r12)? 分配的长为length的空间, 地址比fp低, 也就是通过 [r12, #-xxx]的方式来访问
					//这个...不会超过4096的限制吧....不会吧.....在一个函数里开大小超过4096的数组???
					if(ins.opd2.getType()==OpdType::Imme){
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    move_imme(victim, -r11_offset-4*f->length + ins.opd2.getVal(),os);
					    ins_add(victim,victim,11,os);//这里我们认为r11应当没有变过, 但是实际上r11可能发生了变化
					}else{
					    int arg2 = load_reg_into(ins.opd2.getId(), os);
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    move_imme(victim,-r11_offset -4 * f->length, os);
					    ins_add(victim, victim, arg2, os);
					    ins_add(victim, victim, 11, os);
					}
					break;
				case SpaceType::Data://relative to .data, datasec? 在data区加入额外的标签方便load/st
					if(ins.opd2.getType()==OpdType::Imme){
					    mov_reg_out(0, os);
                  			    int datasec = 0; hot[0] = true;
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    move_imme(victim, ins.opd2.getVal(), os);
					    os << "bl" <<" "<<"__get_data_section"<<std::endl;
					    //os <<"ldr" << " "<<"r"<<datasec<<", "<<"=datasec"<<std::endl;
					    ins_add(victim, victim, datasec, os);
					}else{
					    mov_reg_out(0, os);
                  			    int datasec = 0; hot[0] = true;
					    os << "bl" <<" "<<"__get_data_section"<<std::endl;
					    int arg2 = load_reg_into(ins.opd2.getId(), os);
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    ins_mov(victim, arg2, os);
					    ins_add(victim, victim, datasec, os);
					}
					break;
				case SpaceType::BSS://relative to .bss, bsssec? 在bss区加入额外的标签方便load/store?
					if(ins.opd2.getType()==OpdType::Imme){
					    mov_reg_out(0, os);
                  			    int bsssec = 0; hot[0] = true;
					    os << "bl" <<" "<<"__get_bss_section"<<std::endl;
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    move_imme(victim, ins.opd2.getVal(), os);
					    ins_add(victim, victim, bsssec, os);
					}else{
					    mov_reg_out(0, os);
                  			    int bsssec = 0; hot[0] = true;
					    os << "bl" <<" "<<"__get_bss_section"<<std::endl;
					    int arg2 = load_reg_into(ins.opd2.getId(), os);
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    ins_mov(victim, arg2, os);
					    ins_add(victim, victim, bsssec, os);
					}
					break;
				case SpaceType::Abs:
					//mov Opd2 into Opd1
					if(ins.opd2.getType()==OpdType::Imme){
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    move_imme(victim, ins.opd2.getVal() ,os);
					}else{
					    int arg2 = load_reg_into(ins.opd2.getId(), os);
					    int victim = prepare_reg_for(ins.opd1.getId(), os);		
					    ins_mov(victim, arg2, os);
					}
					break;
			}
			break;
	            default:
			printf("not implemented!");
			assert(0);
	                break;
		}
	    }
	}
    }
    void generateData(const TacProg &prog, std::ostream &os){//generate .data section for global variable
	//TODO: 设计一个函数, 在一开始把datasec标签的值搞出来
	int L = prog.data.store.size();
	if(L==0)return;
	os << ".text" << std::endl;
	os << ".type __get_data_section, %function" << std::endl;
	os << "__get_data_section:"<<std::endl;
	os << "ldr r0, =datasec" << std::endl;
	os << "bx lr" << std::endl;
        os << ".data" << std::endl;
	os << ".align 2" << std::endl;
	os << "datasec:" << std::endl;
	os << ".word ";
	for(int i=0;i<L;++i){// global array with initial value
	    if(i==L-1){
	    	os << prog.data.store[i] << std::endl;
	    }else{
	        os << prog.data.store[i] << ",";
	    }
	}
    }
    void generateBss(const TacProg &prog, std::ostream &os){//when generate, need to differentiate two cases: modify the array itself(without other temporary) / move the array value into another variable?
	//TODO: 设计一个函数, 把bsssec标签的值搞出来
        os << ".text" << std::endl;	
	os << ".type __get_bss_section, %function" << std::endl;
	os << "__get_bss_section:"<<std::endl;
	os << "ldr r0, =bsssec" << std::endl;
	os << "bx lr" << std::endl;
	os << ".bss" << std::endl;
	os << "bsssec:" << std::endl;
	os << ".align 2" << std::endl;
	int len = prog.bss.length;
	os << ".zero" <<" " << len * 4 << std::endl;
    }
    int tacToArm(const TacProg &prog, std::ostream &os){
        // .text
        this->generateText(prog, os); 
        // push local variable on the stack, gather infomation of global variable
        // put constant into text section? global constant: end of text section(after exit(0))
        // local constant: end of function(after bx)

        // .data
        this->generateData(prog, os);
        // .bss
        this->generateBss(prog, os); // global variables init as zeros
	return 0;
    }
}TOARM;
void gen_armasm(const TacProg &pg, std::ostream& os){
	TOARM.tacToArm(pg, os);
}

