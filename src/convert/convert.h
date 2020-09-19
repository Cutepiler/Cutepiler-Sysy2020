#pragma once
#include "../tac/tac.h"
#include "../flowgraph/flowgraph.h"

void test_branch_requirement(FlowGraph &flowgraph);
void convert_store_opd1_to_register(FlowGraph &flowgraph);
void convert_param_opd1_to_register(FlowGraph &flowgraph);
void convert_mul_opd_to_register(FlowGraph &flowgraph);
void limit_offset(FlowGraph &flowgraph);
void call_transformation(FlowGraph &flowgraph);
bool is_valid_imme(unsigned int val);
void make_immediate_valid(FlowGraph &flowgraph);
void load_store_limit(FlowGraph &flowgraph);
void addr_transformation(FlowGraph &flowgraph);
void convert_abs_offset_to_register(FlowGraph &flowgraph);
void convert_tac(FlowGraph &flowgraph);