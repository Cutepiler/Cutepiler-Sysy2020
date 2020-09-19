#include "../tac/tac.h"
bool const_ok_for_arm(int x);
void exit0(std::ostream &os);
void return_main(std::ostream &os);
void func_return(bool is_main, std::ostream &os);
void gen_armasm(const TacProg &pg, std::ostream& os);
class AsmGenerate;
