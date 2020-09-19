#include <iostream>
#include "./driver/driver.h"
#include "env/env.h"

int main(int argc, char **argv)
{
	if (argc == 1) {
		std::cout << "Cutepiler: I am NOT a compiler!" << std::endl;
		cmd_help();
		return 0;
	} else {
		if (!parse_argument(argc, argv)) {	
			return 0;
		}
		return run_compiler();
	}
}
