#pragma once

#include <map>
#include <string>
#include "../tac/tac.h"

using std::string;

// a simple parser for an arm instruction
string get_inst_type(const string &inst);
std::vector<string> get_opd_list(const string &inst);
void add_cond(string &inst, const string &cond);
void add_suffix(string &inst);
bool is_uncond_branch(const string &inst);
bool is_array_base(const string &inst, int arr_id);
string get_label(const string &inst);
bool is_caller_save(int id);
string get_cond_code(TacOpr cond);

// utilities may be used by generator
string toOperand2(const TacOpd &opd, const std::map<TacOpd, int> &reg);