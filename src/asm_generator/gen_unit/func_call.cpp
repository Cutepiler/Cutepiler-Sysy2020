#include "gen_unit.h"
LinePtr gen_bl(const string &function_name){
	return std::make_shared<Line>("bl " + function_name, 32);
}
LinePtr gen_bx_lr(){
	return std::make_shared<Line>("bx lr", 32); 
}
LinePtr gen_push(const string &push_registers){
	return std::make_shared<Line>("push {" + push_registers + "}", 32); 
}
LinePtr gen_pop(const string &pop_registers){
	return std::make_shared<Line>("pop {" + pop_registers + "}", 32); 
}
