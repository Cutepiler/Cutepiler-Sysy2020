/* This is the driver of the complier */

#include <cassert> 
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "driver.h" 
#include "../env/env.h"
#include "../tree/tree.h"
#include "../typecheck/typecheck.h"
#include "../ast2tac/generate_pass.h"
#include "../flowgraph/flowgraph.h"
#include "../util/cuteprint.h"
#include "../toarm/toarm.h"
#include "../optimizer/opt_driver.h"
#include "../convert/convert.h"
#include "../asm_generator/asm.h"
#include "../optimizer/adhoc/adhoc.h"

using std::string; 
using std::cout; 
using std::endl;
using std::cerr; 
using std::to_string;  

static void msg(string err)
{
    cout << "[Error] " << err << "." << endl; 
}

static void msg_debug(string logtext)
{
    // cerr << "[Log] " << logtext << endl; 
}

static void msg_test(string testResult) {
    cerr << "[Test]" << testResult << endl;
}

static void set_optlevel(int level)
{
    //OPT_LEVEL = 1;   
    OPT_LEVEL = level; 
    msg_debug("Optimization level = " + to_string(level));
}

void cmd_help()
{
    printf("[Usage]\tcutepiler input.sysy [options]\n");
    printf("[Options]\n");
    printf("\t-S\t\tCompile to ARM ASM\n");
    printf("\t-O{d}\t\tCompile in optimization level {d}\n");
    printf("\t-o {file}\tOutput to {file} (default \'a.S\')\n");   
}

/**
 * @argc: number of arguments 
 * @argv: list of arguments
 * @return: true for good, false for fail 
 */
bool parse_argument(int argc, char **argv)
{
    assert(argc != 1); 
    FILE_IN = "a.in";
    FILE_OUT = "a.S"; 
    PROG_IN = "";
    PROG_OUT = "a.out";
    logger_init("log.txt");
    debuger_init("debug.txt");
    for (int cur = 1; cur < argc; cur++) {
        string str = argv[cur], tmp;
        int level; 
        if (str[0] != '-') {
            FILE_IN = str;
            continue;
        }
        auto check_next = [&argc,&cur](std::string opt, std::string follow) {
            if (cur + 1 == argc) {
                msg("\'" + opt + "\' should be followed by "+ follow); 
                return false; 
            }
            return true;
        };
        switch (str[1]) {
            case 'S':  // compile to arm asm 
                TARGET = ASM; 
                msg_debug("Set Target: ARM ASM"); 
                break; 
            case 'O':  // optimization level
                tmp = str.substr(2);
                try {
                    level = std::stoi(tmp);
                } catch(std::invalid_argument err) {
                    msg("Optimization level should be an integer");
                    return false; 
                }
                set_optlevel(level);
                break;
            case 'o':  // output file 
                if (!check_next("-o", "output file name"))
                    return false; 
                FILE_OUT = argv[++cur]; 
                break;
            case 'a': 
                if (!check_next("-a", "ast file name"))
                    return false;
                AST_NAME = argv[++cur];
                PRINT_AST = true; 
                break; 
            case '-':
                if (str.substr(2) == "pi") {
                    if (!check_next("--pi", "program input file"))
                        return false;
                    PROG_IN = argv[++cur];
                } else if (str.substr(2) == "po") {
                    if (!check_next("--po", "program output file"))
                        return false; 
                    PROG_OUT = argv[++cur];
                } else if (str.substr(2) == "log") {
                    if (!check_next("--log", "log file name"))
                        return false; 
                    logger_init(argv[++cur]);
                } else if (str.substr(2) == "debug") {
                    if (!check_next("--debug", "debug file name"))
                        return false; 
                    debuger_init(argv[++cur]);
                }
                break;
            default: 
                msg("Unable to recognize parameter \'" + str + "\'"); 
                return false; 
                break; 
        }
    }
    return true;
}

extern TopLevel* run_parser();

#include "../ast2tac/generate_pass.h"

int run_compiler()
{
    std::ifstream fin(FILE_IN);
    std::ofstream fout(FILE_OUT);

    TopLevel *program = run_parser();
        
    TypeCheck type_checker; 
    GenVisitor translator; 

    //my_assert(false);
    program->accept(&type_checker);


    // logger << type_checker;
    program->accept(&translator);

    auto pg = translator.getProg(); 
    logger << pg << endl;  
    for (auto func : pg.funcs) {
        // logger << "Function : " << func->name << endl; 
        // auto flow = FlowGraph(func->insts, pg.fbase); 
        // logger << flow << endl;
    } 

    OPT_LEVEL = 1;
    RUN_TEST = false;
    if (FILE_IN.find("conv") != string::npos) {
        RUN_TEST = true;
    }
    if (OPT_LEVEL >= 1) {
        std::set<int> pure_funcs;
        optimize_pred(pg, pure_funcs);
        for (auto func : pg.funcs) {
            auto flow = FlowGraph(func->insts, func, pg.fbase);
            flow.calcDominator();
            flow.toSSA();
            flow.calcVars();
            flow.calcDefUses();
            optimize(flow, pg, pure_funcs, true, false);
            flow.toSatisfyUniquePredOrSuccProp();
            func->insts = flow.toInsts();
        }
        logger << pg;
        name_discretization(pg);
        optimize_mid(pg, pure_funcs);
        for (auto func : pg.funcs) {
            auto flow = FlowGraph(func->insts, func, pg.fbase);
            flow.calcDominator();
            flow.toSSA();
            flow.calcVars();
            flow.calcDefUses();
            optimize(flow, pg, pure_funcs, false, true);
            flow.toSatisfyUniquePredOrSuccProp();
            simple_if_else_trans(flow);
            convert_tac(flow);
            simple_if_trans(flow);
            //            optimize_tac(flow);
            flow.graphColoring();
            flow.computeLiveness();
            func->insts = flow.toInsts();
            flow.removeMov();
            branch_merging(func->insts);
        }
        logger << pg;
        auto asm_pg = AsmProg(pg);
        fout << asm_pg;
    } else {
        gen_armasm (pg, fout);
    }
//    logger << pg << endl;
/*    if(TARGET == ASM){
        std::ifstream fin(FILE_IN);
        std::ofstream fout(FILE_OUT);
        auto asm_pg = AsmProg(pg);
        fout << asm_pg;
    }
    else{
        logger << "interpreter started" << endl;
        std::ifstream fin(PROG_IN);
        std::ofstream fout(PROG_OUT);
        if (new_inte(pg, fin, fout) != OK) msg("Interpreter error");
    }*/
    // TODO: output
//    assert(program != nullptr);
    return 0; 
}
