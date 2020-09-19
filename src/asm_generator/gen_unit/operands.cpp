#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "gen_unit.h"

using std::string;
using std::to_string;

const string stack_pointer = "sp";
const string link_register = "lr";
const string program_counter = "pc";

string gpr(int id) {
    if (id == 13) return "lr";
    return "r" + to_string(id);
}

string constant(unsigned int value) {
    for(int i = 0;i < 16;i++){
        unsigned int num;
        int t;
        num = value << (2 * i);
        if (i == 0) t = 0;else t = 32 - 2 * i;
        num |= (value & (0xffffffff << t)) >> t;
        if (num <= 0x000000ff) return '#' + to_string(value);
    }
    return "";
    /*
    for (unsigned int mask = 0xff; (mask >> 31) == 0; mask <<= 1) {
        if ((value & (~mask)) == 0) return '#' + to_string(value);
    }
    std::stringstream tempstream;
    tempstream << std::setfill('0') << std::setw(8) << std::hex << value;
    string hexform = tempstream.str();
    assert(hexform.length() == 8);
    if (hexform[0] == hexform[4] && hexform[1] == hexform[5] &&
        hexform[2] == hexform[6] && hexform[3] == hexform[7]) {
        if (hexform[0] == '0' && hexform[1] == '0' ||
            hexform[2] == '0' && hexform[3] == '0')
            return '#' + to_string(value);
        if (hexform[0] == hexform[2] && hexform[1] == hexform[3])
            return '#' + to_string(value);
    }
    return "";*/
}

string reg_with_shift(string Rm, string shift_opr, string amount) {
    if (shift_opr == "") return Rm;
    if (shift_opr == "rrx") assert(amount == "");
    return Rm + ", " + shift_opr + " " + amount;
}


// labels
string label(int id) {
    return ".L" + std::to_string(id);
}

string new_label() {
    static int counter = 0;
    return ".Lb" + std::to_string(counter++);
}

string array_name(int id) {
    switch (id) {
        case DATA_SEC_ID:
            return "datasec";
        case BSS_SEC_ID:
            return "bsssec";
        default:
            assert(false);
    }
}

LinePtr gen_label(const string &label) {
    return std::make_shared<Line>(label + ":", 0);
}
