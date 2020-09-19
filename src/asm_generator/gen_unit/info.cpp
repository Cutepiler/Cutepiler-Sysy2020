#include "gen_unit.h"
LinePtr gen_info(const string &info){
	        return std::make_shared<Line>(info, 32);
}
LinePtr gen_word(const int &word){
	        return std::make_shared<Line>(".word " + std::to_string(word), 32);
}
